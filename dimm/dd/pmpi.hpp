/*! \file pmpi.hpp
**
**/
#ifndef PERSISTENT_MPI_H

#include <mpi.h>
#include <vector>

#define PERSISTENT_MPI_H

/*! \brief All persistent MPI related stuff goes here like
**         the send/receive buffers, MPI requests and status
*/
class pmpi {
  public:
  /// \brief Empty
  pmpi();
  /// \brief Default
  pmpi( int nprocs );
  /// \brief Destructor
  ~pmpi();
  /// Wait for MPI request to complete
  void wait();
  /// Start the persistent send/receive req
  void start();
  /// Clears the persistent MPI requests
  void free_reqs();
  ///  Resize the send/receive buffers
  template<typename T>
  void resize( size_t counts );
  ///  Resize the send/receive buffers
  template<typename T>
  size_t size();
  /// Data Access Functions
  std::vector<MPI_Request> &send_reqs();
  std::vector<MPI_Request> &recv_reqs();
  std::vector<MPI_Status> &send_stats();
  std::vector<MPI_Status> &recv_stats();
  /// Get the send buffer as a type T array
  template<typename T>
  inline T *send_buf();
  /// Get the send buffer as a type T array
  template<typename T>
  inline T *recv_buf();
  ///  Resize the send/receive buffers
  template<typename T>
  size_t buf_size();

  private:
  /// MPI_Request objects
  std::vector<MPI_Request> _send_req,
                           _recv_req;
  /// MPI_Status objects
  std::vector<MPI_Status>  _send_stat,
                           _recv_stat;

  /// The send/receive buffer
  std::vector<char> _buf;

};

pmpi
::pmpi( int nprocs )
:_send_req( nprocs, MPI_REQUEST_NULL ),
 _recv_req( nprocs, MPI_REQUEST_NULL ),
 _send_stat( nprocs ),
 _recv_stat( nprocs )
{

}

pmpi
::~pmpi() {
  wait();
  free_reqs();
}

void pmpi
::wait()
{
  /// Receive Request
  for( unsigned i = 0; i < _recv_req.size(); ++i )
    if( _recv_req[i] != MPI_REQUEST_NULL )
      MPI_Wait( &_recv_req[i], &_recv_stat[i] );
  /// Send Request
  for( unsigned i = 0; i < _send_req.size(); ++i )
    if( _send_req[i] != MPI_REQUEST_NULL )
      MPI_Wait( &_send_req[i], &_send_stat[i] );
}

void pmpi
::start()
{
  /// Receive Request
  for( unsigned i = 0; i < _recv_req.size(); ++i )
    if( _recv_req[i] != MPI_REQUEST_NULL )
      MPI_Start( &_recv_req[i] );
  /// Send Request
  for( unsigned i = 0; i < _send_req.size(); ++i )
    if( _send_req[i] != MPI_REQUEST_NULL )
      MPI_Start( &_send_req[i] );
}

void pmpi
::free_reqs()
{
  /// Receive Request
  for( unsigned i = 0; i < _recv_req.size(); ++i )
    if( _recv_req[i] != MPI_REQUEST_NULL )
      MPI_Request_free( &_recv_req[i] );
  /// Send Request
  for( unsigned i = 0; i < _send_req.size(); ++i )
    if( _send_req[i] != MPI_REQUEST_NULL )
      MPI_Request_free( &_send_req[i] );
  /// Fill with NULL requests
  std::fill( _send_req.begin(), _send_req.end(), MPI_REQUEST_NULL );
  std::fill( _recv_req.begin(), _recv_req.end(), MPI_REQUEST_NULL );
}

std::vector<MPI_Request> &pmpi
::send_reqs()
{
  return _send_req;
}

std::vector<MPI_Request> &pmpi
::recv_reqs()
{
  return _recv_req;
}

std::vector<MPI_Status> &pmpi
::send_stats()
{
  return _send_stat;
}

std::vector<MPI_Status> &pmpi
::recv_stats()
{
  return _recv_stat;
}

template<typename T>
void pmpi
::resize( size_t counts ) {
  if( counts * sizeof(T) > _buf.size() )
    _buf.resize( counts * sizeof(T) );
  if( _buf.size() != 0 && counts == 0 )
    _buf.clear();
}

template<typename T>
size_t pmpi
::size() {
  return _buf.size() / sizeof(T);
}

template<typename T>
T *pmpi
::send_buf() {
  return reinterpret_cast<T *>(&_buf[0]);
}

template<typename T>
T *pmpi
::recv_buf() {
  return reinterpret_cast<T *>(&_buf[0]);
}

template<typename T>
size_t pmpi
::buf_size() {
  return _buf.size() / sizeof(T);  
}

#endif


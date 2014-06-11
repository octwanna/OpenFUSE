/*! \file persistentMPI.h
**
**/
#ifndef PERSISTENT_MPI_H

#include <mpi.h>
#include <vector>

#define PERSISTENT_MPI_H

/*! \brief All persistent MPI related stuff goes here like 
**         the send/recv buffers, mpi requests and stats
*/
class persistentMPI {
  public:
  /// \brief Empty
  persistentMPI();
  /// \brief Default
  persistentMPI( int nprocs );
  /// \brief Destructor
  ~persistentMPI();
  /// Wait for MPI request to complete
  void wait();
  /// Start the persistent send/recv req
  void start();
  /// Clears the persistent MPI requests
  void free_reqs();
  ///  Resize the send/recv buffers
  template<typename T>
  void resize( size_t counts );
  ///  Resize the send/recv buffers
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
  
  private: 
  /// MPI_Request objects
  std::vector<MPI_Request> _send_req,
                           _recv_req;
  /// MPI_Status objects
  std::vector<MPI_Status>  _send_stat,
                           _recv_stat;
 
  /// The send/recv buffer
  std::vector<char> _buf;

};

persistentMPI
::persistentMPI( int nprocs )
:_send_req( nprocs, MPI_REQUEST_NULL ),
 _recv_req( nprocs, MPI_REQUEST_NULL ),
 _send_stat( nprocs ),
 _recv_stat( nprocs )
{

}

persistentMPI
::~persistentMPI() {
  free_reqs();
} 

void persistentMPI
::wait()
{
  /// Recv Request
  for( unsigned i = 0; i < _recv_req.size(); ++i )
    if( _recv_req[i] != MPI_REQUEST_NULL )
      MPI_Wait( &_recv_req[i], &_recv_stat[i] );
  /// Send Request
  for( unsigned i = 0; i < _send_req.size(); ++i )
    if( _send_req[i] != MPI_REQUEST_NULL )
      MPI_Wait( &_send_req[i], &_send_stat[i] );
}

void persistentMPI
::start()
{
  /// Recv Request
  for( unsigned i = 0; i < _recv_req.size(); ++i )
    if( _recv_req[i] != MPI_REQUEST_NULL )
      MPI_Start( &_recv_req[i] );
  /// Send Request
  for( unsigned i = 0; i < _send_req.size(); ++i )
    if( _send_req[i] != MPI_REQUEST_NULL )
      MPI_Start( &_send_req[i] );
}

void persistentMPI
::free_reqs()
{
  /// Recv Request
  for( unsigned i = 0; i < _recv_req.size(); ++i )
    if( _recv_req[i] != MPI_REQUEST_NULL )
      MPI_Request_free( &_recv_req[i] );
  /// Send Request
  for( unsigned i = 0; i < _send_req.size(); ++i )
    if( _send_req[i] != MPI_REQUEST_NULL )
      MPI_Request_free( &_send_req[i] );
}

std::vector<MPI_Request> &persistentMPI
::send_reqs()
{
  return _send_req;
}

std::vector<MPI_Request> &persistentMPI
::recv_reqs()
{
  return _recv_req;
}

std::vector<MPI_Status> &persistentMPI
::send_stats()
{
  return _send_stat;
}

std::vector<MPI_Status> &persistentMPI
::recv_stats()
{
  return _recv_stat;
}

template<typename T>
void persistentMPI
::resize( size_t counts ) {
  if( counts * sizeof(T) > _buf.size() )
    _buf.resize( counts * sizeof(T) );
  if( _buf.size() != 0 && counts == 0 ) 
    _buf.clear();
}

template<typename T>
size_t persistentMPI
::size() {
  return _buf.size() / sizeof(T);
}

template<typename T>
T *persistentMPI
::send_buf() {
  return reinterpret_cast<T *>(&_buf[0]);
} 

template<typename T>
T *persistentMPI
::recv_buf() {
  return reinterpret_cast<T *>(&_buf[0]);
} 

#endif


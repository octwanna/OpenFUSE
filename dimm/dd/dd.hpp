/** \file dd.hpp
**        Distributed Directory structure using hash tables
**/
#ifndef DISTRIBUTED_DIRECTORY_H

#define DISTRIBUTED_DIRECTORY_H

#include "pmpi.hpp"
#include "dd_plan.hpp"
#include <vector>

namespace ofuse {

  /// The distributed directory class
  template<typename T, typename uintT, typename HashFun>
  class dd : public HashFun {
    public:
    /// No constructor
    dd();
    /// First constructor using MPI communicator
    dd( size_t size, MPI_Comm &comm );
    /// Access operator
    inline T &operator[]( size_t id );
    /// Form the receive schedule given the send schedule
    void build_recv_plan( dd_plan<uintT> &plan );
    /// Form the send schedule given the receive schedule
    void build_send_plan( dd_plan<uintT> &plan );
    /// Read a set of arrays given the plan, data array and persistent communicator
    void read( dd_plan<uintT> &plan, std::vector<T> &data, pmpi &my_pmpi );
    /// Read a set of arrays given the plan and data array
    void read( dd_plan<uintT> &plan, std::vector<T> &data );
    /// Send data copied to persistentMPI buffer
    /// and receive contiguously in _data array
    void setup_migrate
    (
      dd_plan<T> &plan,
      pmpi &my_pmpi,
      T *recv_buf
    );

    /// Convert the list of global indices to data movement plan
    template<typename ListT>
    void list_to_plan( ListT &list, dd_plan<uintT> &plan);
 
    protected:
    std::vector<T> _data;
    MPI_Comm &_mpi_comm;

  };

  /// Class implementation
  template<typename T, typename uintT, typename HashFun>
  dd<T, uintT, HashFun>
  ::dd( size_t size, MPI_Comm &comm )
  : HashFun( size, comm ),
    _data( this->size() ),
    _mpi_comm(comm)
  { /* empty */ }

  template<typename T, typename uintT, typename HashFun>
  T &dd<T, uintT, HashFun>
  ::operator[]( size_t id )
  {
    return _data[id];
  }

  template<typename T, typename uintT, typename HashFun>
  void dd<T, uintT, HashFun>
  ::build_send_plan( dd_plan<uintT> &plan ){
    assert( plan.is_empty() );
    int comm_sz = this->comm_size(), my_rank = this->rank();
    /// Create send size array and do gather
    int tot_send_size = 0;
    std::vector<int> global_size( comm_sz * comm_sz ),
                     recv_size( comm_sz );
    pmpi my_pmpi( comm_sz );
    /// Create send sizes
    for( int i = 0; i < comm_sz; ++i )
      recv_size[i] = plan.recv_offsets()[i+1] - plan.recv_offsets()[i];
    /// Step 1 : Gather the global send matrix
    MPI_Allgather
    (
      &recv_size[0], comm_sz, MPI_INT,
      &global_size[0], comm_sz, MPI_INT,
      _mpi_comm
    );
    recv_size.clear();
    /// Step 2 : Allocate enough sizes for the receive
    ///          to get the send list from send rank
    for( int i = 0; i < comm_sz; ++i ) {
      int send_size = global_size[ my_rank + i * comm_sz ];
      if( send_size != 0 ) {
        plan.send_offsets()[i+1] = send_size;
        tot_send_size += send_size;
      }
    }
    plan.send_list().resize( tot_send_size );
    /// Calculate the offsets
    for( int i = 0; i < comm_sz; ++i )
      plan.send_offsets()[i+1] += plan.send_offsets()[i];
    /// Step 3 : Communicate receive list to form the send list
    for( int i = 0; i < comm_sz; ++i ) {
      /// Send the list of lids to Send to receive ranks
      if( ( plan.recv_offsets()[i+1] - plan.recv_offsets()[i] ) > 0 )
        MPI_Isend
        (
          &plan.recv_list()[ plan.recv_offsets()[i] ],
          plan.recv_offsets()[i+1] - plan.recv_offsets()[i],
          MPI_INT, i, my_rank, _mpi_comm, &my_pmpi.send_reqs()[i]
        );
      /// Receive the list of lids to Send to receive ranks
      if( ( plan.send_offsets()[i+1]  - plan.send_offsets()[i] ) > 0 )
        MPI_Irecv
        (
          &plan.send_list()[ plan.send_offsets()[i] ],
          plan.send_offsets()[i+1] - plan.send_offsets()[i],
          MPI_INT, i, i, _mpi_comm, &my_pmpi.recv_reqs()[i]
        );
    } /// End of if i != my_rank condition
    my_pmpi.wait();
  }

  template<typename T, typename uintT, typename HashFun>
  void dd<T, uintT, HashFun>
  ::build_recv_plan( dd_plan<uintT> &plan ) {
    assert( plan.is_empty() );
    plan.swap();
    build_send_plan( plan );
    plan.swap();
  }
  
  template<typename T, typename uintT, typename HashFun>
  void dd<T, uintT, HashFun>
  ::read( dd_plan<uintT> &plan, std::vector<T> &recv_buf, pmpi &my_pmpi ) {
    if( plan.is_empty() ) return;
    int T_size_char = sizeof(T);
    /// Copy my processor data
    recv_buf.resize( plan.recv_list().size() );
    for( size_t i = plan.recv_offsets()[ this->rank() ];
                i < plan.recv_offsets()[ this->rank() + 1 ]; ++i )
      memcpy( &recv_buf[i], &_data[plan.recv_list()[i]], sizeof(T) );
    my_pmpi.resize<T>( plan.send_list().size() );
    /// Copy data to send buffer
    for( int i = 0; i < my_pmpi.buf_size<T>(); ++i )
      my_pmpi.send_buf<T>()[i] = _data[ plan.send_list()[i] ];
    /// Now send/receive from ranks other than my_rank
    for( int i = 0; i < this->comm_size(); ++i ) {
      if( i != this->rank() ) {
        /// Non-blocking send
        if( ( plan.send_offsets()[i+1] - plan.send_offsets()[i] ) > 0 ) {
          MPI_Send_init
          (
            &my_pmpi.send_buf<T>()[ plan.send_offsets()[i] ],
            ( plan.send_offsets()[i+1] - plan.send_offsets()[i] ) * T_size_char,
            MPI_CHAR,
            i,
            this->rank(),
            _mpi_comm,
            &my_pmpi.send_reqs()[i]
          );
        }
        /// Blocking receive
        if( ( plan.recv_offsets()[i+1] - plan.recv_offsets()[i] ) > 0 ) {
          MPI_Recv_init
          (
            &recv_buf[ plan.recv_offsets()[i] ],
            ( plan.recv_offsets()[i+1] - plan.recv_offsets()[i] ) * T_size_char,
            MPI_CHAR,
            i,
            i,
            _mpi_comm,
            &my_pmpi.recv_reqs()[i]
          );
        }
      }
    }
  }
  
  
  template<typename T, typename uintT, typename HashFun>
  void dd<T, uintT, HashFun>
  ::read( dd_plan<uintT> &plan, std::vector<T> &recv_buf ) {
    if( plan.is_empty() ) return;
    int T_size_char = sizeof(T);
    pmpi my_pmpi( this->comm_size() );
    /// Copy my processor data
    recv_buf.resize( plan.recv_list().size() );
    for( size_t i = plan.recv_offsets()[ this->rank() ];
                i < plan.recv_offsets()[ this->rank() + 1 ]; ++i )
      memcpy( &recv_buf[i], &_data[plan.recv_list()[i]], sizeof(T) );
    my_pmpi.resize<T>( plan.send_list().size() );
    /// Copy data to send buffer
    for( int i = 0; i < my_pmpi.buf_size<T>(); ++i )
      my_pmpi.send_buf<T>()[i] = _data[ plan.send_list()[i] ];
    /// Now send/receive from ranks other than my_rank
    for( int i = 0; i < this->comm_size(); ++i ) {
      if( i != this->rank() ) {
        /// Non-blocking send
        if( ( plan.send_offsets()[i+1] - plan.send_offsets()[i] ) > 0 ) {
          MPI_Send_init
          (
            &my_pmpi.send_buf<T>()[ plan.send_offsets()[i] ],
            ( plan.send_offsets()[i+1] - plan.send_offsets()[i] ) * T_size_char,
            MPI_CHAR,
            i,
            this->rank(),
            _mpi_comm,
            &my_pmpi.send_reqs()[i]
          );
        }
        /// Blocking receive
        if( ( plan.recv_offsets()[i+1] - plan.recv_offsets()[i] ) > 0 ) {
          MPI_Recv_init
          (
            &recv_buf[ plan.recv_offsets()[i] ],
            ( plan.recv_offsets()[i+1] - plan.recv_offsets()[i] ) * T_size_char,
            MPI_CHAR,
            i,
            i,
            _mpi_comm,
            &my_pmpi.recv_reqs()[i]
          );
        }
      }
    }
  }
  
  template<typename T, typename uintT, typename HashFun>
  template<typename ListT>
  void dd<T, uintT, HashFun>
  ::list_to_plan( ListT &list, dd_plan<uintT> &plan )
  {
    plan.is_empty();
    plan.recv_list().resize( list.size() );
//    std::copy( plan.RecvList().begin(), plan.RecvList().end(), list.begin() );
    for( size_t i = 0; i < list.size(); ++i ) {
      int proc_id = this->pid( plan.recv_list()[i] );
      plan.recv_offsets()[ proc_id + 1 ]++;
      plan.recv_list()[i] -= this->Start( proc_id );
    }
  }

  template<typename T, typename uintT, typename HashFun>
  void dd<T, uintT, HashFun>
  ::setup_migrate
  (
    dd_plan<T> &plan,
    pmpi &my_pmpi,
    T *recv_buf
  )
  {
    assert( plan.is_empty() );
     
  }

} /// End of FUSE namespace

#endif


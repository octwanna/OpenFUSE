/** \file dd.hpp
**        Distributed Directory structre using hash tables
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
    /// First constructor using MPI comm
    dd( size_t size, MPI_Comm &comm );
    /// Access operator
    inline T &operator[]( size_t id );
    /// Form the recv schedule given the send schedule
    void RecvSchedFromSendSched( dd_plan<uintT> &plan );
    /// Form the send schedule given the recv schedule
    void SendSchedFromRecvSched( dd_plan<uintT> &plan );
    /// Send data copied to persistentMPI buffer
    /// and receive contiguously in _data array
    void setup_migrate
    (
      dd_plan<T> &plan,
      pmpi &my_pmpi,
      T *recv_buf
    );
    /// Read elements from DD using list
    template<typename ListT>
    void read( ListT &list, T *recv );
    /// Read elements from DD using list
    template<typename ListT>
    void read( ListT &list, std::vector<T> &recv );
    /// Read elements from DD using plan
    void read( dd_plan<uintT> &plan, T *recv );
    /// Read elements from DD using plan
    void read( dd_plan<uintT> &plan, std::vector<T> &recv );
    /// Convert the list of gids to data movement plan
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
    _data( this->Size() ),
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
  ::SendSchedFromRecvSched( dd_plan<uintT> &plan ){
    assert( plan.is_empty() );
    int comm_sz = this->CommSize(), my_rank = this->Rank();
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
    /// Step 2 : Allocate enough sizes for the recv
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
    /// Step 3 : Communicate Recv list to form the send list
    for( int i = 0; i < comm_sz; ++i ) {
      /// Send the list of lids to Send to Recv Ranks
      if( ( plan.recv_offsets()[i+1] - plan.recv_offsets()[i] ) > 0 )
        MPI_Isend
        (
          &plan.recv_list()[ plan.recv_offsets()[i] ],
          plan.recv_offsets()[i+1] - plan.recv_offsets()[i],
          MPI_INT, i, my_rank, _mpi_comm, &my_pmpi.send_reqs()[i]
        );
      /// Recv the list of lids to Send to Recv Ranks
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
  ::RecvSchedFromSendSched( dd_plan<uintT> &plan ) {
    assert( plan.is_empty() );
    plan.swap();
    SendSchedFromRecvSched( plan );
    plan.swap();
  }

  /// Read elements from DD using list
  template<typename T, typename uintT, typename HashFun>
  template<typename ListT>
  void dd<T, uintT, HashFun>
  ::read( ListT &list, T *recv )
  {
    dd_plan<uintT> plan( list.size() );
    list_to_plan( list, plan );
    
  }

  /// Read elements from DD using list
  template<typename T, typename uintT, typename HashFun>
  template<typename ListT>
  void dd<T, uintT, HashFun>
  ::read( ListT &list, std::vector<T> &recv )
  {

  }

  /// Read elements from DD using plan
  template<typename T, typename uintT, typename HashFun>
  void dd<T, uintT, HashFun>
  ::read( dd_plan<uintT> &plan, T *recv ){

  } 

  /// Read elements from DD using plan
  template<typename T, typename uintT, typename HashFun>
  void dd<T, uintT, HashFun>
  ::read( dd_plan<uintT> &plan, std::vector<T> &recv )
  {

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
      int proc_id = this->WhatProcID( plan.recv_list()[i] );
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


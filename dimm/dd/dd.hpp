/** \file dd.hpp
**        Distributed Directory structre using hash tables
**/
#ifndef DISTRIBUTED_DIRECTORY_H

#define DISTRIBUTED_DIRECTORY_H

#include "pmpi.hpp"
#include "plan.hpp"
#include <vector>

namespace OF {

  /// The distributed directory class
  template<typename T, typename uintT, typename HashFun>
  class DD : public HashFun {
    public:
    /// No constructor
    DD();
    /// First constructor using MPI comm
    DD( size_t size, MPI_Comm &comm );
    /// Access operator
    inline T &operator[]( size_t id );
    /// Form the recv schedule given the send schedule
    void RecvSchedFromSendSched( DDCommPlan<uintT> &plan );
    /// Form the send schedule given the recv schedule
    void SendSchedFromRecvSched( DDCommPlan<uintT> &plan );
    /// Send data copied to persistentMPI buffer
    /// and receive contiguously in _data array
    void SetupMigrate
    (
      DDCommPlan<uintT> &plan,
      persistentMPI &pmpi,
      T *recv_buf
    );
    /// Read elements from DD using list
    template<typename ListT>
    void Read( ListT &list, T *recv );
    /// Read elements from DD using list
    template<typename ListT>
    void Read( ListT &list, std::vector<T> &recv );
    /// Read elements from DD using plan
    void Read( DDCommPlan<uintT> &plan, T *recv );
    /// Read elements from DD using plan
    void Read( DDCommPlan<uintT> &plan, std::vector<T> &recv );
    /// Convert the list of gids to data movement plan
    template<typename ListT>
    void ListToPlan( ListT &list, DDCommPlan<uintT> &plan );
 
    protected:
    std::vector<T> _data;
    MPI_Comm &_mpi_comm;

  };

  /// Class implementation
  template<typename T, typename uintT, typename HashFun>
  DD<T, uintT, HashFun>
  ::DD( size_t size, MPI_Comm &comm )
  : HashFun( size, comm ),
    _data( this->Size() ),
    _mpi_comm(comm)
  { /* empty */ }

  template<typename T, typename uintT, typename HashFun>
  T &DD<T, uintT, HashFun>
  ::operator[]( size_t id )
  {
    return _data[id];
  }

  template<typename T, typename uintT, typename HashFun>
  void DD<T, uintT, HashFun>
  ::SendSchedFromRecvSched( DDCommPlan<uintT> &plan )
  {
    assert( plan.isEmpty() );
    int comm_sz = this->CommSize(), my_rank = this->Rank();
    /// Create send size array and do gather
    int tot_send_size = 0;
    std::vector<int> global_size( comm_sz * comm_sz ),
                     recv_size( comm_sz );
    persistentMPI pmpi( comm_sz );
    /// Create send sizes
    for( int i = 0; i < comm_sz; ++i )
      recv_size[i] = plan.RecvOffsets()[i+1] - plan.RecvOffsets()[i];
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
        plan.SendOffsets()[i+1] = send_size;
        tot_send_size += send_size;
      }
    }
    plan.SendList().resize( tot_send_size );
    /// Calculate the offsets
    for( int i = 0; i < comm_sz; ++i )
      plan.SendOffsets()[i+1] += plan.SendOffsets()[i];
    /// Step 3 : Communicate Recv list to form the send list
    for( int i = 0; i < comm_sz; ++i ) {
      /// Send the list of lids to Send to Recv Ranks
      if( ( plan.RecvOffsets()[i+1] - plan.RecvOffsets()[i] ) > 0 )
        MPI_Isend
        (
          &plan.RecvList()[ plan.RecvOffsets()[i] ],
          plan.RecvOffsets()[i+1] - plan.RecvOffsets()[i],
          MPI_INT, i, my_rank, _mpi_comm, &pmpi.SendReq()[i]
        );
      /// Recv the list of lids to Send to Recv Ranks
      if( ( plan.SendOffsets()[i+1]  - plan.SendOffsets()[i] ) > 0 )
        MPI_Irecv
        (
          &plan.SendList()[ plan.SendOffsets()[i] ],
          plan.SendOffsets()[i+1] - plan.SendOffsets()[i],
          MPI_INT, i, i, _mpi_comm, &pmpi.RecvReq()[i]
        );
    } /// End of if i != my_rank condition
    pmpi.Wait();
  }

  template<typename T, typename uintT, typename HashFun>
  void DD<T, uintT, HashFun>
  ::RecvSchedFromSendSched( DDCommPlan<uintT> &plan )
  {
    assert( plan.isEmpty() );
    plan.Swap();
    SendSchedFromRecvSched( plan );
    plan.Swap();
  }

  /// Read elements from DD using list
  template<typename T, typename uintT, typename HashFun>
  template<typename ListT>
  void DD<T, uintT, HashFun>
  ::Read( ListT &list, T *recv )
  {
    DDCommPlan<uintT> plan( this->CommSize() );
    ListToPlan( list, plan );
    
  }

  /// Read elements from DD using list
  template<typename T, typename uintT, typename HashFun>
  template<typename ListT>
  void DD<T, uintT, HashFun>
  ::Read( ListT &list, std::vector<T> &recv )
  {

  }

  /// Read elements from DD using plan
  template<typename T, typename uintT, typename HashFun>
  void DD<T, uintT, HashFun>
  ::Read( DDCommPlan<uintT> &plan, T *recv )
  {

  } 

  /// Read elements from DD using plan
  template<typename T, typename uintT, typename HashFun>
  void DD<T, uintT, HashFun>
  ::Read( DDCommPlan<uintT> &plan, std::vector<T> &recv )
  {

  }

  template<typename T, typename uintT, typename HashFun>
  template<typename ListT>
  void DD<T, uintT, HashFun>
  ::ListToPlan( ListT &list, DDCommPlan<uintT> &plan )
  {
    assert( plan.isEmpty() );
    plan.RecvList().resize( list.size() );
//    std::copy( plan.RecvList().begin(), plan.RecvList().end(), list.begin() );
    for( size_t i = 0; i < list.size(); ++i ) {
      int proc_id = this->WhatProcID( plan.RecvList()[i] );
      plan.RecvOffsets()[ proc_id + 1 ]++;
      plan.RecvList()[i] -= this->Start( proc_id );
    }

  }

  template<typename T, typename uintT, typename HashFun>
  void DD<T, uintT, HashFun>
  ::SetupMigrate
  (
    DDCommPlan<uintT> &plan,
    persistentMPI &pmpi,
    T *recv_buf
  )
  {
    assert( plan.isEmpty() );
     
  }

} /// End of FUSE namespace

#endif


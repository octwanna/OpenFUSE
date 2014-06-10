/*! \file DDCommPlan.h
**        Unstructured communication plan
**/
#ifndef DD_COMM_PLAN_H

#define DD_COMM_PLAN_H

#include<mpi.h>
#include<vector>

namespace OF {

  /*! \brief Unstructured Communication Schedule **/
  template<typename uintT>
  class DDCommPlan {
    public:
    /// Empty Constructor
    DDCommPlan();
    /// Default Constructor
    DDCommPlan( int nprocs );
    /// Allocate
    void Resize( int nprocs );
    /// Clear contents
    void Clear();
    /// Clear only the send/recv list 
    void ClearList();
    /// Swap the send/recv schedules
    void Swap();
    /// Checks if plan is empty
    bool isEmpty();
    /******* Data access functions ********/
    std::vector<uintT> &SendList();
    std::vector<uintT> &RecvList();
    std::vector<int> &SendOffsets();
    std::vector<int> &RecvOffsets();
    std::vector<int> &SendProcs();
    std::vector<int> &RecvProcs();
  
    private:
    /// Send Schedule
    std::vector<uintT> _send_list;
    std::vector<int>   _send_procs,
                       _send_offsets;
    /// Receive Schedule
    std::vector<uintT> _recv_list;
    std::vector<int>   _recv_procs, 
                       _recv_offsets;
  };

  /***** Class implementation *****/
  template<typename uintT>
  DDCommPlan<uintT>
  ::DDCommPlan()
  {
    /* empty */
  }

  template<typename uintT>
  DDCommPlan<uintT>
  ::DDCommPlan( int nprocs )
  : _send_procs( nprocs, -1 ),
    _send_offsets( nprocs + 1 ),
    _recv_procs( nprocs, -1 ),
    _recv_offsets( nprocs + 1 )
  {
    /* empty */
  }

  template<typename uintT>
  void DDCommPlan<uintT>
  ::Resize( int nprocs )
  {
    _send_procs.resize( nprocs, -1 );
    _send_offsets.resize( nprocs + 1 );
    _recv_procs.resize( nprocs, -1 );
    _recv_offsets.resize( nprocs + 1 );
  }
 
  template<typename uintT>
  void DDCommPlan<uintT>
  ::Clear()
  {
    _send_procs.clear();
    _send_offsets.clear();
    _send_list.clear();
    _recv_procs.clear();
    _recv_offsets.clear();
    _recv_list.clear();
  }
 
  template<typename uintT>
  void DDCommPlan<uintT>
  ::ClearList()
  {
    _send_list.clear();
    _recv_list.clear();
    int nprocs = _send_procs.size();
    if( nprocs > 0 ) {
      _send_procs.resize( nprocs, -1 );
      _send_offsets.resize( nprocs + 1 );
      _recv_procs.resize( nprocs, -1 );
      _recv_offsets.resize( nprocs + 1 );
    }
  }
 
  template<typename uintT>
  void DDCommPlan<uintT>
  ::Swap()
  {
    _send_list.swap( _recv_list );
    _send_procs.swap( _recv_procs );
    _send_offsets.swap( _recv_offsets );
  }
  
  template<typename uintT>
  bool DDCommPlan<uintT>
  ::isEmpty()
  {
    return !( _send_list.size() == 0 && _recv_list.size() == 0 );
  }
  
  /******* Data access functions ********/
  
  template<typename uintT>
  std::vector<uintT> &DDCommPlan<uintT>
  ::SendList()
  {
    return _send_list;
  }
  
  template<typename uintT>
  std::vector<uintT> &DDCommPlan<uintT>
  ::RecvList()
  {
    return _recv_list;
  }
  
  template<typename uintT>
  std::vector<int> &DDCommPlan<uintT>
  ::SendOffsets()
  {
    return _send_offsets;
  }
  
  template<typename uintT>
  std::vector<int> &DDCommPlan<uintT>
  ::RecvOffsets()
  {
    return _recv_offsets;
  }
  
  template<typename uintT>
  std::vector<int> &DDCommPlan<uintT>
  ::SendProcs()
  {
    return _send_procs;
  }
  
  template<typename uintT>
  std::vector<int> &DDCommPlan<uintT>
  ::RecvProcs()
  {
    return _recv_procs;
  }

} /// End of FUSE namespace

#endif


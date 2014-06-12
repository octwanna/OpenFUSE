/*! \file dd_plan.hpp
**        Unstructured communication plan
**/
#ifndef DD_COMM_PLAN_H

#define DD_COMM_PLAN_H

#include<mpi.h>
#include<vector>

namespace ofuse {

  /*! \brief Unstructured Communication Schedule **/
  template<typename uintT>
  class dd_plan {
    public:
    /// Empty Constructor
    dd_plan();
    /// Default Constructor
    dd_plan( int nprocs );
    /// Allocate
    void resize( int nprocs );
    /// Clear contents
    void clear();
    /// Clear only the send/recv list 
    void clear_list();
    /// Swap the send/recv schedules
    void swap();
    /// Checks if plan is empty
    bool is_empty();
    /******* Data access functions ********/
    std::vector<uintT> &send_list();
    std::vector<uintT> &recv_list();
    std::vector<int> &send_offsets();
    std::vector<int> &recv_offsets();
    std::vector<int> &send_procs();
    std::vector<int> &recv_procs();
  
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
  dd_plan<uintT>
  ::dd_plan()
  {
    /* empty */
  }

  template<typename uintT>
  dd_plan<uintT>
  ::dd_plan( int nprocs )
  : _send_procs( nprocs, -1 ),
    _send_offsets( nprocs + 1 ),
    _recv_procs( nprocs, -1 ),
    _recv_offsets( nprocs + 1 )
  {
    /* empty */
  }

  template<typename uintT>
  void dd_plan<uintT>
  ::resize( int nprocs )
  {
    _send_procs.resize( nprocs, -1 );
    _send_offsets.resize( nprocs + 1 );
    _recv_procs.resize( nprocs, -1 );
    _recv_offsets.resize( nprocs + 1 );
  }
 
  template<typename uintT>
  void dd_plan<uintT>
  ::clear()
  {
    _send_procs.clear();
    _send_offsets.clear();
    _send_list.clear();
    _recv_procs.clear();
    _recv_offsets.clear();
    _recv_list.clear();
  }
 
  template<typename uintT>
  void dd_plan<uintT>
  ::clear_list()
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
  void dd_plan<uintT>
  ::swap()
  {
    _send_list.swap( _recv_list );
    _send_procs.swap( _recv_procs );
    _send_offsets.swap( _recv_offsets );
  }
  
  template<typename uintT>
  bool dd_plan<uintT>
  ::is_empty()
  {
    return !( _send_list.size() == 0 && _recv_list.size() == 0 );
  }
  
  /******* Data access functions ********/
  
  template<typename uintT>
  std::vector<uintT> &dd_plan<uintT>
  ::send_list()
  {
    return _send_list;
  }
  
  template<typename uintT>
  std::vector<uintT> &dd_plan<uintT>
  ::recv_list()
  {
    return _recv_list;
  }
  
  template<typename uintT>
  std::vector<int> &dd_plan<uintT>
  ::send_offsets()
  {
    return _send_offsets;
  }
  
  template<typename uintT>
  std::vector<int> &dd_plan<uintT>
  ::recv_offsets()
  {
    return _recv_offsets;
  }
  
  template<typename uintT>
  std::vector<int> &dd_plan<uintT>
  ::send_procs()
  {
    return _send_procs;
  }
  
  template<typename uintT>
  std::vector<int> &dd_plan<uintT>
  ::recv_procs()
  {
    return _recv_procs;
  }

} /// End of FUSE namespace

#endif


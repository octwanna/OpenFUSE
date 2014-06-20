/*! \file dimm.hpp
 ** \brief Distributed in memory mesh
**/
#include "ihstream.hpp"
#include "dd/dd.hpp"
#include "timing.hpp"
#include <fstream>

#ifndef DIMM_H

#define DIMM_H

namespace ofuse {

  template<typename floatT, typename uintT, typename HashFun>
  class dimm {
    public:
    /// \brief Empty Constructor
    dimm();
    /// \brief Hub Constructor
    dimm( const char *hub_file, MPI_Comm &comm );

    protected:
    /// HUB handle
    ihstream _hum_in;
    /// The mpi communicator
    MPI_Comm _mpi_comm;
    /// The mesh face-nodes DD
    dd< face<uintT>, uintT, HashFun > _face_dd;
    /// The mesh face-LR DD
    dd< leftRight<uintT>, uintT, HashFun > _face_lr_dd;
    /// The mesh node distribution
    dd< node<floatT>, uintT, HashFun > _node_dd;
    /// The cell hash function
    dd< cell<uintT>, uintT, HashFun > _cell_dd;
    /// Face communicator objects
    dd_plan<uintT> _face_plan;

    protected:
    void read_nodes();
    void read_faces();
    void get_face_plan();
    void close();

  };

  /// Class Implementation
  template<typename floatT, typename uintT, typename HashFun>
  dimm<floatT, uintT, HashFun>
  ::dimm( const char *hub_file, MPI_Comm &comm )
  : _hum_in( hub_file, comm ),
    _mpi_comm( comm ),
    _face_dd( _hum_in.nFace(), comm ),
    _face_lr_dd( _hum_in.nFace(), comm ),
    _node_dd( _hum_in.nNode(), comm ),
    _cell_dd( _hum_in.nCell(), comm ),
    _face_plan( _face_dd.comm_size() )
  {
    mpi_time timer(comm);
    double bytes_read;
    double elapsed;
    read_nodes();
    read_faces();
    elapsed = timer.stop() * 1.0e-3;

    /// Print statistics
    if( _face_lr_dd.rank() == 0 ) {
      bytes_read = ( sizeof(_face_dd[0]) +
                     sizeof(_face_lr_dd[0]) ) *
                     _hum_in.nFace();
      bytes_read += sizeof(_node_dd[0]) * _hum_in.nNode();
      bytes_read /= 1024.0 * 1024.0; // Bytes to MB
      std::cerr << "Totally " << bytes_read << " MB read in " << elapsed << " s\n";
      std::cerr << "Read bandwidth = " << bytes_read / elapsed << " MB/s\n";
    }
    close();
#if 1
    
    std::stringstream cat;
    cat << "schedule_list_" << _face_dd.rank() << ".dat";
    std::ofstream fout(cat.str().c_str());
    for( uintT i = 0; i < _face_lr_dd.comm_size(); ++i ) {
      if( _face_plan.send_offsets()[i+1] - _face_plan.send_offsets()[i] > 0 )
        fout << "proc " << _face_dd.rank() << " send to "  << i 
          << " list of size " << _face_plan.send_offsets()[i+1] - _face_plan.send_offsets()[i] << "\n";
      if( _face_plan.recv_offsets()[i+1] - _face_plan.recv_offsets()[i] > 0 )
        fout << "proc " << _face_dd.rank() << " recvs from "  << i 
          << " list of size " << _face_plan.recv_offsets()[i+1] - _face_plan.recv_offsets()[i] << "\n";
    }
    fout.close();
#endif    
  }

  /// Class Implementation
  template<typename floatT, typename uintT, typename HashFun>
  void dimm<floatT, uintT, HashFun>
  ::read_nodes()
  {
    /// Node coordinates
    _hum_in.read< node<floatT>, H5TNode<floatT> >
    (
      &_node_dd[0],
      _node_dd.start(),
      1, _node_dd.size()
    );
  }

  template<typename floatT, typename uintT, typename HashFun>
  void dimm<floatT, uintT, HashFun>
  ::read_faces()
  {
    /// Face Left/Right cell info
    _hum_in.read< leftRight<uintT>, H5TLeftRight<uintT> >
    (
      &_face_lr_dd[0],
      _face_lr_dd.start(),
      1, _face_lr_dd.size()
    );
    /// Face Node info
    _hum_in.read< face<uintT>, H5TFace<uintT> >
    (
      &_face_dd[0],
      _face_dd.start(),
      1, _face_dd.size()
    );
    /// Create the cell face plan
    _face_plan.clear_list();
    /// Use the face L/R cell information
    /// and ceate the face plan object
    get_face_plan();
    /// Invert the Send schedule to Recv schedule
    _face_dd.build_recv_plan( _face_plan  );
  }

  #define INC_MY_RANK 0 // disable feature
  template<typename floatT, typename uintT, typename HashFun>
  void dimm<floatT, uintT, HashFun>
  ::get_face_plan()
  {
    _face_plan.resize( _face_lr_dd.size() );
    for( uintT i = 0; i < _face_lr_dd.size(); ++i ) {
      int left  = _cell_dd.pid( _face_lr_dd[i].left );
      int right = _cell_dd.pid( _face_lr_dd[i].right );
      /// Add to face send plan
#if INC_MY_RANK
      if( left != _face_lr_dd.rank() )
#endif
        _face_plan.send_offsets()[ left+1 ]++;
      if( i + _face_lr_dd.start() < _hum_in.nInternalFace() )
#if INC_MY_RANK
        if( right != _face_lr_dd.rank() )
#endif
          _face_plan.send_offsets()[ right+1 ]++;
    }
    /// Form offset send list offset array
    for( uintT i = 0; i < _face_lr_dd.comm_size(); ++i )
      _face_plan.send_offsets()[i+1] += _face_plan.send_offsets()[i];
    /// Allocate memory for send list
    _face_plan.send_list().resize( _face_plan.send_offsets()[ _face_lr_dd.comm_size() ] );
    /// Counter to track addition into face send list
    std::vector<int> count_offset( _face_plan.send_offsets() );
    for( uintT i = 0; i < _face_lr_dd.size(); ++i ) {
      int left  = _cell_dd.pid( _face_lr_dd[i].left );
      int right = _cell_dd.pid( _face_lr_dd[i].right );
      /// Add to face send plan
#if INC_MY_RANK
      if( left != _face_lr_dd.rank() ) {
#endif        
        _face_plan.send_list()[ count_offset[ left ] ] = i + _face_lr_dd.start();
        count_offset[ left ]++;
#if INC_MY_RANK
      }
#endif
      if( i + _face_lr_dd.start() < _hum_in.nInternalFace() ) {
#if INC_MY_RANK
        if( right != _face_lr_dd.rank() ) {
#endif
          _face_plan.send_list()[ count_offset[ right ] ] = i + _face_lr_dd.start();
          count_offset[ right ]++;
#if INC_MY_RANK
        }
#endif
      }
    }
    for( uintT i = 0; i < _face_lr_dd.comm_size(); ++i )
      assert( count_offset[i] == _face_plan.send_offsets()[i+1] );
  }
  #undef INC_MY_RANK
  
  template<typename floatT, typename uintT, typename HashFun>
  void dimm<floatT, uintT, HashFun>
  ::close()
  {
    _hum_in.close();
  }

} // End of FUSE namespace

#endif

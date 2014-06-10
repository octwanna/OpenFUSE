/*! \file dimm.hpp
 ** \brief Distributed in memory mesh
**/
#include "ihstream.hpp"
#include "dd/dd.hpp"
#include "timing.hpp"
#include <fstream>

#ifndef DIMM_H

#define DIMM_H

namespace FUSE {

  template<typename floatT, typename uintT, typename HashFun>
  class DIMM {
    public:
    /// \brief Empty Constructor
    DIMM();
    /// \brief Hub Constructor
    DIMM( const char *hub_file, MPI_Comm &comm );

    protected:
    /// HUB handle
    ihstream _hub_in; 
    /// The mpi communicator
    MPI_Comm _mpi_comm;
    /// The mesh face-nodes DD
    DD< face<uintT>, uintT, HashFun > _face_dd;
    /// The mesh face-LR DD
    DD< leftRight<uintT>, uintT, HashFun > _face_lr_dd;
    /// The mesh node distribution
    DD< node<floatT>, uintT, HashFun > _node_dd;
    /// The cell hash function
    HashFun _cell_hash;
    /// Face communicator objects
    DDCommPlan<uintT> _face_plan;

    private:
    void ReadNodeDataHUB();
    void ReadFaceDataHUB();
    void FacePlanUsingFaceLR( DDCommPlan<uintT> &face_plan );
    void CloseHUBFile();
   
  };

  /// Class Implementation
  template<typename floatT, typename uintT, typename HashFun>
  DIMM<floatT, uintT, HashFun>
  ::DIMM( const char *hub_file, MPI_Comm &comm )
  : _hub_in( hub_file, comm ),
    _mpi_comm( comm ),
    _face_dd( _hub_in.nFace(), comm ),
    _face_lr_dd( _hub_in.nFace(), comm ),
    _node_dd( _hub_in.nNode(), comm ),
    _cell_hash( _hub_in.nCell(), comm ),
    _face_plan( _face_dd.CommSize() )
  {
    MPITiming timer(comm);
    double bytes_read;
    double elapsed;
    ReadNodeDataHUB();
    ReadFaceDataHUB();
    elapsed = timer.Stop() * 1.0e-3;
    
    /// Print statistics
    if( _face_lr_dd.Rank() == 0 ) {
      bytes_read = ( sizeof(_face_dd[0]) + sizeof(_face_lr_dd[0]) ) * _hub_in.nFace();
      bytes_read += sizeof(_node_dd[0]) * _hub_in.nNode();
      bytes_read /= 1024.0 * 1024.0; // Bytes to MB
      std::cerr << "Totally " << bytes_read << " MB read in " << elapsed << " s\n";
      std::cerr << "Read bandwidth = " << bytes_read / elapsed << " MB/s\n";
    }
    CloseHUBFile();
  }

  /// Class Implementation
  template<typename floatT, typename uintT, typename HashFun>
  void DIMM<floatT, uintT, HashFun>
  ::ReadNodeDataHUB()
  {
    /// Node coordinates
    _hub_in.Read< node<floatT>, H5TNode<floatT> >
    (
      &_node_dd[0],
      _node_dd.Start(), 
      1, _node_dd.Size()
    );
  }
 
  template<typename floatT, typename uintT, typename HashFun>
  void DIMM<floatT, uintT, HashFun>
  ::ReadFaceDataHUB()
  {
    /// Face Left/Right cell info
    _hub_in.Read< leftRight<uintT>, H5TLeftRight<uintT> >
    (
      &_face_lr_dd[0],
      _face_lr_dd.Start(), 
      1, _face_lr_dd.Size()
    );
    /// Face Node info
    _hub_in.Read< face<uintT>, H5TFace<uintT> >
    (
      &_face_dd[0],
      _face_dd.Start(), 
      1, _face_dd.Size()
    );
    /// Create the cell face plan
    _face_plan.ClearList();
    /// Use the face L/R cell information
    /// and ceate the face plan object
    FacePlanUsingFaceLR( _face_plan );
    /// Invert the Send schedule to Recv schedule
    _face_dd.RecvSchedFromSendSched( _face_plan  ); 
  }
 
  template<typename floatT, typename uintT, typename HashFun>
  void DIMM<floatT, uintT, HashFun>
  ::FacePlanUsingFaceLR( DDCommPlan<uintT> &face_plan )
  {
    face_plan.Resize( _face_lr_dd.Size() );
    for( uintT i = 0; i < _face_lr_dd.Size(); ++i ) {
      int left  = _cell_hash.WhatProcID( _face_lr_dd[i].left );
      int right = _cell_hash.WhatProcID( _face_lr_dd[i].right );
      /// Add to face send plan
      if( left != _face_lr_dd.Rank() )
        face_plan.SendOffsets()[ left+1 ]++;
      if( i + _face_lr_dd.Start() < _hub_in.nInternalFace() )
        if( right != _face_lr_dd.Rank() )
          face_plan.SendOffsets()[ right+1 ]++;
    }
    /// Form offset send list offset array
    for( uintT i = 0; i < _face_lr_dd.CommSize(); ++i )
      face_plan.SendOffsets()[i+1] += face_plan.SendOffsets()[i];
    /// Allocate memory for send list
    face_plan.SendList().resize( face_plan.SendOffsets()[ _face_lr_dd.CommSize() ] );
    /// Counter to track addition into face send list
    std::vector<int> count_offset( face_plan.SendOffsets() );
    for( uintT i = 0; i < _face_lr_dd.Size(); ++i ) {
      int left  = _cell_hash.WhatProcID( _face_lr_dd[i].left );
      int right = _cell_hash.WhatProcID( _face_lr_dd[i].right );
      /// Add to face send plan
      if( left != _face_lr_dd.Rank() ) {
        face_plan.SendList()[ count_offset[ left ] ] = i + _face_lr_dd.Start();
        count_offset[ left ]++;
      }
      if( i + _face_lr_dd.Start() < _hub_in.nInternalFace() ) {
        if( right != _face_lr_dd.Rank() ) {
          face_plan.SendList()[ count_offset[ right ] ] = i + _face_lr_dd.Start();
          count_offset[ right ]++;
        }
      }
    }
    for( uintT i = 0; i < _face_lr_dd.CommSize(); ++i )
      assert( count_offset[i] == face_plan.SendOffsets()[i+1] );
  }
 
  template<typename floatT, typename uintT, typename HashFun>
  void DIMM<floatT, uintT, HashFun>
  ::CloseHUBFile()
  {
    _hub_in.Close();
  }
 
} // End of FUSE namespace

#endif

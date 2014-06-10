/*! \file cellcellDIMM.h
 ** \brief Distributed in memory mesh with cell info
**/
#include "cdimm.hpp"

#ifndef CELL_cellDIMM_H

#define CELL_cellDIMM_H

namespace FUSE {

  template<typename floatT, typename uintT, typename HashFun>
  class cellDIMM {
    public:
    /// \brief Empty Constructor
    cellDIMM();
    /// \brief Hub Constructor
    cellDIMM( const char *hub_file, MPI_Comm &comm );

    protected:
    /// The mesh cell-face DD
    DD< cell<uintT>, uintT, HashFun > _cell_face_dd;
    /// The mesh cell-cell DD
    DD< cell<uintT>, uintT, HashFun > _cell_cell_dd;

    private:
    void ReadCellDataHUB();
    void ReadCellFromCache();
    void FormCellDD();
   
  };

  /// Class Implementation
  template<typename floatT, typename uintT, typename HashFun>
  cellDIMM<floatT, uintT, HashFun>
  ::cellDIMM( const char *hub_file, MPI_Comm &comm )
  : DIMM<floatT, uintT, HashFun>( hub_file, comm ),
    _cell_face_dd( _hub_in.nCell(), comm ),
    _cell_cell_dd( _hub_in.nCell(), comm )
  {
    MPITiming timer(comm);
    double bytes_read;
    double elapsed;
    ReadCellDataHUB();
    elapsed = timer.Stop() * 1.0e-3;
    
    /// Print statistics
    if( _face_lr_dd.Rank() == 0 ) {
      bytes_read = ( sizeof(_cell_face_dd[0]) + sizeof(_cell_cell_dd[0]) ) * _hub_in.nCell();
      bytes_read /= 1024.0 * 1024.0; // Bytes to MB
      std::cerr << "Totally " << bytes_read << " MB read in " << elapsed << " s\n";
      std::cerr << "Cell Read bandwidth = " << bytes_read / elapsed << " MB/s\n";
    }
    CloseHUBFile();
  }

  template<typename floatT, typename uintT, typename HashFun>
  void cellDIMM<floatT, uintT, HashFun>
  ::ReadCellDataHUB()
  {
    h5h::listString cellCache;
    cellCache.push_back( hub::cellCacheLink[ hub::PRIMARY ] );
    /// Check if cache data exisits
    bool inCache = h5h::isValidLink( _hub_in.File(), cellCache );
    if( inCache )
      ReadCellFromCache();
    else
      FormCellDD();
  }
 
  template<typename floatT, typename uintT, typename HashFun>
  void cellDIMM<floatT, uintT, HashFun>
  ::ReadCellFromCache()
  {
    h5h::listString cellFaceCache, cellCellCache;
    /// Cell Face cache
    cellFaceCache.push_back( hub::cellCacheLink[ hub::PRIMARY ] );
    cellFaceCache.push_back( hub::cellCacheLink[ hub::SECONDARY ] );
    _hub_in.Read< cell<uintT>, H5TCell<uintT> >
    (
      &_cell_face_dd[0], cellFaceCache,
      _cell_face_dd.Start(), 1,
      _cell_face_dd.Size()
    );
    /// Cell-cell cache
    cellCellCache.push_back( hub::cellCacheLink[ hub::PRIMARY ] );
    cellCellCache.push_back( hub::cellCacheLink[ hub::FIELD ] );
    _hub_in.Read< cell<uintT>, H5TCell<uintT> >
    (
      &_cell_cell_dd[0], cellFaceCache,
      _cell_cell_dd.Start(), 1,
      _cell_cell_dd.Size()
    );
  }
 
  template<typename floatT, typename uintT, typename HashFun>
  void cellDIMM<floatT, uintT, HashFun>
  ::FormCellDD()
  {

  }
 
} // End of FUSE namespace

#endif

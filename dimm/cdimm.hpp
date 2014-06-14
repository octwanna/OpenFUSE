/*! \file cdimm.h
 ** \brief Distributed in memory mesh with cell info
 **/
#include "dimm.hpp"

#ifndef CELL_DIMM_H

#define CELL_DIMM_H

namespace ofuse {

  template<typename floatT, typename uintT, typename HashFun>
  class cdimm : public dimm<floatT, uintT, HashFun> {
  public:
    /// \brief Empty Constructor
    cdimm();
    /// \brief Hub Constructor
    cdimm(const char *hub_file, MPI_Comm &comm);

  protected:
    /// The mesh cell-face dd
    dd< cell<uintT>, uintT, HashFun > _cell_face_dd;
    /// The mesh cell-cell dd
    dd< cell<uintT>, uintT, HashFun > _cell_cell_dd;

  private:
    void read_cells();
    void read_cache();
    void form_cell_dd();

  };

  /// Class Implementation

  template<typename floatT, typename uintT, typename HashFun>
  cdimm<floatT, uintT, HashFun>
  ::cdimm(const char *hub_file, MPI_Comm &comm)
  : dimm<floatT, uintT, HashFun>(hub_file, comm),
  _cell_face_dd(this->_hum_in.nCell(), comm),
  _cell_cell_dd(this->_hum_in.nCell(), comm) {
    mpi_time timer(comm);
    double bytes_read;
    double elapsed;
    read_cells();
    elapsed = timer.stop() * 1.0e-3;

    /// Print statistics
    if (this->_face_lr_dd.rank() == 0) {
      bytes_read = (sizeof (_cell_face_dd[0]) + sizeof (_cell_cell_dd[0])) * this->_hum_in.nCell();
      bytes_read /= 1024.0 * 1024.0; // Bytes to MB
      std::cerr << "Totally " << bytes_read << " MB read in " << elapsed << " s\n";
      std::cerr << "Cell Read bandwidth = " << bytes_read / elapsed << " MB/s\n";
    }
    this->close();
  }

  template<typename floatT, typename uintT, typename HashFun>
  void cdimm<floatT, uintT, HashFun>
  ::read_cells() {
    h5pp::listString cellCache;
    cellCache.push_back(hum::cellCacheLink[ hum::PRIMARY ]);
    /// Check if cache data exisits
    bool inCache = h5pp::IsValidLink(this->_hum_in.file(), cellCache);
    if (inCache)
      read_cache();
    else
      form_cell_dd();
  }

  template<typename floatT, typename uintT, typename HashFun>
  void cdimm<floatT, uintT, HashFun>
  ::read_cache() {
    h5pp::listString cellFaceCache, cellCellCache;
    /// Cell Face cache
    cellFaceCache.push_back(hum::cellCacheLink[ hum::PRIMARY ]);
    cellFaceCache.push_back(hum::cellCacheLink[ hum::SECONDARY ]);
    this->_hum_in. template read< cell<uintT>, H5TCell<uintT> >
      (
      &_cell_face_dd[0], cellFaceCache,
      _cell_face_dd.start(), 1,
      _cell_face_dd.size()
      );
    /// Cell-cell cache
    cellCellCache.push_back(hum::cellCacheLink[ hum::PRIMARY ]);
    cellCellCache.push_back(hum::cellCacheLink[ hum::FIELD ]);
    this->_hum_in. template read< cell<uintT>, H5TCell<uintT> >
      (
      &_cell_cell_dd[0], cellFaceCache,
      _cell_cell_dd.start(), 1,
      _cell_cell_dd.size()
      );
  }

  template<typename floatT, typename uintT, typename HashFun>
  void cdimm<floatT, uintT, HashFun>
  ::form_cell_dd() {

  }

} // End of FUSE namespace

#endif

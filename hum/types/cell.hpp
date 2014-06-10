/*! \file cell.hpp
 **       The header file containing the basic
 **       mesh entity shape cell
 **/
#ifndef CELL_ENTITY_HPP

#define CELL_ENTITY_HPP

/*! \brief Cell entity of the mesh
 **        Contains the face IDs
 **        forming the cell
 **/
template< typename uintT >
struct cell {
  uintT bField;
  uintT entityID[6];

#define WIDTH 6
  /// Add an entry to the cell data-structure

  inline void Add(uintT id) {
    uintT mask = (1 << WIDTH) - 1;
    uintT n = bField & mask;
    entityID[n] = id;
    bField = (bField & ~mask) | (n + 1);
    std::cout << " entityID[" << n << "] = " << id << "\n";
    std::cout << "Total faces = " << n + 1 << "\n";
  }
  /// Add an entry to the cell data-structure

  inline void AddL(uintT id) {
    uintT mask = (1 << WIDTH) - 1;
    uintT n = bField & mask;
    entityID[n] = id;
    bField = (bField & ~mask) | (n + 1);
  }
  /// Add an entry to the cell data-structure

  inline void AddR(uintT id) {
    uintT mask = (1 << WIDTH) - 1;
    uintT n = bField & mask;
    entityID[n] = id;
    bField = (bField & ~mask) | (n + 1);
    (*this) << n;
  }
  ///

  inline void operator<<(unsigned n) {
    unsigned pos = WIDTH + n;
    bField ^= 1 << pos;
  }
  /// 

  inline int operator>>(unsigned n) {
    unsigned pos = WIDTH + n;
    uintT mask = 1 << pos;
    return 1 - 2 * ((bField & mask) >> pos);
  }

  inline unsigned Size() {
    uintT mask = (1 << WIDTH) - 1;
    return bField & mask;
  }
#if 0
  /// Private functions
private:
  /// Get the value of the i^{th} bitfield
  inline unsigned const operator[](unsigned i) {
    uintT mask = (((1 << WIDTH) - 1) << i * WIDTH);
    return ( bField & mask) >> i * WIDTH;
  }
#endif
#undef WIDTH
};

/*! \brief polyCell entity of the mesh
 **        Contains the face IDs forming
 **        the polyhedral cell
 **/
template <typename uintT>
struct polyCell {
  uintT bField;
  uintT *entityIDs;
};

#endif


/*! \file patch.hpp
 **       The header file containing the basic
 **       mesh boundary patch types
 **/
#ifndef PATCH_ENTITY_HPP

#define PATCH_ENTITY_HPP

#include<map>
#include<string>

/*! \brief The boundary patch data-structure
 **        WARNING: Do not change the ordering in
 **        patchBC type. HDF5 types will fail if you
 **        change the ordering.
 **/
template <typename uintT>
struct patchBC {
  uintT bcType;
  uintT startFace;
  uintT faceCount;
  uintT attachedToProcID;
};

/*! \brief Boundary face (patch) map
 **        The boundary patch to string
 **        map data-structure
 **        
 **/
template <typename uintT>
struct patchBCMap {
  typedef std::map< std::string, patchBC<uintT> > Type;
  typedef typename std::map< std::string, patchBC<uintT> >::iterator itType;
};


#endif

/*! \file types.hpp
 **       Header file defines the basic hum types
 **       these correspond to the basic mesh entity
 **       types namely, node, face, cell, bc data, etc
 **/
#ifndef TYPES_HPP

#define TYPES_HPP

#include "h5++.hpp"
#include "types/node.hpp"
#include "types/edge.hpp"
#include "types/face.hpp"
#include "types/cell.hpp"
#include "types/patch.hpp"
#include "constants.hpp"

struct humType {
public:

  ~humType() {
    H5Tclose(_mtype);
    H5Tclose(_ftype);
  }

  hid_t &file_t() {
    return _ftype;
  }

  hid_t &mem_t() {
    return _mtype;
  }

  const char *linkStr() {
    return _link.c_str();
  }

  hid_t &operator()() {
    return _ftype;
  }


protected:
  hid_t _ftype, _mtype;
  std::string _link;

};

/*! \brief The node HDF5 type data-structure
 ** 
 **/
template < typename floatT >
struct H5TNode : public humType {
public:

  /*! \brief Constructor commits the HDF5
   **        type node to the node struct of FUSE
   **/
  H5TNode() {
    hsize_t dims(3);
    _mtype = H5Tarray_create(h5pp::GetHDF5Type<floatT>(), 1, &dims);
    _ftype = H5Tarray_create(h5pp::GetHDF5Type<floatT>(), 1, &dims);
    _link = hum::nodeLink[hum::PRIMARY];
  }
};

/*! \brief The edge HDF5 type data-structure
 **
 **/
template < typename uintT >
struct H5TEdge : public humType {
public:

  /*! \brief Constructor commits the HDF5
   **        type edge to the edge struct of FUSE
   **/
  H5TEdge() {
    hsize_t dims(2);
    _ftype = H5Tcreate(H5T_COMPOUND, sizeof ( edge<uintT>));
    _mtype = H5Tcreate(H5T_COMPOUND, sizeof ( edge<uintT>));
    _type_2 = H5Tarray_create(h5pp::GetHDF5Type<uintT>(), 1, &dims);
    H5Tinsert(_ftype, hum::edgeLink[hum::ENTITY], HOFFSET(edge<uintT>, entityID), _type_2);
    H5Tinsert(_mtype, hum::edgeLink[hum::ENTITY], HOFFSET(edge<uintT>, entityID), _type_2);
    _link = hum::edgeLink[hum::PRIMARY];
  }

  /*! \brief Destructor
   **
   **/
  ~H5TEdge() {
    H5Tclose(_type_2);
  }

private:
  hid_t _type_2; /*!< The hdf5 type access */
};

/*! \brief The Face HDF5 type data-structure
 **
 **/
template < typename uintT >
struct H5TFace : public humType {
public:

  /*! \brief Constructor commits the HDF5
   **        type face to the face struct of FUSE
   **/
  H5TFace() {
    hsize_t dims(4);
    _ftype = H5Tcreate(H5T_COMPOUND, sizeof ( face<uintT>));
    _mtype = H5Tcreate(H5T_COMPOUND, sizeof ( face<uintT>));
    _type_4 = H5Tarray_create(h5pp::GetHDF5Type<uintT>(), 1, &dims);
    H5Tinsert(_ftype, hum::faceLink[hum::FIELD],
      HOFFSET(face<uintT>, bField),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_ftype, hum::faceLink[hum::ENTITY],
      HOFFSET(face<uintT>, entityID), _type_4);
    H5Tinsert(_mtype, hum::faceLink[hum::FIELD],
      HOFFSET(face<uintT>, bField),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_mtype, hum::faceLink[hum::ENTITY],
      HOFFSET(face<uintT>, entityID), _type_4);
    _link = hum::faceLink[hum::PRIMARY];
  }

  /*! \brief Destructor
   **
   **/
  ~H5TFace() {
    H5Tclose(_type_4);
  }

private:
  hid_t _type_4; /*!< The hdf5 type access */
};

/*! \brief The patch HDF5 type data-structure
 **
 **/
template < typename uintT >
struct H5TPatch : public humType {
public:

  /*! \brief Constructor commits the HDF5 
   **        type patch to the patch struct of FUSE
   **/
  H5TPatch() {
    _mtype = H5Tcreate(H5T_COMPOUND, sizeof ( patchBC<uintT>));
    H5Tinsert(_mtype, hum::patchLink[hum::BCTYPE], HOFFSET(patchBC<uintT>, bcType),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_mtype, hum::patchLink[hum::START_FACE], HOFFSET(patchBC<uintT>, startFace),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_mtype, hum::patchLink[hum::FACE_COUNT], HOFFSET(patchBC<uintT>, faceCount),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_mtype, hum::patchLink[hum::PROC_ID], HOFFSET(patchBC<uintT>, attachedToProcID),
      h5pp::GetHDF5Type<uintT>());

    _ftype = H5Tcreate(H5T_COMPOUND, sizeof ( patchBC<uintT>));
    //- sizeof( std::vector<uintT> ) );
    H5Tinsert(_ftype, hum::patchLink[hum::BCTYPE], HOFFSET(patchBC<uintT>, bcType),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_ftype, hum::patchLink[hum::START_FACE], HOFFSET(patchBC<uintT>, startFace),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_ftype, hum::patchLink[hum::FACE_COUNT], HOFFSET(patchBC<uintT>, faceCount),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_ftype, hum::patchLink[hum::PROC_ID], HOFFSET(patchBC<uintT>, attachedToProcID),
      h5pp::GetHDF5Type<uintT>());
    _link = hum::patchLink[hum::SECONDARY];
  }
};

/*! \brief The internalFaceLeftRightCell HDF5 type data-structure
 **
 **/
template < typename uintT >
struct H5TLeftRight : public humType {
public:

  /*! \brief Constructor commits the HDF5
   **        type leftRight to the leftRight struct of FUSE
   **/
  H5TLeftRight() {
    _ftype = H5Tcreate(H5T_COMPOUND, sizeof ( leftRight<uintT>));
    _mtype = H5Tcreate(H5T_COMPOUND, sizeof ( leftRight<uintT>));
    H5Tinsert(_ftype, hum::faceLink[hum::LEFT_ID], HOFFSET(leftRight<uintT>, left),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_ftype, hum::faceLink[hum::RIGHT_ID], HOFFSET(leftRight<uintT>, right),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_mtype, hum::faceLink[hum::LEFT_ID], HOFFSET(leftRight<uintT>, left),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_mtype, hum::faceLink[hum::RIGHT_ID], HOFFSET(leftRight<uintT>, right),
      h5pp::GetHDF5Type<uintT>());
    _link = hum::faceLink[hum::SECONDARY];
  }
};

/*! \brief The Cell HDF5 type data-structure
 **
 **/
template < typename uintT >
struct H5TCell : public humType {
public:

  /*! \brief Constructor commits the HDF5 
   **        type face to the face struct of FUSE
   **/
  H5TCell() {
    hsize_t dims(6);
    _ftype = H5Tcreate(H5T_COMPOUND, sizeof ( cell<uintT>));
    _mtype = H5Tcreate(H5T_COMPOUND, sizeof ( cell<uintT>));
    _type_6 = H5Tarray_create(h5pp::GetHDF5Type<uintT>(), 1, &dims);
    H5Tinsert(_ftype, hum::faceLink[hum::FIELD],
      HOFFSET(cell<uintT>, bField),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_ftype, hum::faceLink[hum::ENTITY],
      HOFFSET(cell<uintT>, entityID), _type_6);
    H5Tinsert(_mtype, hum::faceLink[hum::FIELD],
      HOFFSET(cell<uintT>, bField),
      h5pp::GetHDF5Type<uintT>());
    H5Tinsert(_mtype, hum::faceLink[hum::ENTITY],
      HOFFSET(cell<uintT>, entityID), _type_6);
    _link = hum::faceLink[hum::PRIMARY];
  }

  /*! \brief Destructor
   **        
   **/
  ~H5TCell() {
    H5Tclose(_type_6);
  }
private:
  hid_t _type_6; /*!< The hdf5 type access */
};

#endif


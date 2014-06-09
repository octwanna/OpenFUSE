/*! \file ohstream.h
 **       hum output file stream class
 **/
#ifndef OHSTREAM_HPP

#define OHSTREAM_HPP

#include "types.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <fstream>

/*! \brief hum output file stream class
 **        It provides stream access to the hum
 **        file very similar to C++ ofstream
 **/
class ohstream {
public:
  /*! \brief Empty construcor */
  ohstream();

  /*! \brief Empty construcor */
  ~ohstream();

  /*! \brief hum output file for serial write */
  ohstream(const char *fname);

  /*! \brief Open hum output file for serial write */
  void open(const char *fname);

  /*! \brief Close hum output file */
  void close();

  /*! Write node values to hum file **/
  template<typename floatT>
  void write(node<floatT> *n);

  /*! Write face values to hum file **/
  template<typename uintT>
  void write(face<uintT> *n);

  /*! Write node values to hum file **/
  template<typename floatT>
  void write(node<floatT> *n, hsize_t offset, hsize_t stride, hsize_t size);

  /*! Write face values to hum file **/
  template<typename uintT>
  void write(face<uintT> *n, hsize_t offset, hsize_t stride, hsize_t size);

  /*! Write internal face LR cell values to hum file **/
  template<typename uintT>
  void write(leftRight<uintT> *n, hsize_t offset, hsize_t stride, hsize_t size);

  /*! Write PatchBC information **/
  template<typename uintT>
  void write(std::string &patchName, patchBC<uintT> &patch);

  /*! Write PatchBC internal cell information **/
  template<typename uintT>
  void write
  (
    std::string &patchName, uintT *patchCells,
    hsize_t offset, hsize_t stride, hsize_t size
    );

  /** Private memebers **/
private:
  /*! \brief Make class member values to default */
  void set();

  /*! \brief Write node information to hum using offset/stride 
   **        uses the link information stored in the humT
   **/
  template < typename T, typename humT >
  void write(T *data, hsize_t offset,
    hsize_t stride, hsize_t mem_size,
    hsize_t _filesize);

  /*! \brief Write node information to hum using offset/stride 
   **        uses the link information stored in the humT
   **/
  template < typename T, typename humT >
  void write(T *data, h5pp::listString &link,
    hsize_t offset, hsize_t stride,
    hsize_t mem_size, hsize_t _filesize);

  /*! \brief Write node information to hum using offset/stride 
   **        uses the link information stored in the humT
   **/
  template < typename T >
  void write(T *data, h5pp::listString &link, hsize_t offset,
    hsize_t stride, hsize_t mem_size, hsize_t _filesize);

  /*! \brief Write node information to hum using offset/stride 
   **        uses the link information stored in the humT
   **        This is a general write which takes any arbitray type T
   **/
  template < typename T >
  void write(T *data, hid_t &mem_dtype, hid_t &_filedtype,
    h5pp::listString &link, hsize_t offset,
    hsize_t stride, hsize_t mem_size, hsize_t _filesize);

  /*! \brief Read all entity sizes from hum file */
  void read();

  /** Protected memebers **/
protected:
  hid_t _file; /*!< The hdf5 file handle */
  hsize_t _n_cell, _n_face, _n_node; /*!<  */
  hsize_t _n_internal_face, _n_face_adjncy; /*!<  */
  std::map< std::string, hsize_t > _n_patch_face; /*!<  */
  bool _is_open; /*!<  */
  int _int_size;
};

/***********   Implementation of template class  ****************/

ohstream::ohstream() {
  set();
}

ohstream::~ohstream() {
  close();
}

ohstream::ohstream(const char *fname) {
  set();
  open(fname);
}

void ohstream::open(const char *fname) {
  close();
  std::ifstream test_f(fname);
  if (test_f.fail()) {
    _file = H5Fcreate(fname, H5F_ACC_TRUNC,
      H5P_DEFAULT, H5P_DEFAULT);
    _is_open = true;
  } else {
    _file = H5Fopen(fname, H5F_ACC_RDWR,
      H5P_DEFAULT);
    _is_open = true;
    read();
  }
}

void ohstream::close() {
  if (_is_open == true) {
    H5Fclose(_file);
    set();
  }
}

template<typename floatT>
void ohstream::write(node<floatT> *n) {
  write< node<floatT>, H5TNode<floatT> >
    (
    n, 0, 1, _n_node, _n_node
    );
}

template<typename uintT>
void ohstream::write(face<uintT> *n) {
  write< face<uintT>, H5TFace<uintT> >
    (
    n, 0, 1, _n_face, _n_face
    );
}

template<typename floatT>
void ohstream::write(node<floatT> *n, hsize_t offset, hsize_t stride, hsize_t size) {
  write< node<floatT>, H5TNode<floatT> >
    (
    n, offset, stride, size, _n_node
    );
}

template<typename uintT>
void ohstream::write(face<uintT> *n, hsize_t offset, hsize_t stride, hsize_t size) {
  write< face<uintT>, H5TFace<uintT> >
    (
    n, offset, stride, size, _n_face
    );
}

template<typename uintT>
void ohstream::write(leftRight<uintT> *n, hsize_t offset, hsize_t stride, hsize_t size) {
  write< leftRight<uintT>, H5TLeftRight<uintT> >
    (
    n, offset, stride, size, _n_face
    );
}

template<typename uintT>
void ohstream::write(std::string &patchName, patchBC<uintT> &patch) {
  h5pp::listString link;
  H5TPatch<uintT> H5T;
  link.push_back(hum::patchLink[hum::PRIMARY]);
  link.push_back(patchName);
  link.push_back(H5T.linkStr());
  h5pp::WriteVectorDataSerial
    (
    _file, &patch, H5T.mem_t(), H5T.file_t(),
    link, 0, 1, 1, 1
    );
}

template<typename uintT>
void ohstream::write
(
  std::string &patchName, uintT *patchCells,
  hsize_t offset, hsize_t stride, hsize_t size
  ) {
  h5pp::listString link;
  H5TPatch<uintT> H5T;
  link.push_back(hum::patchLink[hum::PRIMARY]);
  link.push_back(patchName);
  link.push_back(hum::patchLink[hum::ENTITY]);

  assert((_n_patch_face.find(patchName) != _n_patch_face.end()));
  hid_t dtype = H5Tcopy(h5pp::GetHDF5Type<uintT>());
  h5pp::WriteVectorDataSerial
    (
    _file, patchCells, dtype, dtype, link, offset,
    stride, size, _n_patch_face[patchName]
    );
  H5Tclose(dtype);
}

/*** Private function members ***/
void ohstream::set() {
  _n_cell = 0;
  _n_face = 0;
  _n_node = 0;
  _n_internal_face = 0;
  _is_open = false;
  _int_size = 0;
}

template < typename T, typename humT >
void ohstream::write(T *data, hsize_t offset,
  hsize_t stride, hsize_t mem_size,
  hsize_t _filesize) {
  humT H5T;
  h5pp::listString link;
  link.push_back(H5T.linkStr());
  write<T, humT>(data, link, offset, stride, mem_size, _filesize);
}

template < typename T, typename humT >
void ohstream::write(T *data, h5pp::listString &link,
  hsize_t offset, hsize_t stride,
  hsize_t mem_size, hsize_t _filesize) {
  humT H5T;
  write(data, H5T.mem_t(), H5T.file_t(), link, offset,
    stride, mem_size, _filesize);
}

template < typename T >
void ohstream::write(T *data, h5pp::listString &link,
  hsize_t offset, hsize_t stride,
  hsize_t mem_size, hsize_t _filesize) {
  assert(_is_open);
  hid_t dtype = H5Tcopy(h5pp::GetHDF5Type<T>());
  h5pp::WriteVectorDataSerial
    (
    _file, data, dtype, dtype,
    link, offset,
    stride, mem_size, _filesize
    );
  H5Tclose(dtype);
}

template < typename T >
void ohstream::write(T *data, hid_t &mem_dtype, hid_t &_filedtype,
  h5pp::listString &link, hsize_t offset,
  hsize_t stride, hsize_t mem_size, hsize_t _filesize) {
  assert(_is_open);
  h5pp::WriteVectorDataSerial
    (
    _file, data, mem_dtype, _filedtype,
    link, offset, stride, mem_size, _filesize
    );
}

void ohstream::read() {
  if (_is_open == false) return;
  h5pp::listString link;
  /// Get _n_node
  link.push_back(hum::nodeLink[hum::PRIMARY]);
  _n_node = h5pp::GetVectorLength<hsize_t>(_file, link);
  link.pop_back();
  /// Get _n_face
  link.push_back(hum::faceLink[hum::PRIMARY]);
  _n_face = h5pp::GetVectorLength<hsize_t>(_file, link);
  link.pop_back();
  /// Get _n_internal_face
  link.push_back(hum::miscLink[hum::FIELD]);
  _n_internal_face = h5pp::ReadAttribute<hsize_t>(_file, link);
  link.pop_back();
  /// Get _n_cell
  link.push_back(hum::miscLink[hum::PRIMARY]);
  _n_cell = h5pp::ReadAttribute<hsize_t>(_file, link);
  link.pop_back();
  /// Get face adj size
  link.push_back(hum::miscLink[hum::SECONDARY]);
  _n_face_adjncy = h5pp::ReadAttribute<hsize_t>(_file, link);
  link.pop_back();
  /// Get the integer size
  link.push_back(hum::miscLink[hum::ENTITY]);
  _int_size = h5pp::GetAttributeType(_file, link);
  link.pop_back();
  /// Debug print
  std::cerr << "_n_cell = " << _n_cell << "\n"
    << "_n_face = " << _n_face << "\n"
    << "_n_node = " << _n_node << "\n"
    << "_n_internal_face = " << _n_internal_face << std::endl;
}

#endif


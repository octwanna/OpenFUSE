/*! \file ihstream.hpp
 ** Input file interface for HUM mesh files. It can also write to the files 
 ** based on user parameter passed but it requires the existence of a HUM mesh
 ** file. The stream interface uses this header file.
 **/
 
#ifndef IHSTREAM_HPP

#define IHSTREAM_HPP

#include "types.hpp"
#include <vector>

namespace OF {
  template<typename floatT, typename uintT, typename HashFun>
  class dimm;
  template<typename floatT, typename uintT, typename HashFun>
  class cdimm;
}

/*! \brief hum input file stream class
 **        It provides stream access to the hum
 **        file very similar to C++ ofstream
 **/
class ihstream {
  template<typename T>
  friend class faceLeftRightStreamer;

  template<typename T>
  friend class faceStreamer;

  template<typename T>
  friend class nodeStreamer;

  template<typename floatT, typename uintT, typename HashFun>
  friend class OF::dimm;

  template<typename floatT, typename uintT, typename HashFun>
  friend class OF::cdimm;

  friend class patchInfo;

public:
  /*! \brief Empty construcor */
  ihstream();

  /*! \brief Empty construcor */
  ~ihstream();

  /*! \brief hum output file for serial write */
  ihstream(const char *fname);

  /*! \brief hum output file for serial write */
  ihstream(const char *fname, bool read_write);

  /*! \brief hum output file for serial write */
  ihstream(const char *fname, MPI_Comm &comm);

  /*! \brief hum output file for serial write */
  ihstream(const char *fname, MPI_Comm &comm, bool read_write);

  /*! \brief Open hum output file for serial write */
  void open(const char *fname);

  /*! \brief Open hum output file for serial write */
  void open(const char *fname, bool read_write);

  /*! \brief Open hum output file for serial write */
  void open(const char *fname, MPI_Comm &comm);

  /*! \brief Open hum output file for serial write */
  void open(const char *fname, MPI_Comm &comm, bool read_write);

  /*! \brief Close hum output file */
  void close();

  /*! Get total nodes in file **/
  hsize_t &nNode();

  /*! Get the MPI communicator **/
  MPI_Comm &get_comm();

  /*! Get the integer size of gids in hum */
  int &get_int_size();

  /*! Get total faces in file **/
  hsize_t &nFace();

  /*! Get total faces in file **/
  hsize_t &nCell();

  /*! Get total faces in file **/
  hsize_t &nInternalFace();

  /*! Get total faces in file **/
  hsize_t &nFaceAdjncy();

  /*! Get total faces in file **/
  hsize_t nPatch();

  /*! Get patch info by giving its index **/
  const patchBC<hsize_t> &get_patch_info(hsize_t num) const;

  /*! Get patch name by giving its index **/
  const std::string &get_patch_name(hsize_t num) const;

  /*! \brief Read all nodes from hum file **/
  template < typename floatT >
  void read(node<floatT> *data);

  /*! \brief Read all faces from hum file **/
  template < typename uintT >
  void read(face<uintT> *data);

  /*! \brief Read all internal face LR cell from hum file **/
  template < typename uintT >
  void read(leftRight<uintT> *data);

  /*! \brief Read node information from hum using   list
   **        uses the link information stored in the humT
   **/
  template < typename T, typename humT >
  void read(T *data, hsize_t *list, hsize_t listSize);

  /*! \brief Read Bounding box information **/
  template < typename floatT >
  void read(node<floatT> &min, node<floatT> &max);

  /*! Write node values to hum file **/
  template<typename floatT>
  void write(node<floatT> *n);

  /*! Return the file hid_t */
  hid_t &file();

  /** Protected members **/
protected:
  hid_t _file; /*!< The hdf5 file handle */
  hsize_t _n_cell, _n_face, _n_node; /*!<  */
  hsize_t _n_internal_face, _n_face_adjncy; /*!<  */
  patchBCMap<hsize_t>::Type _patch; /*!<  */
  bool _is_open, _is_parallel; /*!<  */
  hsize_t _max_patch_face;
  std::vector< patchBC<hsize_t> > _patch_info_by_num;
  std::vector< std::string > _patch_name_by_num;
  MPI_Comm _mpi_comm;
  int _int_size;

  /** Private members **/
private:
  /*! \brief Make class member values to default */
  void set();

  /*! \brief Read all entity sizes from hum file */
  void read_size();

  /*! \brief Read node information from hum using offset/stride 
   **        uses the link information stored in the humT
   **/
  template < typename T, typename humT >
  void read(T *data, hsize_t offset,
    hsize_t stride, hsize_t size);

  /*! \brief Read node information from hum using offset/stride */
  template < typename T, typename humT >
  void read(T *data, h5pp::listString &link, hsize_t offset,
    hsize_t stride, hsize_t size);

  /*! \brief Read node information from hum using offset/stride */
  template < typename T >
  void read(T *data, h5pp::listString &link, hsize_t offset,
    hsize_t stride, hsize_t size);

  /*! \brief Read node information from hum using offset/stride */
  template < typename T >
  void read(T *data, hid_t mem_dtype, h5pp::listString &link,
    hsize_t offset, hsize_t stride, hsize_t size);

  /*! \brief Read node information from hum using a list */
  template < typename T, typename humT >
  void read(T *data, h5pp::listString &link, hsize_t listSize, hsize_t *list);

  /*! \brief Read node information from hum using a list */
  template < typename T >
  void read(T *data, h5pp::listString &link,
    hsize_t listSize, hsize_t *list);

  /*! \brief Read node information from hum using a list */
  template < typename T >
  void read(T *data, hid_t &mem_dtype, h5pp::listString &link,
    hsize_t listSize, hsize_t *list);

  /***** streamer specific functions *****/

  /*! \brief Writes the face info to file (r/w mode) */
  template< typename uintT >
  void write(face<uintT> *f, hsize_t offset, hsize_t stride, hsize_t size);

  /*! \brief Writes the faceL/R info to file (r/w mode) */
  template< typename uintT >
  void write(leftRight<uintT> *lr, hsize_t offset, hsize_t stride, hsize_t size);

  /*! \brief Writes the faceL/R info to file (r/w mode) */
  template< typename uintT >
  void write(node<uintT> *n, hsize_t offset, hsize_t stride, hsize_t size);

  /*! \brief Writes the faceL/R info to file (r/w mode) */
  template< typename uintT >
  void write(node<uintT> *n, hsize_t *list, hsize_t listSize);

  /*! \brief Writes the faceL/R info to file (r/w mode) */
  template< typename T, typename humT >
  void write(T *data, hsize_t offset, hsize_t stride, hsize_t mem_size, hsize_t file_size);

  /*! \brief Writes the faceL/R info to file (r/w mode) */
  template< typename T >
  void write(T *data, h5pp::listString &link, hsize_t offset,
    hsize_t stride, hsize_t mem_size, hsize_t file_size);

};

/*************** Implementation goes here ***********************/
ihstream
::ihstream()
: _n_cell(0), _n_face(0),
_n_node(0), _n_internal_face(0),
_n_face_adjncy(0), _is_open(false),
_is_parallel(false), _int_size(0) {
  // Empty
}

ihstream
::~ihstream() {
  close();
}

ihstream
::ihstream(const char *fname)
: _n_cell(0), _n_face(0),
_n_node(0), _n_internal_face(0),
_n_face_adjncy(0), _is_open(true),
_is_parallel(false) {
  _file = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
  read_size();
}

ihstream
::ihstream(const char *fname, bool read_write)
: _n_cell(0), _n_face(0),
_n_node(0), _n_internal_face(0),
_n_face_adjncy(0), _is_open(true),
_is_parallel(false) {
  unsigned h5_mode = (read_write == true) ? H5F_ACC_RDWR : H5F_ACC_RDONLY;
  _file = H5Fopen(fname, h5_mode, H5P_DEFAULT);
  read_size();
}

ihstream
::ihstream(const char *fname, MPI_Comm &comm)
: _n_cell(0), _n_face(0),
_n_node(0), _n_internal_face(0),
_n_face_adjncy(0), _is_open(true),
_is_parallel(true),
_mpi_comm(comm) {
  hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, MPI_INFO_NULL);
  _file = H5Fopen(fname, H5F_ACC_RDONLY, plist_id);
  H5Pclose(plist_id);
  read_size();
}

ihstream
::ihstream(const char *fname, MPI_Comm &comm, bool read_write)
: _n_cell(0), _n_face(0),
_n_node(0), _n_internal_face(0),
_n_face_adjncy(0), _is_open(true),
_is_parallel(true),
_mpi_comm(comm) {
  hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, MPI_INFO_NULL);
  unsigned h5_mode = (read_write == true) ? H5F_ACC_RDWR : H5F_ACC_RDONLY;
  _file = H5Fopen(fname, h5_mode, plist_id);
  H5Pclose(plist_id);
  read_size();
}

void ihstream
::open(const char *fname) {
  close();
  _file = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
  _is_open = true;
  read_size();
}

void ihstream
::open(const char *fname, bool read_write) {
  close();
  unsigned h5_mode = (read_write == true) ? H5F_ACC_RDWR : H5F_ACC_RDONLY;
  _file = H5Fopen(fname, h5_mode, H5P_DEFAULT);
  _is_open = true;
  read_size();
}

void ihstream
::open(const char *fname, MPI_Comm &comm) {
  close();
  _mpi_comm = comm;
  hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, MPI_INFO_NULL);
  _file = H5Fopen(fname, H5F_ACC_RDONLY, plist_id);
  H5Pclose(plist_id);
  _is_open = true;
  read_size();
  _is_parallel = true;
}

void ihstream
::open(const char *fname, MPI_Comm &comm, bool read_write) {
  close();
  _mpi_comm = comm;
  hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, MPI_INFO_NULL);
  unsigned h5_mode = (read_write == true) ? H5F_ACC_RDWR : H5F_ACC_RDONLY;
  _file = H5Fopen(fname, h5_mode, plist_id);
  H5Pclose(plist_id);
  _is_open = true;
  read_size();
  _is_parallel = true;
}

void ihstream
::close() {
  if (_is_open == true) {
    H5Fclose(_file);
    set();
  }
}

hsize_t &ihstream
::nNode() {
  return _n_node;
}

MPI_Comm &ihstream
::get_comm() {
  return _mpi_comm;
}

int &ihstream
::get_int_size() {
  return _int_size;
}

hsize_t &ihstream
::nFace() {
  return _n_face;
}

hsize_t &ihstream
::nCell() {
  return _n_cell;
}

hsize_t &ihstream
::nInternalFace() {
  return _n_internal_face;
}

hsize_t &ihstream
::nFaceAdjncy() {
  return _n_face_adjncy;
}

hsize_t ihstream
::nPatch() {
  return _patch.size();
}

const patchBC<hsize_t> &ihstream
::get_patch_info(hsize_t num) const {
  return _patch_info_by_num[num];
}

const std::string &ihstream
::get_patch_name(hsize_t num) const {
  return _patch_name_by_num[num];
}

template < typename floatT >
void ihstream
::read(node<floatT> *data) {
  read< node<floatT>, H5TNode<floatT> >(data, 0, 1, _n_node);
}

template < typename uintT >
void ihstream
::read(face<uintT> *data) {
  read< face<uintT>, H5TFace<uintT> >(data, 0, 1, _n_face);
}

template < typename uintT >
void ihstream
::read(leftRight<uintT> *data) {
  read< leftRight<uintT>, H5TLeftRight<uintT> >(data, 0, 1, _n_face);
}

/*! \brief Read Bounding box information **/
template < typename floatT >
void ihstream
::read(node<floatT> &min, node<floatT> &max) {
  H5TNode<floatT> my_h5t;
  h5pp::listString link;
  /// Read AABB min attribute
  link.push_back(hum::AABBLink[hum::PRIMARY]);
  h5pp::ReadAttribute(_file, link, min, my_h5t.mem_t());
  /// Read AABB max attribute
  link.pop_back();
  link.push_back(hum::AABBLink[hum::SECONDARY]);
  h5pp::ReadAttribute(_file, link, max, my_h5t.mem_t());
}

/*** Private member functions *****/
void ihstream
::set() {
  _n_cell = 0;
  _n_face = 0;
  _n_node = 0;
  _n_internal_face = 0;
  _n_face_adjncy = 0;
  _is_open = false;
  _is_parallel = false;
}

void ihstream
::read_size() {
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
  /// Get the face adjncy
  link.push_back(hum::miscLink[hum::SECONDARY]);
  _n_face_adjncy = h5pp::ReadAttribute<hsize_t>(_file, link);
  link.pop_back();
  /// Get the integer size
  link.push_back(hum::miscLink[hum::ENTITY]);
  _int_size = h5pp::GetAttributeType(_file, link);
  link.pop_back();
#if 0 // Debug print
  std::cerr << "_n_cell = " << _n_cell << "\n"
    << "_n_face = " << _n_face << "\n"
    << "_n_node = " << _n_node << "\n"
    << "_n_internal_face = " << _n_internal_face << std::endl;
#endif
  //// Read patch information
  link.push_back(hum::patchLink[hum::PRIMARY]);
  hsize_t npatch = h5pp::GetSubGroupSize(_file, link);
  _max_patch_face = 0;
  _patch_info_by_num.reserve(npatch);
  _patch_name_by_num.reserve(npatch);
  for (hsize_t i = 0; i < npatch; ++i) {
    std::string patch_name = h5pp::GetSubGroupName(_file, i, link);
    link.push_back(patch_name);
    link.push_back(hum::patchLink[hum::SECONDARY]);
    read< patchBC<hsize_t>, H5TPatch<hsize_t> >
      (
      &_patch[patch_name], link, 0, 1, 1
      );

    _patch_info_by_num.push_back(_patch[patch_name]);
    _patch_name_by_num.push_back(patch_name);
#if 0 /// Debug print
    std::cerr << patch_name << " "
      << _patch[patch_name].startFace
      << " " << _patch[patch_name].faceCount
      << "\n";
#endif
    link.pop_back();
    link.pop_back();
    if (_patch[patch_name].faceCount > _max_patch_face)
      _max_patch_face = _patch[patch_name].faceCount;
  }
}

template < typename T, typename humT >
void ihstream
::read(T *data, hsize_t offset,
  hsize_t stride, hsize_t size) {
  humT H5T;
  h5pp::listString link;
  link.push_back(H5T.linkStr());
  read<T, humT>(data, link, offset, stride, size);
}

template < typename T, typename humT >
void ihstream
::read(T *data, h5pp::listString &link, hsize_t offset,
  hsize_t stride, hsize_t size) {
  humT H5T;
  read(data, H5T.mem_t(), link, offset, stride, size);
}

template < typename T >
void ihstream
::read(T *data, h5pp::listString &link, hsize_t offset,
  hsize_t stride, hsize_t size) {
  read(data, h5pp::GetHDF5Type<T>(), link, offset, stride, size);
}

template < typename T >
void ihstream
::read(T *data, hid_t mem_dtype, h5pp::listString &link,
  hsize_t offset, hsize_t stride, hsize_t size) {
  assert(_is_open);
  if (_is_parallel == false)
    h5pp::ReadVectorDataSerial
    (
    _file, data, mem_dtype, link,
    offset, stride, size
    );
  else
    h5pp::ReadVectorData
    (
    _file, data, mem_dtype, link,
    offset, stride, size
    );
}

template < typename T, typename humT >
void ihstream
::read(T *data, hsize_t *list, hsize_t listSize) {
  humT H5T;
  h5pp::listString link;
  link.push_back(H5T.linkStr());
  read<T, humT>(data, link, listSize, list);
}

template < typename T, typename humT >
void ihstream
::read(T *data, h5pp::listString &link, hsize_t listSize, hsize_t *list) {
  humT H5T;
  read(data, H5T.mem_t(), link, listSize, list);
}

template < typename T >
void ihstream
::read(T *data, h5pp::listString &link,
  hsize_t listSize, hsize_t *list) {
  read(data, h5pp::GetHDF5Type<T>(), link, listSize, list);
}

template < typename T >
void ihstream
::read(T *data, hid_t &mem_dtype, h5pp::listString &link,
  hsize_t listSize, hsize_t *list) {
  assert(_is_open);
  if (_is_parallel == false)
    h5pp::ReadVectorDataSerial
    (
    _file, data, mem_dtype,
    link, listSize, list
    );
  else
    h5pp::ReadVectorData
    (
    _file, data, mem_dtype,
    link, listSize, list
    );
}

/*! Write node values to hum file **/
template<typename floatT>
void ihstream
::write(node<floatT> *n) {
  write< node<floatT>, H5TNode<floatT> >
    (
    n, 0, 1, _n_node, _n_node
    );
}

/****** Streamer Specific functions ************/

/*! \brief Writes the face info to file (r/w mode) */
template< typename uintT >
void ihstream
::write(face<uintT> *f, hsize_t offset, hsize_t stride, hsize_t size) {
  write< face<uintT>, H5TFace<uint> >
    (
    f, offset, stride, size, _n_face
    );
}

/*! \brief Writes the faceL/R info to file (r/w mode) */
template< typename uintT >
void ihstream
::write(leftRight<uintT> *lr, hsize_t offset, hsize_t stride, hsize_t size) {
  write< leftRight<uintT>, H5TLeftRight<uintT> >
    (
    lr, offset, stride, size, _n_face
    );
}

/*! \brief Writes the faceL/R info to file (r/w mode) */
template< typename floatT >
void ihstream
::write(node<floatT> *n, hsize_t offset, hsize_t stride, hsize_t size) {
  write< node<floatT>, H5TNode<floatT> >
    (
    n, offset, stride, size, _n_node
    );
}

/*! \brief Writes the template data info to file (r/w mode) */
template< typename T, typename humT >
void ihstream
::write(T *data, hsize_t offset, hsize_t stride, hsize_t mem_size, hsize_t file_size) {
  assert(_is_open);
  humT H5T;
  h5pp::listString link;
  link.push_back(H5T.linkStr());
  if (_is_parallel == false)
    h5pp::WriteVectorDataSerial
    (
    _file, data, H5T.mem_t(), H5T.file_t(),
    link, offset, stride, mem_size, file_size
    );
  else
    h5pp::WriteVectorData
    (
    _file, data, H5T.mem_t(), H5T.file_t(),
    link, offset, stride, mem_size, file_size
    );
}

/*! \brief Writes the template data info to file (r/w mode) 
 **         uses link information
 */
template< typename T >
void ihstream
::write(T *data, h5pp::listString &link, hsize_t offset,
  hsize_t stride, hsize_t mem_size, hsize_t file_size) {
  assert(_is_open);
  hid_t type = H5Tcopy(h5pp::GetHDF5Type<T>());
  if (_is_parallel == false)
    h5pp::WriteVectorDataSerial
    (
    _file, data, type, type,
    link, offset, stride, mem_size, file_size
    );
  else
    h5pp::WriteVectorData
    (
    _file, data, type, type,
    link, offset, stride, mem_size, file_size
    );
  H5Tclose(type);
}

/*! \brief Return the file hid_t
 */
hid_t &ihstream
::file() {
  return _file;
}
#endif


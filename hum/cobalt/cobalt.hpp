/*! \file cobalt.hpp
 ** The cobalt mesh file interface for conversion into hum file format
 **/
 
#ifndef COBALT_HPP

#define COBALT_HPP

#include "ohstream.hpp"
#include <fstream>
#include <vector>
#include <cstdlib>
#include <limits>
#include <algorithm>

template < typename floatT, typename uintT >
class COBALT : public ohstream {
public:
  /// \brief Constructor for the mesh class
  COBALT(const char *cFile, const char *humFile, double limit);
  /// \brief Destructor for the mesh class
  ~COBALT();
  /// \brief Start the file parser 
  void Start();

private:
  std::ifstream _in_file;
  std::streampos _face_beg;
  size_t _max_bytes;
  //  bool _has_tri, _has_quad, _is_hybrid ;
  std::map< uintT, hsize_t > _n_patch_face_cobalt;
  std::map< uintT, hsize_t > _patch_offset;
  typename std::map< uintT, std::vector< face<uintT> > > _patch_face;
  typename std::map< uintT, std::vector< leftRight<uintT> > > _patch_internal_cell;
  hsize_t _count_face, _count_node, _count_internal_face;
  node<floatT> _min, _max;

  /*! \brief Read/write the vertex data and dump 
   **        to HDF5 file
   **/
  void FirstPass();
  /*! \brief Parse face data and get sizes **/
  void SecondPass();
  /*! \brief Read/write internal faces and patch faces **/
  void ThirdPass();
  /*! \brief Read the list of faces from cobalt file **/
  inline void ReadOneFace(face<uintT> &f, int64_t &left, int64_t &right);
#if 0 // No longer necessary
  /*! \brief Check if file has hybrid mesh entities **/
  inline void CheckHybrid(face<uintT> &f);
#endif
  /*! \brief Forms the patch bc infomation by parsing all faces in cobalt file **/
  inline void FormPatchInfo();
  /*! \brief */
  void WritePatchFaceUsingOffset();
  /*! \brief */
  void WritePatchInternalCellOffset();
  /*! \brief */
  inline void WriteCellSize();
  /*! \brief */
  inline void WriteAABB();
  /*! \brief */
  inline void IncrementPatchOffset();
  /*! \brief */
  inline hsize_t GetNodeChunkSize();
  /*! \brief */
  inline hsize_t GetNumNodeChunks();
  /*! \brief */
  inline hsize_t GetFaceChunkSize();
  /*! \brief */
  inline hsize_t GetNumFaceChunks();

};

/***** Class function implementation *******/
template < typename floatT, typename uintT >
COBALT<floatT, uintT>
::COBALT(const char *cFile, const char *humFile, double limit)
:
//  _has_tri(false), _has_quad(false), _is_hybrid(false),
_in_file(cFile),
_max_bytes(size_t(limit * 1024.0e0 * 1024.0e0 * 1024.0e0)),
_count_face(0), _count_node(0), _count_internal_face(0) {
  //  _in_file.open( cFile );
  if (_in_file.fail()) {
    std::cerr << "Error: Unable to open " << cFile << "\n";
    //throw IOException();
    exit(1);
  }
  /// Limit is given in GB so convert it to bytes
  std::cerr << "Total bytes allocated from buffer = "
    << _max_bytes << "(" << limit << " GB)\n";
  open(humFile);
}

template < typename floatT, typename uintT >
COBALT<floatT, uintT>
::~COBALT() {
  /* empty */
}

template < typename floatT, typename uintT >
void COBALT<floatT, uintT>
::Start() {
  FirstPass();
  SecondPass();
  ThirdPass();
}

/****** Private Function *******/

template < typename floatT, typename uintT >
void COBALT<floatT, uintT>
::FirstPass() {
  std::cerr << "Pass 1 - Vertex Data\n";
  int ndim, nmesh, nfacespercell, npatch, ncorn;
  hsize_t chunk_size;

  _in_file >> ndim >> nmesh >> npatch;
  _in_file >> _n_node >> _n_face >> _n_cell >> ncorn >> nfacespercell;
  std::cerr << "Total Nodes = " << _n_node << "\n";
  std::cerr << "Total Faces = " << _n_face << "\n";
  std::cerr << "Total Cells = " << _n_cell << "\n";
  WriteCellSize();

  /// Allocate memory for node
  unsigned last_chunk_size = _n_node - GetNumNodeChunks() * GetNodeChunkSize();
  std::vector< node<floatT> > temp_node(GetNodeChunkSize());
  /// Set AABB to max/min of floatT type
  for (unsigned i = 0; i < 3; ++i) {
    _min.xyz[i] = std::numeric_limits<floatT>::max();
    _max.xyz[i] = std::numeric_limits<floatT>::min();
  }
  /// Stream COBALT ASCII file
  for (int i = 0; i < GetNumNodeChunks() + 1; ++i) {
    if (i == GetNumNodeChunks()) chunk_size = last_chunk_size;
    else chunk_size = GetNodeChunkSize();
    for (hsize_t j = 0; j < chunk_size; ++j) {
      _in_file >> temp_node[j].xyz[0]
        >> temp_node[j].xyz[1]
        >> temp_node[j].xyz[2];
      /// Find AABB min/max coord
      for (unsigned k = 0; k < 3; ++k) {
        _min.xyz[k] = std::min(_min.xyz[k], temp_node[j].xyz[k]);
        _max.xyz[k] = std::max(_max.xyz[k], temp_node[j].xyz[k]);
      }
    }
    write(&temp_node[0], _count_node, 1, chunk_size);
    _count_node += chunk_size;
    std::cerr << "Written chunk " << i + 1
      << " of " << GetNumNodeChunks() + 1 << "\n";
  } // End Loop over chunks
  /// Add tolerance
  for (unsigned k = 0; k < 3; ++k) {
    _min.xyz[k] -= common::GetTolerance<floatT>();
    _max.xyz[k] += common::GetTolerance<floatT>();
  }
  /// Write the AABB as an attribute
  WriteAABB();
}

template < typename floatT, typename uintT >
void COBALT<floatT, uintT>
::SecondPass() {
  std::cerr << "Pass 2 - Face Data Sizes\n";
  int64_t left, right;
  face<uintT> temp_face;
  _n_face_adjncy = 0;
  /// Record the face file pointer
  _face_beg = _in_file.tellg();
  _n_internal_face = 0;
  for (uintT i = 0; i < _n_face; ++i) {
    ReadOneFace(temp_face, left, right);
    if (right >= 0)
      _n_internal_face++;
    else
      _n_patch_face_cobalt[ std::abs(right) ]++;
    //CheckHybrid( temp_face );
    _n_face_adjncy += temp_face.bField;
  }
  /// Write the face adjncy data to file
  h5pp::listString link;
  link.push_back(hum::miscLink[hum::SECONDARY]);
  h5pp::WriteAttribute(_file, link, _n_face_adjncy);
  /// Write the internal face size info to file
  link.pop_back();
  link.push_back(hum::miscLink[hum::FIELD]);
  h5pp::WriteAttribute(_file, link, _n_internal_face);
  /// Write integer type infomation
  link.pop_back();
  link.push_back(hum::miscLink[hum::ENTITY]);
  uintT IntegerT = sizeof (uintT);
  h5pp::WriteAttribute(_file, link, IntegerT);
  /// Patch information to file
  FormPatchInfo();
  std::cerr << "Total internal faces = " << _n_internal_face << "\n";
  std::cerr << "Face adjncy size     = " << _n_face_adjncy << "\n";
}

template < typename floatT, typename uintT >
void COBALT<floatT, uintT>
::ThirdPass() {
  std::cerr << "Pass 3 - Face Data\n";
  unsigned num_chunks = GetNumFaceChunks();
  unsigned chunk_size = GetFaceChunkSize();
  unsigned last_chunk_size = _n_face - num_chunks * chunk_size;

  std::vector< face<uintT> > face_buf;
  face<uintT> temp_face;
  std::vector< leftRight<uintT> > lr_buf;
  int64_t left, right;

  _in_file.seekg(_face_beg);

  face_buf.resize(chunk_size);
  lr_buf.resize(chunk_size);

  for (int i = 0; i < num_chunks + 1; ++i) {
    face<uintT> *facePtr = &face_buf[0];
    leftRight<uintT> *ptr_lr = &lr_buf[0];
    if (i == num_chunks) chunk_size = last_chunk_size;
    for (unsigned j = 0; j < chunk_size; ++j) {
      ReadOneFace(temp_face, left, right);
      if (right >= 0) {
        *(facePtr) = temp_face;
        ++facePtr;
#if 0 // Bad idea
        if (_is_hybrid == true && temp_face.bField == 4)
          std::swap(left, right);
#endif
        ptr_lr->left = left;
        ptr_lr->right = right;
        ++ptr_lr;
      } else {
        _patch_face[std::abs(right)].push_back(temp_face);
        ptr_lr->left = left;
        ptr_lr->right = 0;
        _patch_internal_cell[std::abs(right)].push_back(*ptr_lr);
      }
    }
    /// Write the face-vertex connectivity
    hsize_t chunksRead = facePtr - &face_buf[0];
    if (chunksRead > 0) write(&face_buf[0], _count_face, 1, chunksRead);
    _count_face += chunksRead;
    /// Write the face LR cell ID
    chunksRead = ptr_lr - &lr_buf[0];
    if (chunksRead > 0) write(&lr_buf[0], _count_internal_face, 1, chunksRead);
    _count_internal_face += chunksRead;
    /// Write the patch face and internal cell data
    WritePatchFaceUsingOffset();
    WritePatchInternalCellOffset();
    IncrementPatchOffset();
    std::cerr << "Written face chunk " << i + 1 << " of " << num_chunks + 1 << "\n";
  }
}

template < typename floatT, typename uintT >
inline void COBALT<floatT, uintT>
::ReadOneFace(face<uintT> &f, int64_t &left, int64_t &right) {
  _in_file >> f.bField;
  if (f.bField == 3) {
    for (int i = f.bField - 1; i >= 0; --i) {
      _in_file >> f.entityID[i];
      f.entityID[i]--;
    }
  } else {
    for (int i = 0; i < f.bField; ++i) {
      _in_file >> f.entityID[i];
      f.entityID[i]--;
    }
  }
  _in_file >> left >> right;
  left--;
  right--;
}

#if 0 /// No longer necessary

template < typename floatT, typename uintT >
inline void COBALT<floatT, uintT>
::CheckHybrid(face<uintT> &f) {
  if (f.bField == 3)
    _has_tri = true;
  else if (f.bField == 4)
    _has_quad = true;
  if (_has_tri == true && _has_quad == true)
    _is_hybrid = true;
}
#endif

template < typename floatT, typename uintT >
inline void COBALT<floatT, uintT>
::FormPatchInfo() {
  hsize_t offset = _n_internal_face;
  patchBC<uintT> temp_patch;
  for (typename std::map< uintT, hsize_t >::iterator it = _n_patch_face_cobalt.begin();
    it != _n_patch_face_cobalt.end(); ++it) {
    std::stringstream cat;
    cat << "patch_" << it->first;
    std::string tempStr = cat.str();

    _n_patch_face[ tempStr ] = it->second;
    temp_patch.startFace = offset;
    temp_patch.bcType = 1;
    temp_patch.faceCount = it->second;
    temp_patch.attachedToProcID = 0;
    write(tempStr, temp_patch);
    _patch_offset[ it->first ] = offset;
    offset += it->second;
  }
#if 0 // no longer necessary
  if (_is_hybrid == true)
    std::cerr << "Hybrid cell types in mesh\n";
  else
    std::cerr << "Uniform cell types in mesh\n";
#endif
}

template < typename floatT, typename uintT >
void COBALT<floatT, uintT>
::WritePatchFaceUsingOffset() {
  typename std::map< uintT, hsize_t >::iterator it;
  for (it = _patch_offset.begin(); it != _patch_offset.end(); ++it) {
    if (_patch_face[it->first].size() > 0) {
      write(&(_patch_face[it->first][0]), it->second, 1, _patch_face[it->first].size());
    }
  }
}

template < typename floatT, typename uintT >
void COBALT<floatT, uintT>
::WritePatchInternalCellOffset() {
  typename std::map< uintT, hsize_t >::iterator it;
  for (it = _patch_offset.begin(); it != _patch_offset.end(); ++it) {
    if (_patch_internal_cell[it->first].size() > 0) {
      write(&(_patch_internal_cell[it->first][0]), it->second, 1, _patch_face[it->first].size());
    }
  }
}

template < typename floatT, typename uintT >
inline void COBALT<floatT, uintT>
::IncrementPatchOffset() {
  typename std::map< uintT, hsize_t >::iterator it;
  for (it = _patch_offset.begin(); it != _patch_offset.end(); ++it)
    it->second += _patch_face[it->first].size();
  _patch_internal_cell.clear();
  _patch_face.clear();
}

template < typename floatT, typename uintT >
inline void COBALT<floatT, uintT>
::WriteCellSize() {
  h5pp::listString link;
  link.push_back(hum::miscLink[hum::PRIMARY]);
  h5pp::WriteAttribute(_file, link, _n_cell);
}

template < typename floatT, typename uintT >
inline void COBALT<floatT, uintT>
::WriteAABB() {
  h5pp::listString link;
  H5TNode<floatT> my_h5_node;
  /// Write attributes to file
  link.push_back(hum::AABBLink[hum::PRIMARY]);
  h5pp::WriteAttribute(_file, link, _min, my_h5_node.mem_t());
  link.pop_back();
  link.push_back(hum::AABBLink[hum::SECONDARY]);
  h5pp::WriteAttribute(_file, link, _max, my_h5_node.mem_t());
}

template < typename floatT, typename uintT >
inline hsize_t COBALT<floatT, uintT>
::GetNodeChunkSize() {
  hsize_t size = _max_bytes / sizeof ( node<floatT>);
  if (size > _n_node) size = _n_node;
  return size;
}

template < typename floatT, typename uintT >
inline hsize_t COBALT<floatT, uintT>
::GetNumNodeChunks() {
  hsize_t num_chunks = _n_node / GetNodeChunkSize();
  return num_chunks;
}

template < typename floatT, typename uintT >
inline hsize_t COBALT<floatT, uintT>
::GetFaceChunkSize() {
  hsize_t size = _max_bytes / sizeof ( face<uintT>);
  if (size > _n_face) size = _n_face;
  return size;
}

template < typename floatT, typename uintT >
inline hsize_t COBALT<floatT, uintT>
::GetNumFaceChunks() {
  hsize_t num_chunks = _n_face / GetFaceChunkSize();
  return num_chunks;
}

#endif


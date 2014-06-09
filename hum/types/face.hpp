/*! \file face.h
 **       The header file containing the basic
 **       mesh entity shape face
 **/
#ifndef FACE_ENTITY_H

#define FACE_ENTITY_H

#include<algorithm>

namespace common {
  const unsigned face_bucket_size = 100000;
}

/*! \brief Face entity of the mesh
 **        Contains the edge IDs
 **        forming the face
 **/
template< typename uintT >
struct face {
  uintT bField;
  uintT entityID[4];
};

/*! \brief The internal cell left/right cell
 **        
 **        
 **/
template <typename uintT>
struct leftRight {
  uintT left;
  uintT right;

  /* Morton code ordering comparision */

  /* returns: true if (this < x) in Morton order */
  inline bool operator<(const leftRight &other) const {
    uintT x = 0, y = left ^ other.left;
    bool ret = left < other.left;
    if ((x < y) & (x < (x ^ y))) {
      x = y;
      y = right ^ other.right;
      if ((x < y) & (x < (x ^ y)))
        ret = (right < other.right);
    }
    return ret;
  }

};

/*! \brief The face permutation object
 **        used to generate cache local
 **        permutations for best locality
 **/
template <typename uintT>
struct faceReorderObject {
  uintT id;
  leftRight<uintT> *data;

  inline bool operator<(const faceReorderObject<uintT> &x) const {
    uintT cur_bucket =
      abs(int64_t(data->left) - int64_t(data->right)) / common::face_bucket_size;
    uintT x_bucket =
      abs(int64_t(x.data->left) - int64_t(x.data->right)) / common::face_bucket_size;
    /// If the buckets are equal
    if (cur_bucket == x_bucket)
      return std::min(data->left, data->right) < std::min(x.data->left, x.data->right);
    else if (cur_bucket > x_bucket)
      return false;
    else
      return true;
  }
};

/*! \brief The face permutation object
 **        used to generate cache local
 **        permutations for best locality
 **/
template <typename uintT>
struct faceReorderObjectSFC {
  uintT id;
  leftRight<uintT> *data;

  inline bool operator<(const faceReorderObjectSFC<uintT> &x) const {
    return *(data) < *(x.data);
  }

};

/*! \brief The comparision functor for sorting faces
 **         along SFC ordering for best locality
 **/
template <typename uintT>
struct faceCompareSFC {
public:
  /// Constructor

  faceCompareSFC(const leftRight<uintT> *data)
  : _data(data) {
    /* empty */
  }

  /* Morton code ordering comparision */

  /* returns: true if (this < x) in Morton order */
  inline bool operator()(uintT i, uintT j) const {
    uintT x = 0, y = _data[i].left ^ _data[j].left;
    bool ret = _data[i].left < _data[j].left;
    if ((x < y) & (x < (x ^ y))) {
      x = y;
      y = _data[i].right ^ _data[j].right;
      if ((x < y) & (x < (x ^ y)))
        ret = (_data[i].right < _data[j].right);
    }
    return ret;
  }

private:
  const leftRight<uintT> *_data;

};

/*! \brief The comparision functor for sorting faces
 **         along 2-l Lex ordering for best locality
 **/
template <typename uintT>
struct faceCompareLex {
public:
  /// Constructor

  faceCompareLex(const leftRight<uintT> *data)
  : _data(data) {
    /* empty */
  }

  /* Two level lexicographic ordering of Pavanakumar et al.*/

  /* returns: true if (this < x) in 2-l Lex order */
  inline bool operator()(uintT i, uintT j) const {
    uintT cur_bucket =
      abs(int64_t(_data[i].left) - int64_t(_data[i].right)) / common::face_bucket_size;
    uintT x_bucket =
      abs(int64_t(_data[j].left) - int64_t(_data[j].right)) / common::face_bucket_size;
    /// If the buckets are equal
    if (cur_bucket == x_bucket)
      return std::min(_data[i].left, _data[i].right) < std::min(_data[j].left, _data[j].right);
    else if (cur_bucket > x_bucket)
      return false;
    else
      return true;
  }

private:
  const leftRight<uintT> *_data;

};

#endif


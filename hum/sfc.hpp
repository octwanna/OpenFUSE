/*! \file sfc.hpp
 ** The Space Filling Curve re-ordering routines for HUM files.
 ** Both serial and parallel mesh ordering tools use these routines. 
 **/

#ifndef SFC_HPP

#define SFC_HPP

#include <bitset>
#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>

namespace sfc {

  /// The permutation object

  template<typename T, typename uintT>
  struct IdKey {
  public:
    uintT id;
    T key; /* Implements < operator */

    /*! \brief Less than operator for key **/
    inline bool operator<(const IdKey<T, uintT> &x) const {
      return ( this->key < x.key);
    }
  };

  /*! \brief The basic sfc index data-type can be sorted by
   **        STL sort by operator overloading
   **/
  struct index_t {
  public:
    /*! \param _key Stores the sfc index key value (x,y,z) **/
    uint64_t xyz[3];

    /*! \brief Less than operator for key **/
    inline bool operator<(const index_t &x) const {
      for (unsigned i = 0; i < 3; ++i) {
        if (xyz[i] >= x.xyz[i]) return false;
        if (xyz[i] < x.xyz[i]) return true;
      }
      return true;
    }
  };

  /*! \brief Inplace permuation of array O(N)
   **        no extra memory and O(N) in time
   **        from Donald Knuth's TAOCP V3
   **/
  template< typename T, typename uintT >
  void InplacePermutation(T *data, uintT *perm, size_t size) {
    T temp;
    uintT j, k;
    for (uintT i = 0; i < size; ++i) {
      if (i != perm[i]) {
        temp = data[i];
        j = i;
        while (i != perm[j]) {
          k = perm[j];
          data[j] = data[k];
          perm[j] = j;
          j = k;
        }
        data[j] = temp;
        perm[j] = j;
      }
    }
  }

  /*! \brief Inplace permuation of array O(N)
   **        no extra memory and O(N) in time
   **        from Donald Knuth's TAOCP V3
   **        (Overloaded version)
   **/
  template < typename T1, typename T2, typename uintT >
  void InplacePermutation(T1 *data1, T2 *data2, uintT *perm, size_t size) {
    T1 temp1;
    T2 temp2;
    uintT j, k;
    for (uintT i = 0; i < size; ++i) {
      if (i != perm[i]) {
        temp1 = data1[i];
        temp2 = data2[i];
        j = i;
        while (i != perm[j]) {
          k = perm[j];
          data1[j] = data1[k];
          data2[j] = data2[k];
          perm[j] = j;
          j = k;
        }
        data1[j] = temp1;
        data2[j] = temp2;
        perm[j] = j;
      }
    }
  }

  enum SFC_NUM_BITS {
    _10BIT, _20BIT, _NBIT
  };

  enum STORAGE_ORDER {
    ROW_MAJOR, COLUMN_MAJOR
  };

  /// The sfcFunctor class for calculating the sfc coding

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT = _10BIT,
  STORAGE_ORDER sorderT = ROW_MAJOR
  >
  struct sfcFunctor {
  public:
    /// Bounding box constructors
    sfcFunctor(floatT *min, floatT *max);
    /// Destructor
    ~sfcFunctor();
    /// The tbb parallel for functor
    void operator()(const tbb::blocked_range<uintT> &range) const;
    /// Get the inverse permutation
    std::vector<uintT> &iperm();
    /// Sort the keys
    inline void sort();
    /// Calculate the inverse permutation
    inline void make_iperm();
    /// Set the co-ordinate data
    void set(size_t num_nodes, floatT *nodes);
    /// Set the co-ordinate data
    void set(size_t num_nodes, size_t start, floatT *nodes);
    /// Clear the buffers
    void clear();
    /// Get functions
    IdKey<uint32_t, uintT> *get32();
    /// Get functions
    IdKey<uint64_t, uintT> *get64();

  private:
    /// The bounding box for sfc calculation
    floatT _min[3], _max[3];
    /// The size and pointer to the data
    floatT *_nodes;
    uintT _n_node, _start;
    /// The permutation inverse
    std::vector<uintT> _iperm;
    /// The keys with id (32 bit)
    IdKey<uint32_t, uintT> *_id_key_32;
    /// The keys with id (64 bit)
    IdKey<uint64_t, uintT> *_id_key_64;
    /// The keys for the nbit encoding
    IdKey<index_t, uintT> *_id_key_nbit;
    /// The 10 bit morton key encoding
    inline uint32_t _10bit(uint32_t x, uint32_t y, uint32_t z) const;
    /// The 20 bit morton key encoding
    inline uint64_t _20bit(uint32_t x, uint32_t y, uint32_t z) const;
    /// The 20 bit morton key encoding
    inline index_t _nbit(index_t xyz) const;
  };

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  void sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::clear() {
    if (_id_key_32 != 0)
      delete[] _id_key_32;
    if (_id_key_64 != 0)
      delete[] _id_key_64;
    if (_id_key_nbit != 0)
      delete[] _id_key_nbit;
    _iperm.clear();
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  IdKey<uint32_t, uintT> *sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::get32() {
    return _id_key_32;
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  IdKey<uint64_t, uintT> *sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::get64() {
    return _id_key_64;
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::~sfcFunctor() {
    /* empty */
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::sfcFunctor(floatT *min, floatT *max)
  : _nodes(0), _n_node(0), _start(0),
  _id_key_32(0), _id_key_64(0),
  _id_key_nbit(0) {
    /// Min-Max copy
    for (unsigned i = 0; i < 3; ++i)
      _min[i] = min[i];
    for (unsigned i = 0; i < 3; ++i)
      _max[i] = max[i];
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  void sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::set(size_t num_nodes, floatT *nodes) {
    _n_node = num_nodes;
    _nodes = nodes;
    if (nbitT == _10BIT)
      _id_key_32 = new IdKey<uint32_t, uintT>[_n_node];
    if (nbitT == _20BIT)
      _id_key_64 = new IdKey<uint64_t, uintT>[_n_node];
    if (nbitT == _NBIT)
      _id_key_nbit = new IdKey<index_t, uintT>[_n_node];
    tbb::parallel_for(tbb::blocked_range<uintT>(0, _n_node), *this);
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  void sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::set(size_t num_nodes, size_t start, floatT *nodes) {
    _start = start;
    _n_node = num_nodes;
    _nodes = nodes;
    if (nbitT == _10BIT)
      _id_key_32 = new IdKey<uint32_t, uintT>[_n_node];
    if (nbitT == _20BIT)
      _id_key_64 = new IdKey<uint64_t, uintT>[_n_node];
    if (nbitT == _NBIT)
      _id_key_nbit = new IdKey<index_t, uintT>[_n_node];
    tbb::parallel_for(tbb::blocked_range<uintT>(0, _n_node), *this);
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  void sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::operator()(const tbb::blocked_range<uintT> &range) const {
    for (uintT i = range.begin(); i < range.end(); ++i) {
      if (nbitT == _10BIT) {
        uint32_t xyz[3];
        for (unsigned j = 0; j < 3; ++j)
          xyz[j] = uint32_t((_nodes[i * 3 + j] - _min[j]) /
          (_max[j] - _min[j]) * ((1 << 10) - 1));
        _id_key_32[i].key = _10bit(xyz[0], xyz[1], xyz[2]);
        _id_key_32[i].id = i + _start;
      }
      if (nbitT == _20BIT) {
        uint32_t xyz[3];
        for (unsigned j = 0; j < 3; ++j)
          xyz[j] = uint32_t((_nodes[i * 3 + j] - _min[j]) /
          (_max[j] - _min[j]) * ((1 << 20) - 1));
        _id_key_64[i].key = _20bit(xyz[0], xyz[1], xyz[2]);
        _id_key_64[i].id = i + _start;
      }
      if (nbitT == _NBIT) {
        index_t xyz;
        for (unsigned j = 0; j < 3; ++j)
          xyz.xyz[j] = uint64_t((_nodes[i * 3 + j] - _min[j]) /
          (_max[j] - _min[j]) * (std::numeric_limits<uint64_t>::max()));
        _id_key_nbit[i].key = _nbit(xyz);
        _id_key_nbit[i].id = i + _start;
      }
    }
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  uint32_t sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::_10bit(uint32_t x, uint32_t y, uint32_t z) const {
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;

    y = (y | (y << 16)) & 0x030000FF;
    y = (y | (y << 8)) & 0x0300F00F;
    y = (y | (y << 4)) & 0x030C30C3;
    y = (y | (y << 2)) & 0x09249249;

    z = (z | (z << 16)) & 0x030000FF;
    z = (z | (z << 8)) & 0x0300F00F;
    z = (z | (z << 4)) & 0x030C30C3;
    z = (z | (z << 2)) & 0x09249249;

    return x | (y << 1) | (z << 2);
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  uint64_t sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::_20bit(uint32_t x, uint32_t y, uint32_t z) const {
    uint32_t lo_x = x & 1023u;
    uint32_t lo_y = y & 1023u;
    uint32_t lo_z = z & 1023u;
    uint32_t hi_x = x >> 10u;
    uint32_t hi_y = y >> 10u;
    uint32_t hi_z = z >> 10u;
    return ( uint64_t(_10bit(hi_x, hi_y, hi_z)) << 30)
      | uint64_t(_10bit(lo_x, lo_y, lo_z));
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  index_t sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::_nbit(index_t xyz) const {
    const uintT DIM = 3;
    const uintT bitsPerWord = sizeof (uint64_t) * CHAR_BIT;
    uintT myPos = 0;
    std::bitset< bitsPerWord > super[DIM];
    /// The bit interleaving
    for (int i = 0; i < bitsPerWord; ++i) {
      for (int j = 0; j < DIM; ++j) {
        myPos = i * DIM + j;
        super[ myPos / bitsPerWord ].test(myPos % bitsPerWord);
        super[ myPos / bitsPerWord ][ myPos % bitsPerWord ] =
          ((xyz.xyz[ j ] >> i) & 1);
      }
    }
    for (int j = 0; j < DIM; ++j)
      xyz.xyz[j] = super[DIM - j - 1].to_ulong();
    return xyz;
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  std::vector<uintT> &sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::iperm() {
    return _iperm;
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  void sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::sort() {
    if (nbitT == _10BIT)
      tbb::parallel_sort(_id_key_32, _id_key_32 + _n_node);
    if (nbitT == _20BIT)
      tbb::parallel_sort(_id_key_64, _id_key_64 + _n_node);
    if (nbitT == _NBIT)
      tbb::parallel_sort(_id_key_nbit, _id_key_nbit + _n_node);
  }

  template
  <
  typename floatT,
  typename uintT,
  SFC_NUM_BITS nbitT,
  STORAGE_ORDER sorderT
  >
  void sfcFunctor<floatT, uintT, nbitT, sorderT>
  ::make_iperm() {
    if (nbitT == _10BIT) {
      _iperm.resize(_n_node);
      for (uintT i = 0; i < _n_node; ++i)
        _iperm[ _id_key_32[i].id ] = i;
    }
    if (nbitT == _20BIT) {
      _iperm.resize(_n_node);
      for (uintT i = 0; i < _n_node; ++i)
        _iperm[ _id_key_64[i].id ] = i;
    }
    if (nbitT == _NBIT) {
      _iperm.resize(_n_node);
      for (uintT i = 0; i < _n_node; ++i)
        _iperm[ _id_key_nbit[i].id ] = i;
    }
  }

} // end of sfc namespace

#endif


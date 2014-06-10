/*! \file node.hpp
 **       The header file containing the basic
 **       mesh entity shape face
 **/
#ifndef NODE_ENTITY_HPP

#define NODE_ENTITY_HPP

#include<limits>
#include<cmath>

/*! \brief Node entity of the mesh
 **        The coordinates xyz are stored
 **        for 2d meshes also we store z
 **        coordinate.
 **/
template< typename floatT >
struct node {
  floatT xyz[3];

  /// For tecplot write function
  inline floatT &operator[](unsigned i) {
    return xyz[i];
  }

  /// For tecplot write function

  inline floatT *operator()() {
    return &xyz[0];
  }

  /// Element-wise addition by scalar 

  inline void operator+=(const floatT &x) {
    for (unsigned i = 0; i < 3; ++i)
      xyz[i] += x;
  }

  /// Element-wise addition by vector 

  inline void operator+=(const node<floatT> &x) {
    for (unsigned i = 0; i < 3; ++i)
      xyz[i] += x.xyz[i];
  }

  /// The dot product
  inline floatT dot(const node<floatT> &x);

  /// The cross product
  inline node<floatT> cross(const node<floatT> &x);

  /// The cross product
  inline floatT normalize();

  /// Scale by a scalar
  inline void scale(floatT x);

  /// Return the number of components making up this pod data

  static inline unsigned NumComp() {
    return 3;
  }

  /// Morton ordering of nodes

  bool operator<(const node<floatT> &x) {

  }

};

template< typename floatT >
node<floatT> node<floatT>
::cross(const node<floatT> &x) {
  node<floatT> ret;
  for (unsigned i = 0; i < 3; i++)
    ret.xyz[i] = (xyz[(i + 1) % 3] * x.xyz[(i + 2) % 3]) -
    (x.xyz[(i + 1) % 3] * xyz[(i + 2) % 3]);
  return ret;
}

template< typename floatT >
floatT node<floatT>
::dot(const node<floatT> &x) {
  double ret;
  ret = xyz[0] * x.xyz[0] + xyz[1] * x.xyz[1] + xyz[2] * x.xyz[2];
  return ret;
}

template< typename floatT >
floatT node<floatT>
::normalize() {
  double ret;
  ret = std::sqrt(dot(*this));
  for (unsigned i = 0; i < 3; i++)
    xyz[i] /= ret;
  return ret;
}

template< typename floatT >
void node<floatT>
::scale(floatT x) {
  for (unsigned i = 0; i < 3; i++)
    xyz[i] *= x;
}

/// Addition operator

template<typename floatT>
inline node<floatT> operator+(const node<floatT> &lhs, const node<floatT> &rhs) {
  node<floatT> ret;
  for (unsigned i = 0; i < 3; ++i)
    ret()[i] = lhs.xyz[i] + rhs.xyz[i];
  return ret;
}

/// Sub operator

template<typename floatT>
inline node<floatT> operator-(const node<floatT> &lhs, const node<floatT> &rhs) {
  node<floatT> ret;
  for (unsigned i = 0; i < 3; ++i)
    ret()[i] = lhs.xyz[i] - rhs.xyz[i];
  return ret;
}

namespace common {

  template<typename floatT>
  inline floatT GetTolerance() {
    return std::sqrt(std::numeric_limits<floatT>::epsilon());
  }
}

#endif


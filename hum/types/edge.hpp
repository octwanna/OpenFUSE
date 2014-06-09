/*! \file edge.h
 **       The header file containing the basic
 **       mesh entity shape edge
 **/
#ifndef EDGE_ENTITY_H

#define EDGE_ENTITY_H

/*! \brief Edge entity of the mesh
 **        Contains the two node IDs
 **        forming the edge
 **/
template< typename uintT >
struct edge {
  uintT entityID[2];
};

#endif

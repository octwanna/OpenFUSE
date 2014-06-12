.. figure:: figures/logo_medium.png

Introduction
============
.. figure:: figures/mind_map.png
   :width: 70%


OpenFUSE is a software framework, which supports various
numerical algorithms to solve Partial Differential Equations (PDEs) on
unstructured meshes. Unlike other software frameworks, OpenFUSE
provides only the bare minimum data and functions required to implement any
type of numerical solvers. OpenFUSE does not by
itself support any particular type of numerical method viz., finite volume,
finite element or discontinuous Galerkin. It only
supplies the necessary mesh data-structure and access to inter-processor data
using a novel in-memory mesh database system called "dimm". 
It even supplies its own distributed parallel mesh partitioner.
All algorithms used in OpenFUSE are extremely scalable and has the
following key features required for efficient numerical solutions to PDEs.


 * Unstructured mesh file format based on HDF5 geared for parallel I/O and non-linear elements
 * Scalable mesh conversion (from other formats)
 * Parallel distributed mesh decomposition
 * Parallel adaptive mesh refinement
 * Parallel visualization using plugins for VisIT and paraview
 * Abstract operator and container
 * Distributed Graph Algorithms

Most of the components like parallel mesh decomposition, adaptation and distributed graphs algorithms have been communicated to peer review journals, hence the source code for the same is withheld. They will be released once published, nevertheless the file format and serial conversion tools are available.

.. note::
  This project was borne out of my frustration to partition and visualize very large meshes using OpenFOAM and SU2 CFD solver. They inherently use serial partitioning due to the lack of parallel - I/O, and mesh query. Using OpenFUSE it is possible to convert meshes bigger than 1B cells and perform distributed parallel partitioning of the meshes in under 60s. Moreover, it is not necessary to store paritioned meshes as it is simple to write the solution to one global file using parallel I/O. Since many of my other colleagues in the community share the same frustration, I have created converters for OpenFOAM and SU2 to output partitioned meshes. A direct API access to OpenFOAM parallel fvMesh class is under development. The mesh adaptation in OpenFUSE at present cannot handle hanging nodes and works only on tetrahedral meshes.



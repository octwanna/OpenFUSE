#include "ihstream.hpp"
#include "faceStreamer.hpp"
#include "faceLeftRightStreamer.hpp"
#include <string>
#include <tclap/CmdLine.h>
#include <stdint.h>
#include "sfc.hpp"
#include <fstream>
#include <sys/time.h>

/// Some typedefs
typedef TCLAP::CmdLine CmdLineClass;
typedef TCLAP::ValueArg<std::string> StringArg;
typedef TCLAP::ValueArg<unsigned> IntArg;

/// Reorder nodes
template<typename uintT>
void ReorderNode(const char *hum_file, size_t limit);

/// Reorder cells
template<typename uintT>
void ReorderCell(const char *hum_file, size_t limit);

/// Main program

int main(int nargs, char *args[]) {
  try {
    /// The command line object
    CmdLineClass cmd
      (
      "Mesh reordering for hum - serial stream version",
      ' ', "0.1"
      );
    /// The Input hum file name
    StringArg hum_file_arg
      (
      "i", "input",
      "The hum mesh file", true,
      "", "string"
      );
    cmd.add(hum_file_arg);
    /// The Output hum file name
    IntArg buf_size_arg
      (
      "s", "size",
      "The stream buffer size - in entity counts", false,
      10000, "integer"
      );
    cmd.add(buf_size_arg);
    /// Toggle node reordering
    TCLAP::SwitchArg is_node_arg
      (
      "n", "node",
      "Disable node re-ordering",
      cmd, true
      );
    /// Toggle cell reordering
    TCLAP::SwitchArg is_cell_arg
      (
      "c", "cell",
      "Disable cell re-ordering",
      cmd, true
      );
    cmd.parse(nargs, args);
    std::string hum_file = hum_file_arg.getValue();
    double buf_size = buf_size_arg.getValue();
    bool is_cell = is_cell_arg.getValue();
    bool is_node = is_node_arg.getValue();
    bool is64;
    {
      ihstream hum_in(hum_file.c_str());
      is64 = (hum_in.get_int_size() > 4) ? true : false;
      hum_in.close();
    }
    //#if 0
    //// Cell re-ordering
    if (is_cell) {
      if (is64)
        ReorderCell<uint64_t>(hum_file.c_str(), buf_size);
      else
        ReorderCell<uint32_t>(hum_file.c_str(), buf_size);
    }
    //// Node re-ordering
    if (is_node) {
      if (is64)
        ReorderNode<uint64_t>(hum_file.c_str(), buf_size);
      else
        ReorderNode<uint32_t>(hum_file.c_str(), buf_size);
    }
    //#endif
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg "
      << e.argId() << std::endl;
    return 1;
  }
  return 0;
}

template<typename uintT>
void ReorderNode(const char *hum_file, size_t limit) {
  node<double> min, max;
  std::vector< node<double> > nodes;
  std::vector< uintT > iperm, perm;
  struct timeval begin, end;
  double elapsed;
  std::cout << " ===========================================\n";
  std::cout << " ====  Node re-ordering and re-numbering ===\n";
  std::cout << " ===========================================\n";
  /// Construct sfc and sort and get fwd/inv permutation
  gettimeofday(&begin, NULL);
  std::cout << "SFC construction + sorting ... ";
  /// Read from input file
  ihstream hum_in(hum_file, true);
  hum_in.read(min, max);
  nodes.resize(hum_in.nNode());
  hum_in.read(&nodes[0]);
  sfc::sfcFunctor<double, uintT> sfc_func(min.xyz, max.xyz);
  sfc_func.set(nodes.size(), &(nodes[0].xyz[0]));
  sfc_func.sort();
  sfc_func.make_iperm();
  iperm.swap(sfc_func.iperm());
  sfc_func.clear();
  gettimeofday(&end, NULL);
  elapsed = double( end.tv_sec - begin.tv_sec) * 1.0e3
    + double( end.tv_usec - begin.tv_usec) / 1.0e3;
  std::cout << "(done) " << elapsed << " ms\n";

  /// Re-order nodes
  gettimeofday(&begin, NULL);
  std::cout << "Node re-ordering ... ";
  /// Create permutation
  perm.resize(iperm.size());
  for (uintT i = 0; i < perm.size(); ++i)
    perm[ iperm[i] ] = i;
  sfc::InplacePermutation(&nodes[0], &perm[0], perm.size());
  hum_in.write(&nodes[0]);
  gettimeofday(&end, NULL);
  elapsed = double( end.tv_sec - begin.tv_sec) * 1.0e3
    + double( end.tv_usec - begin.tv_usec) / 1.0e3;
  std::cout << "(done) " << elapsed << " ms\n";

  /// Re-number face-node connectivity
  gettimeofday(&begin, NULL);
  std::cout << "Stream re-numbering of face-nodes ... ";
  faceStreamer<uintT> fs(hum_in, 10000);
  fs.SetWriteBufOn();
  while (!fs.isEof()) {
    for (uintT i = 0; i < fs.GetNumFaceNodes(); ++i) {
      fs.FaceNodesData()[i] = iperm[fs.FaceNodesData()[i]];
    }
    fs.Increment();
  }
  gettimeofday(&end, NULL);
  elapsed = double( end.tv_sec - begin.tv_sec) * 1.0e3
    + double( end.tv_usec - begin.tv_usec) / 1.0e3;
  std::cout << "(done) " << elapsed << " ms\n";
}

template<typename uintT>
void ReorderCell(const char *hum_file, size_t limit) {
  std::vector< node<double> > nodes, centroid;
  node<double> min, max;
  struct timeval begin, end;
  double elapsed;
  std::vector<uintT> cell_face_count, iperm;

  std::cout << " ======================================\n";
  std::cout << " ====  Cell left/right re-numbering ===\n";
  std::cout << " ======================================\n";
  /// Centroid construction
  gettimeofday(&begin, NULL);
  std::cout << "Cell centroid construction ...";
  ihstream hum_in(hum_file, true);
  hum_in.read(min, max);
  nodes.resize(hum_in.nNode());
  hum_in.read(&nodes[0]);
  cell_face_count.resize(hum_in.nCell());
  centroid.resize(hum_in.nCell());
  {
    faceLeftRightStreamer<uintT> fs_lr(hum_in, 10000);
    faceStreamer<uintT> fs(hum_in, 10000);
    /// Internal faces
    while (!fs_lr.isEof()) {
      node<double> face_centroid;
      const uintT &left = fs_lr.GetLeftCell();
      const uintT &right = fs_lr.GetRightCell();
      for (unsigned j = 0; j < fs.GetNumFaceNodes(); ++j)
        face_centroid += nodes[fs.FaceNodesData()[j]];
      face_centroid.scale(1.0 / double(fs.GetNumFaceNodes()));
      centroid[left] += face_centroid;
      centroid[right] += face_centroid;
      cell_face_count[left]++;
      cell_face_count[right]++;
      /// Increment the stream counter
      fs_lr.Increment();
      fs.Increment();
    }
    /// Boundary faces
    while (!fs_lr.isEofPatch()) {
      while (!fs_lr.isEofPatchFace()) {
        node<double> face_centroid;
        const uintT &left = fs_lr.GetPatchCell();
        for (unsigned j = 0; j < fs.GetNumFaceNodes(); ++j)
          face_centroid += nodes[fs.FaceNodesData()[j]];
        face_centroid.scale(1.0 / double(fs.GetNumFaceNodes()));
        centroid[left] += face_centroid;
        cell_face_count[left]++;
        /// Increment the stream counter
        fs_lr.IncrementPatchFace();
        fs.Increment();
      }
      fs_lr.IncrementPatch();
    }
  }
  /// Divide by cell_face_count to get cell centroid
  for (uintT i = 0; i < hum_in.nCell(); ++i)
    centroid[i].scale(1.0 / double( cell_face_count[i]));
  cell_face_count.clear();
  nodes.clear();

  gettimeofday(&end, NULL);
  elapsed = double( end.tv_sec - begin.tv_sec) * 1.0e3
    + double( end.tv_usec - begin.tv_usec) / 1.0e3;
  std::cout << "(done) " << elapsed << " ms\n";

#if 0
  std::ofstream fout("centroid.dat");
  fout << "VARIABLES=\"X\",\"Y\",\"Z\"\nZONE I=" << hum_in.nCell() << "\n";
  for (uintT i = 0; i < hum_in.nCell(); ++i)
    fout << centroid[i].xyz[0] << "  "
    << centroid[i].xyz[1] << "  "
    << centroid[i].xyz[2] << "\n";
#endif

#if 1
  /// SFC construction + key sorting
  gettimeofday(&begin, NULL);
  std::cout << "SFC construction + key sorting ...";
  sfc::sfcFunctor<double, uintT, sfc::_20BIT> sfc_func(min.xyz, max.xyz);
  sfc_func.set(centroid.size(), &(centroid[0].xyz[0]));
  sfc_func.sort();
  sfc_func.make_iperm();
  iperm.swap(sfc_func.iperm());
  sfc_func.clear();
  gettimeofday(&end, NULL);
  elapsed = double( end.tv_sec - begin.tv_sec) * 1.0e3
    + double( end.tv_usec - begin.tv_usec) / 1.0e3;
  std::cout << "(done) " << elapsed << " ms\n";
  /// Cell left/right re-numbering
  gettimeofday(&begin, NULL);
  std::cout << "Face left/right cell ID re-numbering (streaming) ...";
  faceLeftRightStreamer<uintT> fs_lr(hum_in, 10000);
  fs_lr.SetWriteBufOn();
  /// Internal faces
  while (!fs_lr.isEof()) {
    fs_lr.FaceLeftData() = iperm[ fs_lr.FaceLeftData() ];
    fs_lr.FaceRightData() = iperm[ fs_lr.FaceRightData() ];
    fs_lr.Increment();
  }
  /// Patch faces
  while (!fs_lr.isEofPatch()) {
    while (!fs_lr.isEofPatchFace()) {
      fs_lr.PatchFaceData() = iperm[ fs_lr.PatchFaceData() ];
      fs_lr.IncrementPatchFace();
    }
    fs_lr.IncrementPatch();
  }
  gettimeofday(&end, NULL);
  elapsed = double( end.tv_sec - begin.tv_sec) * 1.0e3
    + double( end.tv_usec - begin.tv_usec) / 1.0e3;
  std::cout << "(done) " << elapsed << " ms\n";
#endif
}


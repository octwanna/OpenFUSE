#include "constants.hpp"

namespace hum {
  ///
  const char *nodeLink[] = {"Nodes", "XYZ", "", ""};
  ///
  const char *edgeLink[] = {"Edges", "", "BitField", "EntityID"};
  ///
  const char *faceLink[] = {"Faces", "FaceLRCell", "BitField",
    "EntityID", "Left", "Right"};
  ///
  const char *patchLink[] = {"Patches", "PatchInfo", "", "",
    "BCType", "StartFace", "FaceCount", "ProcID"};
  ///
  const char *AABBLink[] = {"Min", "Max"};
  ///
  const char *miscLink[] = {"NumCells", "FaceAdjncySize", "NumInternalFaces", "IntegerT"};
  ///
  const char *cellCacheLink[] = {"Cache", "cellFace", "cellCell"};
}


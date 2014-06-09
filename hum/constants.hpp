/*! \file constants.hpp
 **
 **/

#ifndef CONSTS_H

#define CONSTS_H

namespace hum {

  enum linkType {
    PRIMARY = 0, SECONDARY = 1, FIELD = 2, ENTITY = 3, LEFT_ID = 4, RIGHT_ID = 5
  };

  enum patchLinkType {
    BCTYPE = 4, START_FACE = 5, FACE_COUNT = 6, PROC_ID = 7
  };

  extern const char *nodeLink[];
  extern const char *edgeLink[];
  extern const char *faceLink[];
  extern const char *patchLink[];
  extern const char *AABBLink[];
  extern const char *miscLink[];
  extern const char *cellCacheLink[];

} // End of hum namespace

#endif


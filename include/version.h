#ifndef PASSDOQ_VERSION_H
#define PASSDOQ_VERSION_H

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY (x)

#define PASSDOQ_VERSION_MAJOR 0
#define PASSDOQ_VERSION_MINOR 1
#define PASSDOQ_VERSION_PATCH 0

#define PASSDOQ_VERSION_INT                                                     \
  ((PASSDOQ_VERSION_MAJOR * 10000) + (PASSDOQ_VERSION_MINOR * 100)                \
   + PASSDOQ_VERSION_PATCH)

#define PASSDOQ_VERSION_STRING                                                  \
  TOSTRING (PASSDOQ_VERSION_MAJOR)                                              \
  "." TOSTRING (PASSDOQ_VERSION_MINOR) "." TOSTRING (PASSDOQ_VERSION_PATCH)

#endif

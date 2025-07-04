#ifndef LIBACQUIRE_ACQUIRE_COMMON_DEFS_H
#define LIBACQUIRE_ACQUIRE_COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "acquire_status_codes.h"

enum Checksum {
  LIBACQUIRE_CRC32C,
  LIBACQUIRE_SHA256,
  LIBACQUIRE_SHA512,
  LIBACQUIRE_UNSUPPORTED_CHECKSUM
};

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#if (defined(WIN32) || defined(_WIN32)) && !defined(__WIN32__)
#define __WIN32__ 1
#endif

#ifdef __cplusplus
}
#endif

#endif /* !LIBACQUIRE_ACQUIRE_COMMON_DEFS_H */

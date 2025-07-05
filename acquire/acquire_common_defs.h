#ifndef LIBACQUIRE_ACQUIRE_COMMON_DEFS_H
#define LIBACQUIRE_ACQUIRE_COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_status_codes.h"

enum Checksum {
  LIBACQUIRE_CRC32C,
  LIBACQUIRE_SHA256,
  LIBACQUIRE_SHA512,
  LIBACQUIRE_INVALID_CHECKSUM_ALGO,
  LIBACQUIRE_UNSUPPORTED_CHECKSUM
};

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_COMMON_DEFS_H */

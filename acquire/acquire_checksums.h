/*
 * Prototype for checksum API
 *
 * Always `#include` this when adding new checksum implementations,
 * to ensure the implementation matches the prototype.
 * */

#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "libacquire_export.h"
#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif
#include "acquire_config.h"
#include "acquire_string_extras.h"
#ifdef USE_LIBRHASH
#include "acquire_librhash.h"
#endif /* USE_LIBRHASH */

extern LIBACQUIRE_LIB_EXPORT bool crc32c(const char *, const char *);

extern LIBACQUIRE_LIB_EXPORT bool sha256(const char *, const char *);

extern LIBACQUIRE_LIB_EXPORT bool sha512(const char *, const char *);

enum Checksum {
  LIBACQUIRE_CRC32C,
  LIBACQUIRE_SHA256,
  LIBACQUIRE_SHA512,
  LIBACQUIRE_UNSUPPORTED_CHECKSUM
};

extern LIBACQUIRE_LIB_EXPORT enum Checksum string2checksum(const char *);

extern LIBACQUIRE_LIB_EXPORT bool (*get_checksum_function(enum Checksum))(
    const char *, const char *);

#ifdef LIBACQUIRE_IMPLEMENTATION

enum Checksum string2checksum(const char *const s) {
  if (strncasecmp(s, "CRC32C", 6) == 0)
    return LIBACQUIRE_CRC32C;
  else if (strncasecmp(s, "SHA256", 6) == 0)
    return LIBACQUIRE_SHA256;
  else if (strncasecmp(s, "SHA512", 6) == 0)
    return LIBACQUIRE_SHA512;
  else
    return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
}

bool (*get_checksum_function(enum Checksum checksum))(const char *,
                                                      const char *) {
  switch (checksum) {
  case LIBACQUIRE_CRC32C:
    return crc32c;
  case LIBACQUIRE_SHA256:
    return sha256;
  case LIBACQUIRE_SHA512:
    return sha512;
  case LIBACQUIRE_UNSUPPORTED_CHECKSUM:
  default:
    return NULL;
  }
}

#endif /* LIBACQUIRE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_CHECKSUMS_H */

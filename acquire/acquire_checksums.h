#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

/**
 * @file acquire_checksums.h
 * @brief Collection of checksum function declarations and interface.
 *
 * This header declares typedefs and functions for computing and verifying
 * various checksum algorithms like crc32c, md5, sha1, etc.
 * Also provides functionality to get checksum function by name.
 *
 * NOTE: Always `#include` this when adding new checksum implementations,
 * to ensure the implementation matches the prototype.
 */

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

/**
 * @brief Boolean result type alias for checksum validation functions.
 */

/**
 * @brief Verify file using CRC32C checksum.
 *
 * @param filename Path to file.
 * @param hash Expected CRC32C checksum string.
 *
 * @return true if valid.
 */
extern LIBACQUIRE_EXPORT bool crc32c(const char *filename, const char *hash);

/**
 * @brief Verify file using SHA256 checksum.
 *
 * @param filename Path to file.
 * @param hash Expected checksum string.
 *
 * @return true if valid.
 */
extern LIBACQUIRE_EXPORT bool sha256(const char *filename, const char *hash);

/**
 * @brief Verify file using SHA256 checksum.
 *
 * @param filename Path to file.
 * @param hash Expected checksum string.
 *
 * @return true if valid.
 */
extern LIBACQUIRE_EXPORT bool sha512(const char *filename, const char *hash);

enum Checksum {
  LIBACQUIRE_CRC32C,
  LIBACQUIRE_SHA256,
  LIBACQUIRE_SHA512,
  LIBACQUIRE_UNSUPPORTED_CHECKSUM
};

extern LIBACQUIRE_EXPORT enum Checksum string2checksum(const char *);

/**
 * @brief Obtain a pointer to a checksum function by name.
 *
 * Returns a pointer to a function matching the
 * signature of checksum_func_t, or NULL if not found.
 *
 * @param checksum Discriminant from `enum Checksum` representing checksum
 * algorithm
 *
 * @return Function pointer on success; NULL on failure.
 */
extern LIBACQUIRE_EXPORT bool (*get_checksum_function(enum Checksum checksum))(
    const char *, const char *);

#if defined(LIBACQUIRE_IMPLEMENTATION) && !defined(CHECKSUM_IMPL)
#define CHECKSUM_IMPL

enum Checksum string2checksum(const char *const s) {
  if (s == NULL)
    return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
  else if (strncasecmp(s, "CRC32C", 6) == 0)
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

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) && !defined(CHECKSUM_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_CHECKSUMS_H */

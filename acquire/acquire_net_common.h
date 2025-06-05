/*
 * Networking helper API
 *
 * Provides minimal networking-related helpers used throughout libacquire.
 * Does NOT implement fileutils functions (moved to acquire_fileutils.h).
 * */

#ifndef LIBACQUIRE_ACQUIRE_NET_COMMON_H
#define LIBACQUIRE_ACQUIRE_NET_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_common_defs.h"
#include "acquire_config.h"

/**
 * Check if a file represented by URL is already downloaded locally and matches
 * the checksum.
 *
 * @param url URL or filename
 * @param checksum checksum enum type, e.g., LIBACQUIRE_SHA256
 * @param hash expected hash string to verify
 * @param target_location directory to check in (if NULL, defaults to
 * ".downloads")
 * @return true if already downloaded and checksum verifies, else false
 */
extern LIBACQUIRE_EXPORT bool is_downloaded(const char *url,
                                            enum Checksum checksum,
                                            const char *hash,
                                            const char *target_location);

#if defined(LIBACQUIRE_IMPLEMENTATION) &&                                      \
    !defined(LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL)
#include "acquire_common_defs.h"
#include "acquire_config.h"

#define LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL 1

#ifdef DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".downloads"; }
#endif /* DOWNLOAD_DIR_IMPL */

/**
 * Implementation of is_downloaded.
 * Uses fileutils functions for existence checks and checksum functions for
 * verification.
 */
bool is_downloaded(const char *url, enum Checksum checksum, const char *hash,
                   const char *target_location) {
  char full_local_fname[NAME_MAX + 1];
  const char *filename = is_url(url) ? get_path_from_url(url) : url;

  if (target_location == NULL) {
    target_location = get_download_dir();
  }

  if (filename == NULL || strlen(filename) == 0 ||
      !is_directory(target_location)) {
    return false;
  }

  snprintf(full_local_fname, NAME_MAX + 1, "%s" PATH_SEP "%s", target_location,
           filename);

  if (!is_file(full_local_fname)) {
    return false;
  }

  switch (checksum) {
  case LIBACQUIRE_SHA256:
    return sha256(full_local_fname, hash);
  case LIBACQUIRE_SHA512:
    /* TODO: can add sha512 once implemented and enabled */
    return false;
  case LIBACQUIRE_UNSUPPORTED_CHECKSUM:
  default:
    return false;
  }
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          !defined(LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_NET_COMMON_H */

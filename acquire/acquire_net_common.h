#ifndef LIBACQUIRE_ACQUIRE_NET_COMMON_H
#define LIBACQUIRE_ACQUIRE_NET_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Include the public headers that declare the functions we use */
#include "acquire_checksums.h"
#include "acquire_download.h"
#include "acquire_fileutils.h"
#include "acquire_handle.h"
#include "acquire_url_utils.h"

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif

/**
 * Check if a file represented by URL is already downloaded locally and matches
 * the checksum.
 */
extern LIBACQUIRE_EXPORT bool is_downloaded(const char *url,
                                            enum Checksum checksum,
                                            const char *hash,
                                            const char *target_location);

extern LIBACQUIRE_EXPORT const char *get_download_dir(void);

#if defined(LIBACQUIRE_IMPLEMENTATION) &&                                      \
    defined(LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL)

/*
 * Implementation of is_downloaded.
 * Uses fileutils for existence checks and the new handle-based verification
 * API.
 */
bool is_downloaded(const char *url, enum Checksum checksum, const char *hash,
                   const char *target_location) {
  char full_local_fname[NAME_MAX + 1];
  const char *filename = is_url(url) ? get_path_from_url(url) : url;
  struct acquire_handle *verify_handle;
  int result;

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

  verify_handle = acquire_handle_init();
  if (!verify_handle)
    return false;

  result = acquire_verify_sync(verify_handle, full_local_fname, checksum, hash);

  acquire_handle_free(verify_handle);

  return result == 0;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_NET_COMMON_H */

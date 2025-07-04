#ifndef LIBACQUIRE_ACQUIRE_NET_COMMON_H
#define LIBACQUIRE_ACQUIRE_NET_COMMON_H

#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#include "acquire_checksums.h"
#include "libacquire_export.h"

extern LIBACQUIRE_EXPORT bool is_downloaded(const char *url,
                                            enum Checksum checksum,
                                            const char *hash,
                                            const char *target_location);

extern LIBACQUIRE_EXPORT const char *get_download_dir(void);

#if defined(LIBACQUIRE_IMPLEMENTATION) &&                                      \
    defined(LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL)

/* Implementation-specific includes */
#include "acquire_fileutils.h"
#include "acquire_handle.h"
#include "acquire_url_utils.h"

#include <string.h>

bool is_downloaded(const char *url, enum Checksum checksum, const char *hash,
                   const char *target_location) {
  char full_local_fname[NAME_MAX + 1];
  char *filename_from_url = NULL;
  const char *filename;
  struct acquire_handle *verify_handle;
  int result;
  bool is_verified = false;

  if (is_url(url)) {
    filename_from_url = get_path_from_url(url);
    filename = filename_from_url;
  } else {
    filename = url;
  }

  if (filename == NULL || *filename == '\0' || hash == NULL) {
    free(filename_from_url);
    return false;
  }

  if (target_location == NULL) {
    target_location = get_download_dir();
  }

  if (is_file(filename)) {
    size_t len = strlen(filename);
    if (len > NAME_MAX)
      len = NAME_MAX - 1;
    memcpy(full_local_fname, filename, len);
    full_local_fname[len] = '\0';
  } else {
    if (!is_directory(target_location) && !is_file(target_location)) {
      free(filename_from_url);
      return false;
    }

    snprintf(full_local_fname, NAME_MAX + 1, "%s" PATH_SEP "%s",
             target_location, filename);

    if (!is_file(full_local_fname)) {
      free(filename_from_url);
      return false;
    }
  }

  verify_handle = acquire_handle_init();
  if (!verify_handle) {
    free(filename_from_url);
    return false;
  }

  result = acquire_verify_sync(verify_handle, full_local_fname, checksum, hash);
  acquire_handle_free(verify_handle);

  is_verified = (result == 0);

  free(filename_from_url);
  return is_verified;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !LIBACQUIRE_ACQUIRE_NET_COMMON_H */

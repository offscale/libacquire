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

bool is_downloaded(const char *url_or_path, enum Checksum checksum,
                   const char *hash, const char *target_location) {

  char path_buffer[NAME_MAX + 1];
  const char *file_to_check;
  char *filename_from_url = NULL;
  struct acquire_handle *verify_handle;
  int result;

  if (url_or_path == NULL || hash == NULL) {
    return false;
  }

  if (is_url(url_or_path)) {
    filename_from_url = get_path_from_url(url_or_path);
    if (filename_from_url == NULL || *filename_from_url == '\0') {
      free(filename_from_url);
      return false; /* Cannot determine filename from URL. */
    }
    if (target_location == NULL) {
      target_location = get_download_dir();
    }
    if (!is_directory(target_location)) {
      free(filename_from_url);
      return false; /* Download directory is invalid. */
    }
    snprintf(path_buffer, sizeof(path_buffer), "%s%s%s", target_location,
             PATH_SEP, filename_from_url);
    file_to_check = path_buffer;
    free(filename_from_url);
  } else {
    file_to_check = url_or_path;
  }

  if (!is_file(file_to_check)) {
    return false;
  }

  verify_handle = acquire_handle_init();
  if (!verify_handle) {
    return false;
  }

  result = acquire_verify_sync(verify_handle, file_to_check, checksum, hash);
  acquire_handle_free(verify_handle);

  return result == 0;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !LIBACQUIRE_ACQUIRE_NET_COMMON_H */

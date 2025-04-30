/*
 * API that is common between all network libraries… however requires network
 * libs included first
 * */

#ifndef LIBACQUIRE_ACQUIRE_NET_COMMON_H
#define LIBACQUIRE_ACQUIRE_NET_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_download.h"
#include "acquire_string_extras.h"

bool LIBACQUIRE_LIB_EXPORT is_downloaded(const char *url,
                                         enum Checksum checksum,
                                         const char *hash,
                                         const char *target_location);

#ifdef LIBACQUIRE_IMPLEMENTATION
bool is_downloaded(const char *url, enum Checksum checksum, const char *hash,
                   const char *target_location) {
  char full_local_fname[NAME_MAX + 1];
  const char *filename = is_url(url) ? get_path_from_url(url) : url;

  if (target_location == NULL)
    target_location = get_download_dir();

  if (filename == NULL || strlen(filename) == 0 ||
      !is_directory(target_location))
    return false;

  snprintf(full_local_fname, NAME_MAX + 1, "%s" PATH_SEP "%s", target_location,
           filename);

  if (!is_file(full_local_fname))
    return false;

  switch (checksum) {
  case LIBACQUIRE_SHA256:
    return sha256(full_local_fname, hash);
  case LIBACQUIRE_SHA512:
    /* return sha512(full_local_fname, hash); */
  case LIBACQUIRE_UNSUPPORTED_CHECKSUM:
  default:
    return false;
  }
}
#endif /* LIBACQUIRE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_NET_COMMON_H */

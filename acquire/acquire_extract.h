/*
 * Prototype for extract API
 *
 *
 * */

#ifndef LIBACQUIRE_ACQUIRE_EXTRACT_H
#define LIBACQUIRE_ACQUIRE_EXTRACT_H

/**
 * @file acquire_extract.h
 * @brief Extraction of compressed archives.
 *
 * Provides functions to decompress and extract files from archives,
 * supporting common formats like ZIP, TAR, GZ, etc.
 * NOTE: Always `#include` this when adding new extraction
 * implementations, to ensure implementation matches the prototype.
 *
 * The extract API is for expanding the contents of archives
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "libacquire_export.h"

enum LIBACQUIRE_LIB_EXPORT Archive {
  LIBACQUIRE_ZIP,
  LIBACQUIRE_INFER,
  LIBACQUIRE_UNSUPPORTED_ARCHIVE
};

/**
 * @brief Extract an archive file into a specified directory.
 *
 * Supports formats based on `enum Archive` discriminant.
 *
 * @param archive `enum Archive` discriminant type of archive.
 * @param archive_filepath Path to the archive file.
 * @param output_folder Directory path where contents should be extracted.
 *
 * @return `EXIT_SUCCESS` if extraction succeeds;
 * `EXIT_FAILURE` or non-`EXIT_SUCCESS` otherwise.
 */
extern LIBACQUIRE_LIB_EXPORT int extract_archive(enum Archive archive,
                                                 const char *archive_filepath,
                                                 const char *output_folder);

/**
 * @brief Determine archive type based on extension
 *
 * TODO: magic bytes whence `LIBACQUIRE_INFER`
 *
 * @param extension Simple end of path, like ".zip" or ".tar.gz".
 *
 * @return `enum Archive` discriminant; including potential values of
 * `LIBACQUIRE_UNSUPPORTED_ARCHIVE` xor `LIBACQUIRE_INFER`.
 */
extern LIBACQUIRE_LIB_EXPORT enum Archive
extension2archive(const char *extension);

#ifdef LIBACQUIRE_IMPLEMENTATION

#include <acquire_string_extras.h>

extern enum Archive extension2archive(const char *const s) {
  if (strncasecmp(s, ".zip", 6) == 0)
    return LIBACQUIRE_ZIP;
  else if (strlen(s) == 0)
    return LIBACQUIRE_UNSUPPORTED_ARCHIVE;
  else
    return LIBACQUIRE_INFER;
}

#endif /* LIBACQUIRE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_EXTRACT_H */

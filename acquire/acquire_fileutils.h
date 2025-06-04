#ifndef LIBACQUIRE_ACQUIRE_FILEUTILS_H
#define LIBACQUIRE_ACQUIRE_FILEUTILS_H

/**
 * @file acquire_fileutils.h
 * @brief File utility functions.
 *
 * Cross-platform collection of common filesystem operations like file existence
 * checks, path manipulation, temporary filename generation, and directory
 * creation.
 *
 * NOTE: This is not trying to be fully-featured, just enough cross-platform
 * building blocks to build basic nvm-style version managers.
 */

#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#include "libacquire_export.h"
#endif /* __cplusplus */

#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <acquire_common_defs.h>
#include <Fileapi.h>
#include <io.h>
#include <wchar.h>

#ifndef strtok_r
#define strtok_r strtok_s
#endif /* ! strtok_r */

#else

#include <sys/stat.h>
#include <unistd.h>

#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

extern LIBACQUIRE_EXPORT bool is_directory(const char *);

extern LIBACQUIRE_EXPORT bool is_file(const char *);

/**
 * @brief Check if a file exists.
 *
 * @param path Path of the file to check.
 * @return `true` if path is present
 */
extern LIBACQUIRE_EXPORT bool exists(const char *path);

/**
 * @brief Get the size of a given path
 *
 * @param path Path of the file to get the size of.
 * @return `-1` if file doesn't exist otherwise its size
 */
extern LIBACQUIRE_EXPORT off_t filesize(const char *path);

/**
 * @brief Get the size of a given path
 *
 * @param path Path of the file to determine relativity of
 * @return `true` if the path is a relative
 */
extern LIBACQUIRE_EXPORT bool is_relative(const char *path);

/**
 * @brief Get the extension from a path
 *
 * @param path Path of the file to get the file-extension from
 * @return `path` if the path failed to determine extension otherwise extension
 * (with leading ".").
 */
extern LIBACQUIRE_EXPORT const char *get_extension(const char *path);

#ifdef LIBACQUIRE_IMPLEMENTATION
#ifdef LIBACQUIRE_IMPL_ACQUIRE_FILEUTILS

bool is_directory(const char *const path) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  const DWORD dwAttrib = GetFileAttributes(path);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
          (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    return false;
  return S_ISDIR(statbuf.st_mode);
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */
}

bool is_file(const char *const path) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  const DWORD dwAttrib = GetFileAttributes(path);

  return dwAttrib != INVALID_FILE_ATTRIBUTES && !is_directory(path);
#else
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    return false;
  return S_ISREG(statbuf.st_mode);
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */
}

bool exists(const char *const path) {
  return
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
      _access
#else
      access
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */
      (path, 0) != -1;
}

off_t filesize(const char *const filename) {
  struct stat st;

  if (stat(filename, &st) == 0)
    return st.st_size;

  return -1;
}

bool is_relative(const char *const filename) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  return !filename || !*filename || (*filename != '\\' && filename[1] != ':');
#else
  return filename[0] != '/';
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */
}

const char *get_extension(const char *const filename) {
  /* Retrieves the file extension(s) from filename
   *
   * "tar" seems to be the only format serving as a middle extension (from Nov
   * 11 2021 rev of:
   * https://en.wikipedia.org/wiki/List_of_archive_formats#Archiving_and_compression
   * )
   * */
  const char *ext0 = filename + strlen(filename), *ext1 = ext0;
  size_t i;
  for (i = 0; filename[i] != '\0'; i++)
    if (filename[i] == '.') {
      ext0 = ext1;
      ext1 = filename + i;
    }

  return strncmp(ext0, ".tar", 4) == 0 ? ext0 : ext1;
}

#endif /* LIBACQUIRE_IMPL_ACQUIRE_FILEUTILS */
#endif /* LIBACQUIRE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_FILEUTILS_H */

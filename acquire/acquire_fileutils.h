/*
 * Cross-platform fileutils API
 *
 * Not trying to be fully-featured, just enough cross-platform building blocks
 * to build basic nvm-style version managers.
 * */

#ifndef LIBACQUIRE_ACQUIRE_FILEUTILS_H
#define LIBACQUIRE_ACQUIRE_FILEUTILS_H

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#include "libacquire_export.h"

#endif /* defined(HAS_STDBOOL) && !defined(bool) */
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <Fileapi.h>
#include <io.h>
#include <wchar.h>

#ifndef strtok_r
#define strtok_r strtok_s
#endif /* ! strtok_r */

#else

#include <sys/stat.h>
#include <unistd.h>

#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) */

extern LIBACQUIRE_LIB_EXPORT bool is_directory(const char *);

extern LIBACQUIRE_LIB_EXPORT bool is_file(const char *);

extern LIBACQUIRE_LIB_EXPORT bool exists(const char *);

extern LIBACQUIRE_LIB_EXPORT off_t filesize(const char *);

extern LIBACQUIRE_LIB_EXPORT bool is_relative(const char *);

extern LIBACQUIRE_LIB_EXPORT const char* get_extension(const char *);

#ifdef LIBACQUIRE_IMPLEMENTATION
bool is_directory(const char *path) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    const DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return false;
    return S_ISDIR(statbuf.st_mode);
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) */
}

bool is_file(const char *path) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    const DWORD dwAttrib = GetFileAttributes(path);

    return dwAttrib != INVALID_FILE_ATTRIBUTES && !is_directory(path);
#else
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return false;
    return S_ISREG(statbuf.st_mode);
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) */
}

bool exists(const char *path) {
    return
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        _access
#else
            access
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) */
                    (path, 0) != -1;
}

off_t filesize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

bool is_relative(const char *filename) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    return !filename || !*filename || (*filename != '\\' && filename[1] != ':');
#else
    return filename[0] != '/';
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) */
}

const char* get_extension(const char *filename) {
    /* Retrieves the file extension(s) from filename
     *
     * "tar" seems to be the only format serving as a middle extension (from Nov 11 2021 rev of:
     * https://en.wikipedia.org/wiki/List_of_archive_formats#Archiving_and_compression )
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

#endif /* LIBACQUIRE_IMPLEMENTATION */

#endif /* ! LIBACQUIRE_ACQUIRE_FILEUTILS_H */

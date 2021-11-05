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
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <Fileapi.h>
#include <io.h>
#include <wchar.h>

#ifndef strtok_r
#define strtok_r strtok_s
#endif /* strtok_r */

#else

#include <sys/stat.h>
#include <unistd.h>

#endif

extern bool is_directory(const char *);

extern bool is_file(const char *);

extern bool exists(const char *);

extern off_t filesize(const char *);

extern bool is_relative(const char *);

extern const char* get_extension(const char *);

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
#endif
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
#endif
}

bool exists(const char *path) {
    return
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        _access
#else
            access
#endif
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
#endif
}

const char* get_extension(const char *filename) {
    /* Retrieves the file extension(s) from filename
     *
     * Redevelopment note: could build a huge set from this, and see if middle extension fits:
     * https://en.wikipedia.org/wiki/List_of_archive_formats
     *
     * Currently: just does anything with .tar.<something> and single extension
     * */

    char *ext1, *ext0;
    {
        char *saveptr1, *split;
        unsigned short j;

        for (j = 1, split = strdup(filename); ; j++, split = NULL) {
            char *token = strtok_r(split, ".", &saveptr1);
            if (token == NULL)
                break;
            ext0 = ext1;
            ext1 = token;
        }
    }

    return filename + strlen(filename) - (1 + strlen(ext1) + ((strcmp(ext0, "tar") == 0) ? strlen(ext0) : -1)) - 1;
}

#endif /* LIBACQUIRE_IMPLEMENTATION */

#endif /* LIBACQUIRE_ACQUIRE_FILEUTILS_H */

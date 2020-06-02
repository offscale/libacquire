#ifndef LIBACQUIRE_FILEUTILS_H
#define LIBACQUIRE_FILEUTILS_H

#include "stdbool.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <Fileapi.h>
#include <io.h>
#else

#include <sys/stat.h>
#include <unistd.h>

#endif

extern bool is_directory(const char *path) {
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

extern bool is_file(const char *path) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    const DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_NORMAL));
#else
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return false;
    return S_ISREG(statbuf.st_mode);
#endif
}

extern bool exists(const char *path) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    return _access(path, 0) != -1;
#else
    return access(path, 0) != -1;
#endif
}

#endif /* LIBACQUIRE_FILEUTILS_H */

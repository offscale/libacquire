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

    return dwAttrib != INVALID_FILE_ATTRIBUTES && !is_directory(path);
#else
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return false;
    return S_ISREG(statbuf.st_mode);
#endif
}

extern bool exists(const char *path) {
    return
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        _access
#else
            access
#endif
                    (path, 0) != -1;
}


extern off_t filesize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

extern bool is_relative(const char *filename) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    return !filename || !*filename || (*filename != '\\' && filename[1] != ':');
#else
    return filename[0] != '/';
#endif
}

#endif /* LIBACQUIRE_ACQUIRE_FILEUTILS_H */

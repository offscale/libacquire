#ifndef LIBACQUIRE_FILEUTILS_H
#define LIBACQUIRE_FILEUTILS_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define OS_WINDOWS
#elif defined(__posix__)
#define OS_POSIX
#endif

#ifdef OS_WINDOWS
#include <Fileapi.h>
#elif defined(OS_POSIX)
#include <sys/stat.h>
#endif

extern int isDirectory(const char *path) {
#ifdef OS_WINDOWS
    DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(OS_POSIX)
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
#endif
}

#endif //LIBACQUIRE_FILEUTILS_H

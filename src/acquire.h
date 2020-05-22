#ifndef LIBACQUIRE_ACQUIRE_H
#define LIBACQUIRE_ACQUIRE_H

#include <stdlib.h>
#include <string.h>

#include "stdbool.h"
#include "checksums.h"
#include "stringutils.h"
#include "fileutils.h"

#ifndef MAX_FILENAME
#define MAX_FILENAME 255
#endif

extern const char* get_download_dir();
extern bool is_downloaded(const char *filename, enum Checksums checksum,
                          const char *hash, const char *target_directory) {
    char *full_local_fname;
    const char *fname = strdup(filename);
    const char *fname_contents;
    FILE *file;
    bool res = false;

    if (target_directory == NULL)
        target_directory = get_download_dir();
    if (filename == NULL || strlen(filename) == 0 || !is_directory(target_directory))
        return false;

    full_local_fname = strdup(target_directory);

    if (is_url(filename) || strchr(filename, '/') != NULL)
        fname = get_path_from_url(filename);

    strncat(full_local_fname, "/", 1);
    strncat(full_local_fname, fname, MAX_FILENAME);

    if (!is_file(full_local_fname))
        return false;


    /* TODO: Checksum verification
    file = fopen(full_local_fname, "rb");
    // fread()
    if (checksum == SHA256)
        res = sha256(fname_contents, hash);
    else if (checksum == SHA512)
        res = sha512(fname_contents, hash);

    fclose(file);
    */

    return res;
}

extern int download(const char*, const char*, const char[248], bool, size_t, size_t);
extern int download_many(const char*[], const char*, const char*, bool, size_t, size_t);

#endif /* LIBACQUIRE_ACQUIRE_H */

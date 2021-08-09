#ifndef LIBACQUIRE_ACQUIRE_H
#define LIBACQUIRE_ACQUIRE_H

#include <stdlib.h>
#include <string.h>

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif
#include "acquire_string_utils.h"
#include "acquire_string_extras.h"
#include "acquire_fileutils.h"
#include "acquire_config.h"

#include "acquire_checksums.h"

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)
#include "acquire_openssl.h"
#elif defined(USE_WINCRYPT)
#include "acquire_wincrypt.h"
#endif

#ifndef MAX_FILENAME
#define MAX_FILENAME 255
#endif

#ifndef NAME_MAX
#define NAME_MAX 4096
#endif

extern const char *get_download_dir();

extern enum Checksum string2checksum(const char *s) {
    if (strncasecmp(s, "SHA256", 6) == 0)
        return LIBACQUIRE_SHA256;
    else if (strncasecmp(s, "SHA512", 6) == 0)
        return LIBACQUIRE_SHA512;
    else return UNSUPPORTED;
}

extern bool is_downloaded(const char *url, enum Checksum checksum,
                          const char *hash, const char *target_directory) {
    char full_local_fname[NAME_MAX + 1];
    const char *filename = is_url(url) ? get_path_from_url(url) : url;

    if (target_directory == NULL)
        target_directory = get_download_dir();

    if (filename == NULL || strlen(filename) == 0 || !is_directory(target_directory))
        return false;

    snprintf(full_local_fname, NAME_MAX + 1,
             "%s/%s", target_directory, filename);

    if (!is_file(full_local_fname))
        return false;

    switch (checksum) {
        case LIBACQUIRE_SHA256:
            return sha256(full_local_fname, hash);
        case LIBACQUIRE_SHA512:
            /* return sha512(full_local_fname, hash); */
        case UNSUPPORTED:
        default:
            return false;
    }
}

extern int download(const char *, enum Checksum, const char *, const char[248], bool, size_t, size_t);

extern int download_many(const char *[], const char *[], enum Checksum[], const char *, bool, size_t, size_t);

/*
 * TODO: Consider using `#ifdef USE_WINCRYPT` style switches to include the relevant header here,
 * TODO: Or alternatively output the entire relevant header source here (programmatically);
 * TODO: Or alternatively amalgamate all headers content here below and allow includer to `#define USE_OPENSSL` &etc.
 * TODO: So others can just `#include <acquire.h>`
 */

#endif /* LIBACQUIRE_ACQUIRE_H */

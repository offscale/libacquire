/*
 * Prototype for download API
 *
 * Always `#include` this when adding new download implementations,
 * to ensure the implementation matches the prototype.
 * */

#ifndef LIBACQUIRE_ACQUIRE_DOWNLOAD_H
#define LIBACQUIRE_ACQUIRE_DOWNLOAD_H

#include <stdlib.h>
#include <string.h>

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif
#include "acquire_url_utils.h"
#include "acquire_string_extras.h"
#include "acquire_fileutils.h"
#include "acquire_config.h"

#include "acquire_checksums.h"

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)
#include "acquire_openssl.h"
#elif defined(USE_WINCRYPT)
#include "acquire_wincrypt.h"
#endif

extern const char *get_download_dir();

extern bool is_downloaded(
        const char *, enum Checksum,
        const char *, const char *
);

extern int download(
        const char *, enum Checksum,
        const char *, const char[248],
        bool, size_t, size_t
);

extern int download_many(
        const char *[], const char *[], enum Checksum[],
        const char *, bool, size_t, size_t
);

#endif /* LIBACQUIRE_ACQUIRE_DOWNLOAD_H */

/*
 * Prototype for download API
 *
 * Always `#include` this when adding new download implementations,
 * to ensure the implementation matches the prototype.
 * */

#ifndef LIBACQUIRE_ACQUIRE_DOWNLOAD_H
#define LIBACQUIRE_ACQUIRE_DOWNLOAD_H

#include <stdlib.h>

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* defined(HAS_STDBOOL) && !defined(bool) */
#include "acquire_config.h"
#include "acquire_fileutils.h"
#include "acquire_url_utils.h"

#include "acquire_checksums.h"
#include "libacquire_export.h"

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)
#include "acquire_openssl.h"
#elif defined(USE_WINCRYPT)
#include "acquire_wincrypt.h"
#endif /* defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL) */

extern LIBACQUIRE_LIB_EXPORT const char *get_download_dir(void);

extern LIBACQUIRE_LIB_EXPORT bool is_downloaded(const char *, enum Checksum,
                                                const char *, const char *);

extern LIBACQUIRE_LIB_EXPORT int download(const char *, enum Checksum,
                                          const char *, const char[NAME_MAX],
                                          bool, size_t, size_t);

extern LIBACQUIRE_LIB_EXPORT int download_many(const char *[], const char *[],
                                               enum Checksum[], const char *,
                                               bool, size_t, size_t);

#endif /* ! LIBACQUIRE_ACQUIRE_DOWNLOAD_H */

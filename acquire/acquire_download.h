#ifndef LIBACQUIRE_ACQUIRE_DOWNLOAD_H
#define LIBACQUIRE_ACQUIRE_DOWNLOAD_H

/**
 * @file acquire_download.h
 * @brief Functionality to download files from network locations.
 *
 * This module handles downloading files via protocols like HTTP(S),
 * providing progress monitoring and retry logic support.
 *
 * NOTE: Always `#include` this when adding new download implementations,
 * to ensure the implementation matches the prototype.
 */

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */
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

extern LIBACQUIRE_EXPORT const char *get_download_dir(void);

extern LIBACQUIRE_EXPORT bool is_downloaded(const char *, enum Checksum,
                                            const char *, const char *);

extern LIBACQUIRE_EXPORT int download(const char *, enum Checksum, const char *,
                                      const char *, bool, size_t, size_t);

extern LIBACQUIRE_EXPORT int download_many(const char *[], const char *[],
                                           enum Checksum[], const char *, bool,
                                           size_t, size_t);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_DOWNLOAD_H */

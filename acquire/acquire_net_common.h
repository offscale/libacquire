/*
 * Networking helper API
 *
 * Provides minimal networking-related helpers used throughout libacquire.
 * */

#ifndef LIBACQUIRE_ACQUIRE_NET_COMMON_H
#define LIBACQUIRE_ACQUIRE_NET_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_common_defs.h"
#include "acquire_config.h"

/**
 * Check if a file represented by URL is already downloaded locally and matches
 * the checksum.
 *
 * @param url URL or filename
 * @param checksum checksum enum type, e.g., LIBACQUIRE_SHA256
 * @param hash expected hash string to verify
 * @param target_location directory to check in (if NULL, defaults to
 * ".downloads")
 * @return true if already downloaded and checksum verifies, else false
 */
extern LIBACQUIRE_EXPORT bool is_downloaded(const char *url,
                                            enum Checksum checksum,
                                            const char *hash,
                                            const char *target_location);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_NET_COMMON_H */

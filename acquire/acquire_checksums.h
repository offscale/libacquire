#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_handle.h"
#include "libacquire_export.h"

/**
 * @brief Enumeration of supported checksum algorithms.
 */
enum Checksum {
  LIBACQUIRE_CRC32C,
  LIBACQUIRE_SHA256,
  LIBACQUIRE_SHA512,
  LIBACQUIRE_UNSUPPORTED_CHECKSUM
};

/**
 * @brief Converts a string name (e.g., "sha256") to a Checksum enum.
 * @return The corresponding enum, or LIBACQUIRE_UNSUPPORTED_CHECKSUM.
 */
extern LIBACQUIRE_EXPORT enum Checksum string2checksum(const char *);

/* --- Asynchronous Verification API --- */

/**
 * @brief Begins an asynchronous checksum verification (non-blocking).
 * @param handle An initialized acquire handle.
 * @param filepath The path to the file to verify.
 * @param algorithm The checksum algorithm to use.
 * @param expected_hash The expected hexadecimal checksum string.
 * @return 0 on success, non-zero on failure.
 */
extern LIBACQUIRE_EXPORT int
acquire_verify_async_start(struct acquire_handle *handle, const char *filepath,
                           enum Checksum algorithm, const char *expected_hash);

/**
 * @brief Polls the status of an asynchronous verification, processing a chunk.
 * @param handle The handle for the in-progress operation.
 * @return The current status of the operation.
 */
extern LIBACQUIRE_EXPORT enum acquire_status
acquire_verify_async_poll(struct acquire_handle *handle);

/**
 * @brief Requests cancellation of an asynchronous verification.
 * @param handle The handle for the operation to be cancelled.
 */
extern LIBACQUIRE_EXPORT void
acquire_verify_async_cancel(struct acquire_handle *handle);

/* --- Synchronous Verification API --- */

/**
 * @brief Verifies a file's checksum synchronously (blocking).
 * @return 0 if the checksum matches, -1 on failure or mismatch.
 */
extern LIBACQUIRE_EXPORT int acquire_verify_sync(struct acquire_handle *handle,
                                                 const char *filepath,
                                                 enum Checksum algorithm,
                                                 const char *expected_hash);

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_CHECKSUMS_IMPL)
enum Checksum string2checksum(const char *const s) {
  if (s == NULL)
    return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
  else if (strncasecmp(s, "CRC32C", 6) == 0)
    return LIBACQUIRE_CRC32C;
  else if (strncasecmp(s, "SHA256", 6) == 0)
    return LIBACQUIRE_SHA256;
  else if (strncasecmp(s, "SHA512", 6) == 0)
    return LIBACQUIRE_SHA512;
  else
    return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
}
#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_CHECKSUMS_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !LIBACQUIRE_ACQUIRE_CHECKSUMS_H */

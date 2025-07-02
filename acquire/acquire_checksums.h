#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "acquire_handle.h"
#include "libacquire_export.h"
#include <string.h> /* strncasecmp */

/** Checksum algorithms supported */
enum Checksum {
  LIBACQUIRE_CRC32C,
  LIBACQUIRE_SHA256,
  LIBACQUIRE_SHA512,
  LIBACQUIRE_UNSUPPORTED_CHECKSUM
};

/**
 * @brief Convert string to checksum enum.
 *
 * Supports at least "CRC32C", "SHA256", "SHA512".
 *
 * @param s String of algorithm name (case-insensitive)
 * @return Enum value or LIBACQUIRE_UNSUPPORTED_CHECKSUM if unknown.
 */
extern LIBACQUIRE_EXPORT enum Checksum string2checksum(const char *s);

/* --- Asynchronous API --- */

extern LIBACQUIRE_EXPORT int
acquire_verify_async_start(struct acquire_handle *handle, const char *filepath,
                           enum Checksum algorithm, const char *expected_hash);

extern LIBACQUIRE_EXPORT enum acquire_status
acquire_verify_async_poll(struct acquire_handle *handle);

extern LIBACQUIRE_EXPORT void
acquire_verify_async_cancel(struct acquire_handle *handle);

/* --- Synchronous API --- */

extern LIBACQUIRE_EXPORT int acquire_verify_sync(struct acquire_handle *handle,
                                                 const char *filepath,
                                                 enum Checksum algorithm,
                                                 const char *expected_hash);

/* --- Backend internal function declarations --- */

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
int _openssl_verify_async_start(struct acquire_handle *handle,
                                const char *filepath, enum Checksum algorithm,
                                const char *expected_hash);
enum acquire_status _openssl_verify_async_poll(struct acquire_handle *handle);
void _openssl_verify_async_cancel(struct acquire_handle *handle);
#endif

#ifdef LIBACQUIRE_USE_WINCRYPT
int _wincrypt_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash);
enum acquire_status _wincrypt_verify_async_poll(struct acquire_handle *handle);
void _wincrypt_verify_async_cancel(struct acquire_handle *handle);
#endif

#ifdef LIBACQUIRE_USE_LIBRHASH
int _librhash_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash);
enum acquire_status _librhash_verify_async_poll(struct acquire_handle *handle);
void _librhash_verify_async_cancel(struct acquire_handle *handle);
#endif

#ifdef LIBACQUIRE_USE_CRC32C
int _crc32c_verify_async_start(struct acquire_handle *handle,
                               const char *filepath, enum Checksum algorithm,
                               const char *expected_hash);
enum acquire_status _crc32c_verify_async_poll(struct acquire_handle *handle);
void _crc32c_verify_async_cancel(struct acquire_handle *handle);
#endif

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_CHECKSUMS_IMPL)

int acquire_verify_async_start(struct acquire_handle *handle,
                               const char *filepath, enum Checksum algorithm,
                               const char *expected_hash) {
  if (!handle || !filepath || !expected_hash) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                               "Invalid arguments");
    return -1;
  }

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  if (algorithm == LIBACQUIRE_SHA256 || algorithm == LIBACQUIRE_SHA512)
    return _openssl_verify_async_start(handle, filepath, algorithm,
                                       expected_hash);
#endif

#ifdef LIBACQUIRE_USE_WINCRYPT
  if (algorithm == LIBACQUIRE_SHA256 || algorithm == LIBACQUIRE_SHA512)
    return _wincrypt_verify_async_start(handle, filepath, algorithm,
                                        expected_hash);
#endif

#ifdef LIBACQUIRE_USE_LIBRHASH
  if (algorithm == LIBACQUIRE_CRC32C || algorithm == LIBACQUIRE_SHA256 ||
      algorithm == LIBACQUIRE_SHA512)
    return _librhash_verify_async_start(handle, filepath, algorithm,
                                        expected_hash);
#endif

#ifdef LIBACQUIRE_USE_CRC32C
  if (algorithm == LIBACQUIRE_CRC32C)
    return _crc32c_verify_async_start(handle, filepath, algorithm,
                                      expected_hash);
#endif

  acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
                           "Unsupported checksum algorithm");
  return -1;
}

enum acquire_status acquire_verify_async_poll(struct acquire_handle *handle) {
  if (!handle)
    return ACQUIRE_ERROR;

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  if (handle->backend_handle)
    return _openssl_verify_async_poll(handle);
#endif

#ifdef LIBACQUIRE_USE_WINCRYPT
  if (handle->backend_handle)
    return _wincrypt_verify_async_poll(handle);
#endif

#ifdef LIBACQUIRE_USE_LIBRHASH
  if (handle->backend_handle)
    return _librhash_verify_async_poll(handle);
#endif

#ifdef LIBACQUIRE_USE_CRC32C
  if (handle->backend_handle)
    return _crc32c_verify_async_poll(handle);
#endif

  return ACQUIRE_ERROR;
}

void acquire_verify_async_cancel(struct acquire_handle *handle) {
  if (!handle)
    return;

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  if (handle->backend_handle) {
    _openssl_verify_async_cancel(handle);
    return;
  }
#endif

#ifdef LIBACQUIRE_USE_WINCRYPT
  if (handle->backend_handle) {
    _wincrypt_verify_async_cancel(handle);
    return;
  }
#endif

#ifdef LIBACQUIRE_USE_LIBRHASH
  if (handle->backend_handle) {
    _librhash_verify_async_cancel(handle);
    return;
  }
#endif

#ifdef LIBACQUIRE_USE_CRC32C
  if (handle->backend_handle) {
    _crc32c_verify_async_cancel(handle);
    return;
  }
#endif
}

int acquire_verify_sync(struct acquire_handle *handle, const char *filepath,
                        enum Checksum algorithm, const char *expected_hash) {
  if (!handle || !filepath || !expected_hash)
    return -1;

  if (acquire_verify_async_start(handle, filepath, algorithm, expected_hash) !=
      0)
    return -1;

  while (acquire_verify_async_poll(handle) == ACQUIRE_IN_PROGRESS)
    ;

  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

enum Checksum string2checksum(const char *const s) {
  if (s == NULL)
    return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
  if (strncasecmp(s, "CRC32C", 6) == 0)
    return LIBACQUIRE_CRC32C;
  if (strncasecmp(s, "SHA256", 6) == 0)
    return LIBACQUIRE_SHA256;
  if (strncasecmp(s, "SHA512", 6) == 0)
    return LIBACQUIRE_SHA512;
  return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
}

#endif /* LIBACQUIRE_IMPLEMENTATION && LIBACQUIRE_CHECKSUMS_IMPL */

#ifdef __cplusplus
}
#endif

#endif /* LIBACQUIRE_ACQUIRE_CHECKSUMS_H */

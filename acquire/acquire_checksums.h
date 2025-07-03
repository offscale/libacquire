#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

  /* duplicate labels so can't have a 'global' switch/case */
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) || defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  switch (algorithm) {
    case LIBACQUIRE_SHA256:
    case LIBACQUIRE_SHA512:
      return _openssl_verify_async_start(handle, filepath, algorithm,
                                         expected_hash);
    default: break;
  }
#endif /* defined(LIBACQUIRE_USE_COMMON_CRYPTO) || defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL) */
#ifdef LIBACQUIRE_USE_WINCRYPT
  switch (algorithm) {
    case LIBACQUIRE_SHA256:
    case LIBACQUIRE_SHA512:
      return _wincrypt_verify_async_start(handle, filepath, algorithm,
                                         expected_hash);
    default: break;
  }
#endif /* LIBACQUIRE_USE_WINCRYPT */
#ifdef LIBACQUIRE_USE_LIBRHASH
  switch (algorithm) {
    case LIBACQUIRE_CRC32C:
    case LIBACQUIRE_SHA256:
    case LIBACQUIRE_SHA512:
      return _librhash_verify_async_start(handle, filepath, algorithm,
                                        expected_hash);
    default:
      break;
  }
#endif /* LIBACQUIRE_USE_LIBRHASH */
#ifdef LIBACQUIRE_USE_CRC32C
  switch (algorithm) {
    case LIBACQUIRE_CRC32C:
      return _crc32c_verify_async_start(handle, filepath, algorithm,
                                      expected_hash);
    default: break;
  }
#endif /* LIBACQUIRE_USE_CRC32C */

  acquire_handle_set_error(
      handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
      "Unsupported checksum algorithm or no backend for it");
  return -1;
}

enum acquire_status acquire_verify_async_poll(struct acquire_handle *handle) {
  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  return _openssl_verify_async_poll(handle);
#elif defined(LIBACQUIRE_USE_WINCRYPT)
  return _wincrypt_verify_async_poll(handle);
#elif defined(LIBACQUIRE_USE_LIBRHASH)
  return _librhash_verify_async_poll(handle);
#elif defined(LIBACQUIRE_USE_CRC32C)
  return _crc32c_verify_async_poll(handle);
#else
  acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                           "No checksum backend compiled");
  return ACQUIRE_ERROR;
#endif
}

void acquire_verify_async_cancel(struct acquire_handle *handle) {
  if (!handle || !handle->backend_handle)
    return;

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  _openssl_verify_async_cancel(handle);
#elif defined(LIBACQUIRE_USE_WINCRYPT)
  _wincrypt_verify_async_cancel(handle);
#elif defined(LIBACQUIRE_USE_LIBRHASH)
  _librhash_verify_async_cancel(handle);
#elif defined(LIBACQUIRE_USE_CRC32C)
  _crc32c_verify_async_cancel(handle);
#endif
}

int acquire_verify_sync(struct acquire_handle *handle, const char *filepath,
                        enum Checksum algorithm, const char *expected_hash) {
  enum acquire_status status;
  if (!handle || !filepath || !expected_hash)
    return -1;

  if (acquire_verify_async_start(handle, filepath, algorithm, expected_hash) !=
      0)
    return -1;

  while ((status = acquire_verify_async_poll(handle)) == ACQUIRE_IN_PROGRESS)
    ;

  return (status == ACQUIRE_COMPLETE) ? 0 : -1;
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
#endif /* __cplusplus */

#endif /* LIBACQUIRE_ACQUIRE_CHECKSUMS_H */

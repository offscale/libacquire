#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "acquire_handle.h"
#include "libacquire_export.h"

#include "acquire_crc32c.h"
#include "acquire_librhash.h"
#include "acquire_openssl.h"
#include "acquire_wincrypt.h"

extern LIBACQUIRE_EXPORT enum Checksum string2checksum(const char *s);
extern LIBACQUIRE_EXPORT int
acquire_verify_async_start(struct acquire_handle *handle, const char *filepath,
                           enum Checksum algorithm, const char *expected_hash);
extern LIBACQUIRE_EXPORT enum acquire_status
acquire_verify_async_poll(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT void
acquire_verify_async_cancel(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT int acquire_verify_sync(struct acquire_handle *handle,
                                                 const char *filepath,
                                                 enum Checksum algorithm,
                                                 const char *expected_hash);

#if defined(LIBACQUIRE_IMPLEMENTATION)
#ifndef ACQUIRE_CHECKSUMS_IMPL_
#define ACQUIRE_CHECKSUMS_IMPL_
#include <string.h>

enum Checksum string2checksum(const char *const s) {
  if (s == NULL)
    return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
  if (strcasecmp(s, "crc32c") == 0)
    return LIBACQUIRE_CRC32C;
  if (strcasecmp(s, "sha256") == 0)
    return LIBACQUIRE_SHA256;
  if (strcasecmp(s, "sha512") == 0)
    return LIBACQUIRE_SHA512;
  return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
}

int acquire_verify_async_start(struct acquire_handle *handle,
                               const char *filepath, enum Checksum algorithm,
                               const char *expected_hash) {
  if (!handle || !filepath || !expected_hash) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                               "Invalid arguments");
    return -1;
  }
  handle->active_backend = ACQUIRE_BACKEND_NONE;
  handle->status = ACQUIRE_IDLE;
  handle->error.code = ACQUIRE_OK;
  handle->error.message[0] = '\0';
#if defined(LIBACQUIRE_USE_LIBRHASH)
  if (_librhash_verify_async_start(handle, filepath, algorithm,
                                   expected_hash) == 0) {
    handle->active_backend = ACQUIRE_BACKEND_CHECKSUM_LIBRHASH;
    return 0;
  }
  if (handle->error.code != ACQUIRE_OK)
    return -1;
#endif
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  if (_openssl_verify_async_start(handle, filepath, algorithm, expected_hash) ==
      0) {
    handle->active_backend = ACQUIRE_BACKEND_CHECKSUM_OPENSSL;
    return 0;
  }
  if (handle->error.code != ACQUIRE_OK)
    return -1;
#endif
#if defined(LIBACQUIRE_USE_WINCRYPT)
  if (_wincrypt_verify_async_start(handle, filepath, algorithm,
                                   expected_hash) == 0) {
    handle->active_backend = ACQUIRE_BACKEND_CHECKSUM_WINCRYPT;
    return 0;
  }
  if (handle->error.code != ACQUIRE_OK)
    return -1;
#endif
#if defined(LIBACQUIRE_USE_CRC32C)
  if (_crc32c_verify_async_start(handle, filepath, algorithm, expected_hash) ==
      0) {
    handle->active_backend = ACQUIRE_BACKEND_CHECKSUM_CRC32C;
    return 0;
  }
  if (handle->error.code != ACQUIRE_OK)
    return -1;
#endif
  acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
                           "Unsupported checksum or no backend");
  return -1;
}

enum acquire_status acquire_verify_async_poll(struct acquire_handle *handle) {
  if (!handle)
    return ACQUIRE_ERROR;
  switch (handle->active_backend) {
#if defined(LIBACQUIRE_USE_LIBRHASH)
  case ACQUIRE_BACKEND_CHECKSUM_LIBRHASH:
    return _librhash_verify_async_poll(handle);
#endif
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
  case ACQUIRE_BACKEND_CHECKSUM_OPENSSL:
    return _openssl_verify_async_poll(handle);
#endif
#if defined(LIBACQUIRE_USE_WINCRYPT)
  case ACQUIRE_BACKEND_CHECKSUM_WINCRYPT:
    return _wincrypt_verify_async_poll(handle);
#endif
#if defined(LIBACQUIRE_USE_CRC32C)
  case ACQUIRE_BACKEND_CHECKSUM_CRC32C:
    return _crc32c_verify_async_poll(handle);
#endif
  default:
    if (handle->status != ACQUIRE_IN_PROGRESS)
      return handle->status;
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "No active backend");
    return ACQUIRE_ERROR;
  }
}

void acquire_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

int acquire_verify_sync(struct acquire_handle *handle, const char *filepath,
                        enum Checksum algorithm, const char *expected_hash) {
  enum acquire_status status;
  if (acquire_verify_async_start(handle, filepath, algorithm, expected_hash) !=
      0)
    return -1;
  do {
    status = acquire_verify_async_poll(handle);
  } while (status == ACQUIRE_IN_PROGRESS);
  return (status == ACQUIRE_COMPLETE) ? 0 : -1;
}

#endif /* ACQUIRE_CHECKSUMS_IMPL_ */
#endif /* defined(LIBACQUIRE_IMPLEMENTATION) */

#ifdef __cplusplus
}
#endif

#endif /* !LIBACQUIRE_ACQUIRE_CHECKSUMS_H */

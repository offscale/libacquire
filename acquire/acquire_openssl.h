#ifndef LIBACQUIRE_ACQUIRE_OPENSSL_H
#define LIBACQUIRE_ACQUIRE_OPENSSL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_common_defs.h"

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO
#include <CommonCrypto/CommonDigest.h>
#elif defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL ||             \
    defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL
#include <openssl/evp.h>
#endif

struct acquire_handle;

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO ||   \
    defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL ||               \
    defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL
int _openssl_verify_async_start(struct acquire_handle *handle,
                                const char *filepath, enum Checksum algorithm,
                                const char *expected_hash);
enum acquire_status _openssl_verify_async_poll(struct acquire_handle *handle);
void _openssl_verify_async_cancel(struct acquire_handle *handle);
#endif

#ifdef LIBACQUIRE_IMPLEMENTATION

#if (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO ||  \
     defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL ||              \
     defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL)

#ifndef EVP_MAX_MD_SIZE
#define EVP_MAX_MD_SIZE 64
#endif /* !EVP_MAX_MD_SIZE */

#include "acquire_handle.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if !defined(LIBACQUIRE_USE_COMMON_CRYPTO) || !LIBACQUIRE_USE_COMMON_CRYPTO
#include <openssl/err.h>
#endif

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif /* !CHUNK_SIZE */

struct openssl_backend {
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO
  union {
    CC_SHA256_CTX sha256;
    CC_SHA512_CTX sha512;
  } ctx;
  enum Checksum algorithm;
#else
  EVP_MD_CTX *ctx;
#endif
  FILE *file;
  char expected_hash[130];
};

static void cleanup_openssl_backend(struct acquire_handle *handle) {
  if (handle && handle->backend_handle) {
    struct openssl_backend *be =
        (struct openssl_backend *)handle->backend_handle;
#if !defined(LIBACQUIRE_USE_COMMON_CRYPTO) || !LIBACQUIRE_USE_COMMON_CRYPTO
    if (be->ctx)
      EVP_MD_CTX_free(be->ctx);
#endif
    if (be->file)
      fclose(be->file);
    free(be);
    handle->backend_handle = NULL;
  }
}

int _openssl_verify_async_start(struct acquire_handle *handle,
                                const char *filepath, enum Checksum algorithm,
                                const char *expected_hash) {
  struct openssl_backend *be;
  size_t expected_len = 0;
  switch (algorithm) {
  case LIBACQUIRE_SHA256:
    expected_len = 64;
    break;
  case LIBACQUIRE_SHA512:
    expected_len = 128;
    break;
  default:
    return -1;
  }
  if (strlen(expected_hash) != expected_len) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_CHECKSUM_FORMAT,
                             "Invalid hash length for selected algorithm");
    return -1;
  }
  be = (struct openssl_backend *)calloc(1, sizeof(struct openssl_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY,
                             "openssl backend memory allocation failed");
    return -1;
  }
  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED, "%s",
                             strerror(errno));
    free(be);
    return -1;
  }

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO
  be->algorithm = algorithm;
  switch (algorithm) {
  case LIBACQUIRE_SHA256:
    CC_SHA256_Init(&be->ctx.sha256);
    break;
  case LIBACQUIRE_SHA512:
    CC_SHA512_Init(&be->ctx.sha512);
    break;
  default:
    break;
  }
#else
  const EVP_MD *md =
      (algorithm == LIBACQUIRE_SHA256) ? EVP_sha256() : EVP_sha512();
  be->ctx = EVP_MD_CTX_new();
  if (!be->ctx || (1 != EVP_DigestInit_ex(be->ctx, md, NULL))) {
    cleanup_openssl_backend(handle);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "EVP_DigestInit_ex failed");
    return -1;
  }
#endif

  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status _openssl_verify_async_poll(struct acquire_handle *handle) {
  struct openssl_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;
  if (!handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (!handle->backend_handle) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "In-progress poll with NULL backend");
    return ACQUIRE_ERROR;
  }
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Checksum cancelled");
    cleanup_openssl_backend(handle);
    return ACQUIRE_ERROR;
  }
  be = (struct openssl_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, sizeof(buffer), be->file);
  if (bytes_read > 0) {
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO
    switch (be->algorithm) {
    case LIBACQUIRE_SHA256:
      CC_SHA256_Update(&be->ctx.sha256, buffer, (CC_LONG)bytes_read);
      break;
    case LIBACQUIRE_SHA512:
      CC_SHA512_Update(&be->ctx.sha512, buffer, (CC_LONG)bytes_read);
      break;
    default:
      break;
    }
#else
    if (1 != EVP_DigestUpdate(be->ctx, buffer, bytes_read)) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "EVP_DigestUpdate failed");
    }
#endif
    if (handle->error.code == ACQUIRE_OK) {
      handle->bytes_processed += bytes_read;
      return ACQUIRE_IN_PROGRESS;
    }
  }
  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED, "%s",
                             strerror(errno));
  } else {
    unsigned char hash[EVP_MAX_MD_SIZE];
    char computed_hex[EVP_MAX_MD_SIZE * 2 + 1];
    unsigned int len = 0;
    int i;
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO
    switch (be->algorithm) {
    case LIBACQUIRE_SHA256:
      len = CC_SHA256_DIGEST_LENGTH;
      CC_SHA256_Final(hash, &be->ctx.sha256);
      break;
    case LIBACQUIRE_SHA512:
      len = CC_SHA512_DIGEST_LENGTH;
      CC_SHA512_Final(hash, &be->ctx.sha512);
      break;
    default:
      break;
    }
#else
    if (1 != EVP_DigestFinal_ex(be->ctx, hash, &len)) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "EVP_DigestFinal_ex failed");
    }
#endif
    if (handle->error.code == ACQUIRE_OK) {
      for (i = 0; (unsigned int)i < len; i++)
        sprintf(computed_hex + (i * 2), "%02x", hash[i]);
      computed_hex[len * 2] = '\0';
      if (strncasecmp(computed_hex, be->expected_hash, len * 2) == 0)
        handle->status = ACQUIRE_COMPLETE;
      else
        acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                                 "Hash mismatch: expected %s, got %s",
                                 be->expected_hash, computed_hex);
    }
  }
  cleanup_openssl_backend(handle);
  return handle->status;
}

void _openssl_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}
#endif /* (defined(LIBACQUIRE_USE_COMMON_CRYPTO) &&                            \
          LIBACQUIRE_USE_COMMON_CRYPTO || defined(LIBACQUIRE_USE_OPENSSL) &&   \
          LIBACQUIRE_USE_OPENSSL || defined(LIBACQUIRE_USE_LIBRESSL) &&        \
          LIBACQUIRE_USE_LIBRESSL) */
#endif /* defined(LIBACQUIRE_IMPLEMENTATION) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_OPENSSL_H */

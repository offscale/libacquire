#ifndef LIBACQUIRE_ACQUIRE_OPENSSL_H
#define LIBACQUIRE_ACQUIRE_OPENSSL_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes needed for declarations and struct definitions
#include "acquire_checksums.h"
#include <stdio.h>

// Conditional struct and API includes based on backend
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO)
#include <CommonCrypto/CommonDigest.h>
struct openssl_backend {
  union {
    CC_SHA256_CTX sha256;
    CC_SHA512_CTX sha512;
  } ctx;
  FILE *file;
  char expected_hash[130];
  enum Checksum algorithm;
};
#elif defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
#include <openssl/evp.h>
struct openssl_backend {
  EVP_MD_CTX *ctx;
  FILE *file;
  char expected_hash[130];
  const EVP_MD *md;
};
#endif

// private function declarations
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                   \
    defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)
int _openssl_verify_async_start(struct acquire_handle *handle,
                                const char *filepath, enum Checksum algorithm,
                                const char *expected_hash);
enum acquire_status _openssl_verify_async_poll(struct acquire_handle *handle);
void _openssl_verify_async_cancel(struct acquire_handle *handle);
#endif

#if defined(LIBACQUIRE_IMPLEMENTATION) && (defined(LIBACQUIRE_USE_COMMON_CRYPTO) || defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL))

#include "acquire_handle.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if !(defined(LIBACQUIRE_USE_COMMON_CRYPTO))
#include <openssl/err.h>
#endif

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif

static void cleanup_openssl_backend(struct acquire_handle *handle) {
  if (handle && handle->backend_handle) {
    /* No dynamic cleanup needed for CommonCrypto context; EVP needs free */
#if !defined(LIBACQUIRE_USE_COMMON_CRYPTO)
    struct openssl_backend *be =
        (struct openssl_backend *)handle->backend_handle;
    if (be->ctx)
      EVP_MD_CTX_free(be->ctx);
#endif
    struct openssl_backend *be =
        (struct openssl_backend *)handle->backend_handle;
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

  be = (struct openssl_backend *)calloc(1, sizeof(struct openssl_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY, "openssl backend");
    return -1;
  }

  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED, "%s", strerror(errno));
    free(be);
    return -1;
  }

#if defined(LIBACQUIRE_USE_COMMON_CRYPTO)
  be->algorithm = algorithm;
  switch (algorithm) {
  case LIBACQUIRE_SHA256:
    expected_len = 64;
    CC_SHA256_Init(&be->ctx.sha256);
    break;
  case LIBACQUIRE_SHA512:
    expected_len = 128;
    CC_SHA512_Init(&be->ctx.sha512);
    break;
  default:
    free(be);
    return -1;
  }
#else
  {
    const EVP_MD *md = NULL;
    switch (algorithm) {
    case LIBACQUIRE_SHA256:
      md = EVP_sha256();
      expected_len = 64;
      break;
    case LIBACQUIRE_SHA512:
      md = EVP_sha512();
      expected_len = 128;
      break;
    default:
      free(be);
      fclose(be->file);
      return -1;
    }
    be->md = md;
    be->ctx = EVP_MD_CTX_new();
    if (!be->ctx) {
      cleanup_openssl_backend(handle);
      acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY, "EVP_MD_CTX_new");
      return -1;
    }
    if (1 != EVP_DigestInit_ex(be->ctx, md, NULL)) {
      cleanup_openssl_backend(handle);
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "EVP_DigestInit_ex failed");
      return -1;
    }
  }
#endif

  if (strlen(expected_hash) != expected_len) {
    cleanup_openssl_backend(handle);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT, "Incorrect hash length for selected algorithm");
    return -1;
  }

  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  be->expected_hash[sizeof(be->expected_hash) - 1] = '\0';
  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status _openssl_verify_async_poll(struct acquire_handle *handle) {
  struct openssl_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;

  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED, "Verification cancelled");
    cleanup_openssl_backend(handle);
    return ACQUIRE_ERROR;
  }

  be = (struct openssl_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, sizeof(buffer), be->file);

  if (bytes_read > 0) {
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO)
    if (be->algorithm == LIBACQUIRE_SHA256) CC_SHA256_Update(&be->ctx.sha256, buffer, (CC_LONG)bytes_read);
    else if (be->algorithm == LIBACQUIRE_SHA512) CC_SHA512_Update(&be->ctx.sha512, buffer, (CC_LONG)bytes_read);
#else
    if (1 != EVP_DigestUpdate(be->ctx, buffer, bytes_read)) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "EVP_DigestUpdate failed");
      handle->status = ACQUIRE_ERROR;
    }
#endif
    if (handle->status != ACQUIRE_ERROR) {
      handle->bytes_processed += bytes_read;
      return ACQUIRE_IN_PROGRESS;
    }
  }

  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED, "%s", strerror(errno));
    handle->status = ACQUIRE_ERROR;
  } else {
    unsigned char hash[CC_SHA512_DIGEST_LENGTH]; /* Max digest size */
    char computed_hex[130];
    unsigned int len = 0;
    int i;
#if defined(LIBACQUIRE_USE_COMMON_CRYPTO)
    if (be->algorithm == LIBACQUIRE_SHA256) {
      len = CC_SHA256_DIGEST_LENGTH;
      CC_SHA256_Final(hash, &be->ctx.sha256);
    } else if (be->algorithm == LIBACQUIRE_SHA512) {
      len = CC_SHA512_DIGEST_LENGTH;
      CC_SHA512_Final(hash, &be->ctx.sha512);
    }
#else
    if (1 != EVP_DigestFinal_ex(be->ctx, hash, &len)) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "EVP_DigestFinal_ex failed");
      handle->status = ACQUIRE_ERROR;
    }
#endif
    if (handle->status != ACQUIRE_ERROR) {
      for (i = 0; i < len; i++) {
        sprintf(computed_hex + (i * 2), "%02x", hash[i]);
      }
      computed_hex[len * 2] = '\0';
      if (strncasecmp(computed_hex, be->expected_hash, len * 2) == 0) {
        handle->status = ACQUIRE_COMPLETE;
      } else {
        acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "Hash mismatch. Expected %s, got %s", be->expected_hash, computed_hex);
      }
    }
  }

  cleanup_openssl_backend(handle);
  return handle->status;
}

void _openssl_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) && (defined(LIBACQUIRE_USE_COMMON_CRYPTO) || defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL)) */

#ifdef __cplusplus
}
#endif

#endif /* !LIBACQUIRE_ACQUIRE_OPENSSL_H */

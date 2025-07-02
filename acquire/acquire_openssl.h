#ifndef LIBACQUIRE_OPENSSL_H
#define LIBACQUIRE_OPENSSL_H

#if defined(LIBACQUIRE_IMPLEMENTATION) &&                                      \
    (defined(LIBACQUIRE_USE_COMMON_CRYPTO) ||                                  \
     defined(LIBACQUIRE_USE_OPENSSL) || defined(LIBACQUIRE_USE_LIBRESSL))

#ifdef __cplusplus
extern "C" {
#endif

#include "acquire_checksums.h"
#include "acquire_handle.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef LIBACQUIRE_USE_COMMON_CRYPTO
#include <CommonCrypto/CommonDigest.h>
#define SHA256_CTX CC_SHA256_CTX
#define SHA256_Init CC_SHA256_Init
#define SHA256_Update CC_SHA256_Update
#define SHA256_Final CC_SHA256_Final
#define SHA512_CTX CC_SHA512_CTX
#define SHA512_Init CC_SHA512_Init
#define SHA512_Update CC_SHA512_Update
#define SHA512_Final CC_SHA512_Final
#else
#include <openssl/sha.h>
#endif

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif

#ifndef SHA512_DIGEST_LENGTH
#define SHA512_DIGEST_LENGTH 64
#endif

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif

struct checksum_backend {
  FILE *file;
  enum Checksum algorithm;
  char expected_hash[SHA512_DIGEST_LENGTH * 2 + 1];
  union {
    SHA256_CTX sha256;
    SHA512_CTX sha512;
  } context;
};

static void cleanup_checksum_backend(struct acquire_handle *handle) {
  if (handle && handle->backend_handle) {
    struct checksum_backend *be =
        (struct checksum_backend *)handle->backend_handle;
    if (be->file)
      fclose(be->file);
    free(be);
    handle->backend_handle = NULL;
  }
}

int _openssl_verify_async_start(struct acquire_handle *handle,
                                const char *filepath, enum Checksum algorithm,
                                const char *expected_hash) {
  struct checksum_backend *be;
  if (!handle || !filepath || !expected_hash) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                             "Invalid arguments for verification");
    return -1;
  }

  be = (struct checksum_backend *)calloc(1, sizeof(struct checksum_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY,
                             "Could not allocate checksum backend");
    return -1;
  }

  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED, "%s",
                             strerror(errno));
    free(be);
    return -1;
  }

  handle->backend_handle = be;
  be->algorithm = algorithm;

  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  be->expected_hash[sizeof(be->expected_hash) - 1] = '\0';

  switch (algorithm) {
  case LIBACQUIRE_SHA256:
    SHA256_Init(&be->context.sha256);
    break;
  case LIBACQUIRE_SHA512:
    SHA512_Init(&be->context.sha512);
    break;
  default:
    acquire_handle_set_error(
        handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
        "Unsupported checksum algorithm for OpenSSL backend");
    cleanup_checksum_backend(handle);
    return -1;
  }

  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status _openssl_verify_async_poll(struct acquire_handle *handle) {
  struct checksum_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;
  int i;

  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Checksum verification cancelled");
    cleanup_checksum_backend(handle);
    return ACQUIRE_ERROR;
  }

  be = (struct checksum_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, CHUNK_SIZE, be->file);

  if (bytes_read > 0) {
    if (be->algorithm == LIBACQUIRE_SHA256)
      SHA256_Update(&be->context.sha256, buffer, bytes_read);
    else if (be->algorithm == LIBACQUIRE_SHA512)
      SHA512_Update(&be->context.sha512, buffer, bytes_read);
    else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
                               "Unsupported algorithm state in OpenSSL poll");
      cleanup_checksum_backend(handle);
      return ACQUIRE_ERROR;
    }
    handle->bytes_processed += bytes_read;
    return ACQUIRE_IN_PROGRESS;
  }

  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED, "%s",
                             strerror(errno));
  } else {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    char hex_hash[SHA512_DIGEST_LENGTH * 2 + 1];
    int hash_len;

    if (be->algorithm == LIBACQUIRE_SHA256) {
      SHA256_Final(hash, &be->context.sha256);
      hash_len = SHA256_DIGEST_LENGTH;
    } else {
      SHA512_Final(hash, &be->context.sha512);
      hash_len = SHA512_DIGEST_LENGTH;
    }

    for (i = 0; i < hash_len; i++)
      sprintf(hex_hash + (i * 2), "%02x", hash[i]);
    hex_hash[hash_len * 2] = '\0';

    if (strncasecmp(hex_hash, be->expected_hash, hash_len * 2) == 0) {
      handle->status = ACQUIRE_COMPLETE;
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "Checksum mismatch. Expected %s, got %s",
                               be->expected_hash, hex_hash);
      handle->status = ACQUIRE_ERROR;
    }
  }

  cleanup_checksum_backend(handle);
  return handle->status;
}

void _openssl_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#ifdef __cplusplus
}
#endif

#endif /* LIBACQUIRE_IMPLEMENTATION && (LIBACQUIRE_USE_COMMON_CRYPTO ||        \
          LIBACQUIRE_USE_OPENSSL) */

#endif /* !LIBACQUIRE_OPENSSL_H */

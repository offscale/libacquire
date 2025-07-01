#ifndef LIBACQUIRE_WINCRYPT_H
#define LIBACQUIRE_WINCRYPT_H

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_USE_WINCRYPT)

#ifdef __cplusplus
extern "C" {
#endif

#include "acquire_checksums.h"
#include "acquire_handle.h"
#include <wincrypt.h>

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif

/* Internal state for an async checksum operation. */
struct checksum_backend {
  FILE *file;
  HCRYPTPROV hProv;
  HCRYPTHASH hHash;
  char expected_hash[129]; /* Max SHA512 hex length + null */
};

static void cleanup_checksum_backend(struct acquire_handle *handle) {
  if (handle && handle->backend_handle) {
    struct checksum_backend *be =
        (struct checksum_backend *)handle->backend_handle;
    if (be->hHash)
      CryptDestroyHash(be->hHash);
    if (be->hProv)
      CryptReleaseContext(be->hProv, 0);
    if (be->file)
      fclose(be->file);
    free(be);
    handle->backend_handle = NULL;
  }
}

int acquire_verify_async_start(struct acquire_handle *handle,
                               const char *filepath, enum Checksum algorithm,
                               const char *expected_hash) {
  struct checksum_backend *be;
  ALG_ID alg_id;

  if (!handle || !filepath || !expected_hash) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                             "Invalid arguments for verification");
    return -1;
  }

  switch (algorithm) {
  case LIBACQUIRE_SHA256:
    alg_id = CALG_SHA_256;
    break;
  case LIBACQUIRE_SHA512:
    alg_id = CALG_SHA_512;
    break;
  default:
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
                             "Unsupported algorithm for WinCrypt backend");
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
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                             "Could not open file '%s'", filepath);
    free(be);
    return -1;
  }

  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  if (!CryptAcquireContext(&be->hProv, NULL, NULL, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "CryptAcquireContext failed");
    cleanup_checksum_backend(handle);
    return -1;
  }

  if (!CryptCreateHash(be->hProv, alg_id, 0, 0, &be->hHash)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "CryptCreateHash failed");
    cleanup_checksum_backend(handle);
    return -1;
  }

  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status acquire_verify_async_poll(struct acquire_handle *handle) {
  struct checksum_backend *be;
  BYTE buffer[CHUNK_SIZE];
  size_t bytes_read;

  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Checksum verification cancelled");
    cleanup_checksum_backend(handle);
    return handle->status;
  }

  be = (struct checksum_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, CHUNK_SIZE, be->file);

  if (bytes_read > 0) {
    if (!CryptHashData(be->hHash, buffer, (DWORD)bytes_read, 0)) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "CryptHashData failed");
      cleanup_checksum_backend(handle);
    }
    handle->bytes_processed += bytes_read;
    return ACQUIRE_IN_PROGRESS;
  }

  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED,
                             "Error reading file");
  } else {            /* EOF */
    BYTE rgbHash[64]; /* SHA512_DIGEST_LENGTH */
    DWORD cbHash = sizeof(rgbHash);
    char hex_hash[129];
    static const char rgbDigits[] = "0123456789abcdef";
    DWORD i, k = 0;

    if (CryptGetHashParam(be->hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
      for (i = 0; i < cbHash; i++) {
        hex_hash[k++] = rgbDigits[rgbHash[i] >> 4];
        hex_hash[k++] = rgbDigits[rgbHash[i] & 0xf];
      }
      hex_hash[k] = '\0';
      if (_stricmp(hex_hash, be->expected_hash) == 0) {
        handle->status = ACQUIRE_COMPLETE;
      } else {
        acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                                 "Checksum mismatch");
      }
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "CryptGetHashParam failed");
    }
  }

  cleanup_checksum_backend(handle);
  return handle->status;
}

void acquire_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

int acquire_verify_sync(struct acquire_handle *handle, const char *filepath,
                        enum Checksum algorithm, const char *expected_hash) {
  if (!handle)
    return -1;
  if (acquire_verify_async_start(handle, filepath, algorithm, expected_hash) !=
      0)
    return -1;
  while (acquire_verify_async_poll(handle) == ACQUIRE_IN_PROGRESS)
    ;
  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_USE_WINCRYPT) */
#endif /* !LIBACQUIRE_WINCRYPT_H */

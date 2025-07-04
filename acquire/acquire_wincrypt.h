#ifndef LIBACQUIRE_ACQUIRE_WINCRYPT_H
#define LIBACQUIRE_ACQUIRE_WINCRYPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "acquire_common_defs.h"

#if defined(__WIN32__)
#include <wincrypt.h>
#include <windows.h>
#endif

struct acquire_handle;

#if defined(LIBACQUIRE_USE_WINCRYPT)
int _wincrypt_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash);
enum acquire_status _wincrypt_verify_async_poll(struct acquire_handle *handle);
void _wincrypt_verify_async_cancel(struct acquire_handle *handle);
#endif

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_USE_WINCRYPT)
#ifndef ACQUIRE_WINCRYPT_IMPL_
#define ACQUIRE_WINCRYPT_IMPL_

#include "acquire_handle.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif

struct wincrypt_backend {
  HCRYPTPROV hProv;
  HCRYPTHASH hHash;
  FILE *file;
  char expected_hash[130];
};

static void cleanup_wincrypt_backend(struct acquire_handle *handle) {
  if (handle && handle->backend_handle) {
    struct wincrypt_backend *be =
        (struct wincrypt_backend *)handle->backend_handle;
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

int _wincrypt_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash) {
  struct wincrypt_backend *be;
  ALG_ID alg_id = 0;
  size_t expected_len = 0;
  switch (algorithm) {
  case LIBACQUIRE_SHA256:
    alg_id = CALG_SHA_256;
    expected_len = 64;
    break;
  case LIBACQUIRE_SHA512:
    alg_id = CALG_SHA_512;
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
  be = (struct wincrypt_backend *)calloc(1, sizeof(struct wincrypt_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY, "wincrypt");
    return -1;
  }
  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                             strerror(errno));
    free(be);
    return -1;
  }
  if (!CryptAcquireContext(&be->hProv, NULL, NULL, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT) ||
      !CryptCreateHash(be->hProv, alg_id, 0, 0, &be->hHash)) {
    cleanup_wincrypt_backend(handle);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "wincrypt init failed");
    return -1;
  }
  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status _wincrypt_verify_async_poll(struct acquire_handle *handle) {
  struct wincrypt_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;
  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED, "Cancelled");
    cleanup_wincrypt_backend(handle);
    return ACQUIRE_ERROR;
  }
  be = (struct wincrypt_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, sizeof(buffer), be->file);
  if (bytes_read > 0) {
    if (!CryptHashData(be->hHash, buffer, (DWORD)bytes_read, 0)) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "CryptHashData failed");
    } else {
      handle->bytes_processed += bytes_read;
      return ACQUIRE_IN_PROGRESS;
    }
  }
  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED,
                             strerror(errno));
  } else {
    BYTE hash[64];
    DWORD hash_len = sizeof(hash);
    char computed_hex[130];
    int i;
    if (CryptGetHashParam(be->hHash, HP_HASHVAL, hash, &hash_len, 0)) {
      for (i = 0; i < (int)hash_len; i++)
        sprintf(computed_hex + (i * 2), "%02x", hash[i]);
      computed_hex[hash_len * 2] = '\0';
      if (strncasecmp(computed_hex, be->expected_hash, hash_len * 2) == 0)
        handle->status = ACQUIRE_COMPLETE;
      else
        acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                                 "Hash mismatch: %s != %s", be->expected_hash,
                                 computed_hex);
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "CryptGetHashParam failed");
    }
  }
  cleanup_wincrypt_backend(handle);
  return handle->status;
}

void _wincrypt_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#endif /* ACQUIRE_WINCRYPT_IMPL_ */
#endif /* defined(LIBACQUIRE_IMPLEMENTATION) */

#ifdef __cplusplus
}
#endif

#endif /* !LIBACQUIRE_ACQUIRE_WINCRYPT_H */

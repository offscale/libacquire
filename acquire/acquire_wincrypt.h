#ifndef LIBACQUIRE_ACQUIRE_WINCRYPT_H
#define LIBACQUIRE_ACQUIRE_WINCRYPT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(LIBACQUIRE_USE_WINCRYPT) && LIBACQUIRE_USE_WINCRYPT

#include "acquire_common_defs.h"
#include "acquire_string_extras.h"
#include "acquire_windows.h"

#include <Guiddef.h>

#include <basetsd.h>

#include <wincrypt.h>

struct acquire_handle;

int _wincrypt_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash);
enum acquire_status _wincrypt_verify_async_poll(struct acquire_handle *handle);
void _wincrypt_verify_async_cancel(struct acquire_handle *handle);

#ifdef LIBACQUIRE_IMPLEMENTATION

#include "acquire_handle.h"
#include "acquire_string_extras.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif /* !CHUNK_SIZE */

struct wincrypt_backend {
  HCRYPTPROV hProv;
  HCRYPTHASH hHash;
  FILE *file;
  char expected_hash[130];
  ALG_ID alg_id;
};

static void reverse_bytes(BYTE *p, size_t len) {
  size_t i;
  for (i = 0; i < len / 2; ++i) {
    BYTE t = p[i];
    p[i] = p[len - 1 - i];
    p[len - 1 - i] = t;
  }
}

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
  {
    const errno_t err = fopen_s(&be->file, filepath, "rb");
    if (err != 0 || be->file == NULL) {
      fprintf(stderr, "couldn't open file for reading %s\n", filepath);
      acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                               "Cannot open file: %s", filepath);
      free(be);
      return -1;
    }
  }
  if (!CryptAcquireContext(&be->hProv, NULL, NULL, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT) ||
      !CryptCreateHash(be->hProv, alg_id, 0, 0, &be->hHash)) {
    cleanup_wincrypt_backend(handle);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "wincrypt init failed");
    return -1;
  }
  be->alg_id = alg_id;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    const errno_t e = strncpy_s(be->expected_hash, sizeof(be->expected_hash),
                                expected_hash, sizeof(be->expected_hash) - 1);
    if (e)
      be->expected_hash[0] = '\0';
  }
#else
  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
#endif
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
      handle->bytes_processed += (off_t)bytes_read;
      return ACQUIRE_IN_PROGRESS;
    }
  }
  if (ferror(be->file)) {
    char error_code[256];
    strerror_s(error_code, sizeof(error_code), errno);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED,
                             "File read error: %s", error_code);
  } else {
    BYTE hash[64];
    DWORD hash_len = sizeof(hash);
    char computed_hex[130];
    if (CryptGetHashParam(be->hHash, HP_HASHVAL, hash, &hash_len, 0)) {
      size_t BUF_SIZE = sizeof(computed_hex);
      DWORD j;

      if (be->alg_id == CALG_SHA_512) {
        /*reverse_bytes(hash, hash_len);*/
      }

      for (j = 0; j < hash_len; j++)
        sprintf_s(computed_hex + (j * 2), BUF_SIZE - (j * 2), "%02x", hash[j]);
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

#endif /* LIBACQUIRE_IMPLEMENTATION */

#endif /* defined(LIBACQUIRE_USE_WINCRYPT) && LIBACQUIRE_USE_WINCRYPT */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_WINCRYPT_H */

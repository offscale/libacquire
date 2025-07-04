#ifndef LIBACQUIRE_ACQUIRE_LIBRHASH_H
#define LIBACQUIRE_ACQUIRE_LIBRHASH_H

#ifdef LIBACQUIRE_USE_LIBRHASH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <rhash.h>

#include "acquire_common_defs.h"
#include "acquire_handle.h"
#include "libacquire_export.h"

struct rhash_backend {
  rhash handle;
  FILE *file;
  char expected_hash[130];
  unsigned int algorithm_id;
};

static int rhash_lib_initialized = 0;

extern LIBACQUIRE_EXPORT int
_librhash_verify_async_start(struct acquire_handle *handle,
                             const char *filepath, enum Checksum algorithm,
                             const char *expected_hash);
extern LIBACQUIRE_EXPORT enum acquire_status
_librhash_verify_async_poll(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT void
_librhash_verify_async_cancel(struct acquire_handle *handle);

#ifdef LIBACQUIRE_IMPLEMENTATION
#include "acquire_handle.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif /* !CHUNK_SIZE */

static void to_hex(char *dest, const unsigned char *const src,
                   const size_t len) {
  size_t i;
  for (i = 0; i < len; ++i)
    sprintf(dest + (i * 2), "%02x", src[i]);
}

void cleanup_rhash_backend(struct acquire_handle *handle) {
  if (handle && handle->backend_handle) {
    struct rhash_backend *be = (struct rhash_backend *)handle->backend_handle;
    if (be->handle)
      rhash_free(be->handle);
    if (be->file)
      fclose(be->file);
    free(be);
    handle->backend_handle = NULL;
  }
}

int _librhash_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash) {
  struct rhash_backend *be;
  unsigned int rhash_algo_id = 0;
  size_t expected_len = 0;
  switch (algorithm) {
  case LIBACQUIRE_CRC32C:
    rhash_algo_id = RHASH_CRC32C;
    expected_len = 8;
    break;
  case LIBACQUIRE_SHA256:
    rhash_algo_id = RHASH_SHA256;
    expected_len = 64;
    break;
  case LIBACQUIRE_SHA512:
    rhash_algo_id = RHASH_SHA512;
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
  if (!rhash_lib_initialized) {
    rhash_library_init();
    rhash_lib_initialized = 1;
  }
  be = (struct rhash_backend *)calloc(1, sizeof(struct rhash_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY, "rhash");
    return -1;
  }
  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED, "%s",
                             strerror(errno));
    free(be);
    return -1;
  }
  be->handle = rhash_init(rhash_algo_id);
  if (!be->handle) {
    cleanup_rhash_backend(handle);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "rhash_init");
    return -1;
  }
  be->algorithm_id = rhash_algo_id;
  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status _librhash_verify_async_poll(struct acquire_handle *handle) {
  struct rhash_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;
  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED, "Cancelled");
    cleanup_rhash_backend(handle);
    return ACQUIRE_ERROR;
  }
  be = (struct rhash_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, sizeof(buffer), be->file);
  if (bytes_read > 0) {
    if (rhash_update(be->handle, buffer, bytes_read) < 0) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "rhash_update");
    } else {
      handle->bytes_processed += bytes_read;
      return ACQUIRE_IN_PROGRESS;
    }
  }
  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED, "%s",
                             strerror(errno));
  } else {
    unsigned char hash[64];
    char computed_hex[130];
    int digest_size = 0;
    rhash_final(be->handle, hash);
    digest_size = rhash_get_digest_size(be->algorithm_id);
    /*rhash_print_hex(computed_hex, hash, digest_size, RHPR_DEFAULT);*/
    to_hex(computed_hex, hash, (size_t)digest_size);
    if (strncasecmp(computed_hex, be->expected_hash, digest_size * 2) == 0) {
      handle->status = ACQUIRE_COMPLETE;
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "Hash mismatch: %s != %s", be->expected_hash,
                               computed_hex);
    }
  }
  cleanup_rhash_backend(handle);
  return handle->status;
}

void _librhash_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}
#endif /* LIBACQUIRE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBACQUIRE_USE_LIBRHASH */

#endif /* !LIBACQUIRE_ACQUIRE_LIBRHASH_H */

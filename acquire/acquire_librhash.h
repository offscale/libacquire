#ifndef LIBACQUIRE_ACQUIRE_LIBRHASH_H
#define LIBACQUIRE_ACQUIRE_LIBRHASH_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes needed for declarations and struct definitions
#include "acquire_checksums.h"
#include <stdio.h>
#if defined(LIBACQUIRE_USE_LIBRHASH)
#include <rhash.h>
#endif

// This struct definition MUST be outside the implementation guard
struct rhash_backend {
#if defined(LIBACQUIRE_USE_LIBRHASH)
  rhash handle;
#else
  void *dummy;
#endif
  FILE *file;
  char expected_hash[130];
};

#if defined(LIBACQUIRE_USE_LIBRHASH)
int _librhash_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash);
enum acquire_status _librhash_verify_async_poll(struct acquire_handle *handle);
void _librhash_verify_async_cancel(struct acquire_handle *handle);
#endif

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_USE_LIBRHASH)

#include "acquire_handle.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4096
#endif

static int rhash_lib_initialized = 0;

static void cleanup_rhash_backend(struct acquire_handle *handle) {
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

  switch (algorithm) {
  case LIBACQUIRE_CRC32C:
    rhash_algo_id = RHASH_CRC32C;
    break;
  case LIBACQUIRE_SHA256:
    rhash_algo_id = RHASH_SHA256;
    break;
  case LIBACQUIRE_SHA512:
    rhash_algo_id = RHASH_SHA512;
    break;
  default:
    return -1;
  }

  if (!rhash_lib_initialized) {
    rhash_library_init();
    rhash_lib_initialized = 1;
  }

  be = (struct rhash_backend *)calloc(1, sizeof(struct rhash_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY, "rhash backend");
    return -1;
  }

  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED, "%s", strerror(errno));
    free(be);
    return -1;
  }

  be->handle = rhash_init(rhash_algo_id);
  if (!be->handle) {
    fclose(be->file);
    free(be);
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "rhash_init failed");
    return -1;
  }

  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  be->expected_hash[sizeof(be->expected_hash) - 1] = '\0';

  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status _librhash_verify_async_poll(struct acquire_handle *handle) {
  struct rhash_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;

  if (!handle || !handle->backend_handle) return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS) return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED, "Verification cancelled");
    cleanup_rhash_backend(handle);
    return ACQUIRE_ERROR;
  }

  be = (struct rhash_backend *)handle->backend_handle;
  bytes_read = fread(buffer, 1, sizeof(buffer), be->file);

  if (bytes_read > 0) {
    if (rhash_update(be->handle, buffer, bytes_read) < 0) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "rhash_update failed");
      handle->status = ACQUIRE_ERROR;
    } else {
      handle->bytes_processed += bytes_read;
      return ACQUIRE_IN_PROGRESS;
    }
  }

  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED, "%s", strerror(errno));
    handle->status = ACQUIRE_ERROR;
  } else {
    unsigned char hash[64]; /* Max SHA512 size */
    char computed_hex[130];
    int digest_size;
    rhash_final(be->handle, hash);
    digest_size = rhash_get_digest_size(rhash_get_algorithm(be->handle));
    rhash_print_hex(computed_hex, hash, digest_size, RHPR_LOWERCASE);

    if (strncasecmp(computed_hex, be->expected_hash, digest_size * 2) == 0) {
      handle->status = ACQUIRE_COMPLETE;
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN, "Hash mismatch. Expected %s, got %s", be->expected_hash, computed_hex);
      handle->status = ACQUIRE_ERROR;
    }
  }

  cleanup_rhash_backend(handle);
  return handle->status;
}

void _librhash_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_USE_LIBRHASH) */

#ifdef __cplusplus
}
#endif

#endif /* !LIBACQUIRE_ACQUIRE_LIBRHASH_H */

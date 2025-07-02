#ifndef LIBACQUIRE_ACQUIRE_LIBRHASH_H
#define LIBACQUIRE_ACQUIRE_LIBRHASH_H

/**
 * @file acquire_librhash.h
 * @brief Backend implementation for checksum verification using librhash.
 *
 * This file provides the functions to perform checksum verification (CRC32C,
 * SHA256, SHA512) using the RHash library. It supports both synchronous and
 * asynchronous operations.
 */

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_USE_LIBRHASH)

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rhash.h>

#include "acquire_checksums.h"
#include "acquire_handle.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHUNK_SIZE
/** @def CHUNK_SIZE The size of the buffer used for reading the file in chunks.
 */
#define CHUNK_SIZE 4096
#endif

/**
 * @struct rhash_backend
 * @brief Internal state for an asynchronous librhash operation.
 *
 * This structure holds all necessary state for a checksum operation, including
 * the file handle, the rhash context, the chosen algorithm, and the expected
 * hash value.
 */
struct rhash_backend {
  FILE *file;              /**< File pointer for the file being hashed. */
  rhash handle;            /**< The rhash context handle. */
  enum Checksum algorithm; /**< The checksum algorithm being used. */
  char expected_hash[260]; /**< The expected checksum as a hex string. */
};

/**
 * @brief Maps a libacquire Checksum enum to a librhash algorithm ID.
 * @param algorithm The libacquire checksum algorithm.
 * @return The corresponding librhash algorithm ID, or 0 if unsupported.
 */
static enum rhash_ids rhash_id_from_checksum(enum Checksum algorithm) {
  switch (algorithm) {
  case LIBACQUIRE_CRC32C:
    return RHASH_CRC32C;
  case LIBACQUIRE_SHA256:
    return RHASH_SHA256;
  case LIBACQUIRE_SHA512:
    return RHASH_SHA512;
  default:
    return 0;
  }
}

/**
 * @brief Cleans up all resources associated with the rhash backend.
 * @param handle The main acquire_handle.
 */
static void cleanup_rhash_backend(struct acquire_handle *handle) {
  if (handle && handle->backend_handle) {
    struct rhash_backend *be = (struct rhash_backend *)handle->backend_handle;
    if (be->file)
      fclose(be->file);
    if (be->handle)
      rhash_free(be->handle);
    free(be);
    handle->backend_handle = NULL;
  }
}

/**
 * @brief Starts an asynchronous checksum verification using librhash.
 *
 * This is the internal implementation that gets dispatched to. It initializes
 * the rhash context and prepares the handle for polling.
 *
 * @param handle The main acquire handle.
 * @param filepath Path to the file to verify.
 * @param algorithm The checksum algorithm to use.
 * @param expected_hash The expected checksum in hexadecimal format.
 * @return 0 on success, -1 on failure.
 */
int _librhash_verify_async_start(struct acquire_handle *handle,
                                 const char *filepath, enum Checksum algorithm,
                                 const char *expected_hash) {
  struct rhash_backend *be;
  enum rhash_ids rid;

  if (!handle || !filepath || !expected_hash) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                               "Invalid arguments for rhash");
    return -1;
  }

  rid = rhash_id_from_checksum(algorithm);
  if (rid == 0) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
                             "Unsupported algorithm for rhash backend");
    return -1;
  }

  rhash_library_init();

  be = (struct rhash_backend *)calloc(1, sizeof(struct rhash_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY,
                             "rhash backend");
    return -1;
  }

  be->file = fopen(filepath, "rb");
  if (!be->file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED, "%s",
                             strerror(errno));
    free(be);
    return -1;
  }

  be->handle = rhash_init(rid);
  if (!be->handle) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                             "rhash_init failed");
    fclose(be->file);
    free(be);
    return -1;
  }

  be->algorithm = algorithm;
  strncpy(be->expected_hash, expected_hash, sizeof(be->expected_hash) - 1);
  be->expected_hash[sizeof(be->expected_hash) - 1] = '\0';

  handle->backend_handle = be;
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

/**
 * @brief Polls the asynchronous checksum operation.
 *
 * Reads a chunk of the file, updates the hash, and reports progress. When
 * the file is fully read, it finalizes the hash and compares it to the
 * expected value.
 *
 * @param handle The main acquire handle.
 * @return The current status of the operation (IN_PROGRESS, COMPLETE, ERROR).
 */
enum acquire_status _librhash_verify_async_poll(struct acquire_handle *handle) {
  struct rhash_backend *be;
  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;

  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;

  be = (struct rhash_backend *)handle->backend_handle;

  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Checksum verification cancelled");
    cleanup_rhash_backend(handle);
    return ACQUIRE_ERROR;
  }

  bytes_read = fread(buffer, 1, sizeof(buffer), be->file);
  if (bytes_read > 0) {
    rhash_update(be->handle, buffer, bytes_read);
    handle->bytes_processed += bytes_read;
    return ACQUIRE_IN_PROGRESS;
  }

  if (ferror(be->file)) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_READ_FAILED, "%s",
                             strerror(errno));
    handle->status = ACQUIRE_ERROR;
  } else {
    unsigned char digest_bin[130];
    char digest_hex[260];
    rhash_final(be->handle, digest_bin);
    rhash_print_bytes(
        digest_hex, digest_bin,
        rhash_get_digest_size(rhash_id_from_checksum(be->algorithm)),
        (RHPR_HEX | RHPR_UPPERCASE));

    if (strncasecmp(digest_hex, be->expected_hash, strlen(be->expected_hash)) ==
        0) {
      handle->status = ACQUIRE_COMPLETE;
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "Checksum mismatch: expected %s, got %s",
                               be->expected_hash, digest_hex);
      handle->status = ACQUIRE_ERROR;
    }
  }

  cleanup_rhash_backend(handle);
  return handle->status;
}

/**
 * @brief Signals the asynchronous operation to cancel.
 * @param handle The main acquire handle.
 */
void _librhash_verify_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_USE_LIBRHASH) */

#endif /* !LIBACQUIRE_ACQUIRE_LIBRHASH_H */

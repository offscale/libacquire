/* acquire/acquire_download.h */
#ifndef LIBACQUIRE_ACQUIRE_DOWNLOAD_H
#define LIBACQUIRE_ACQUIRE_DOWNLOAD_H

/**
 * @file acquire_download.h
 * @brief Public API for synchronous and asynchronous file downloads.
 *
 * This module provides both synchronous (blocking) and asynchronous
 * (non-blocking) APIs for downloading files. Operations are managed
 * through a stateful `acquire_handle`.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_checksums.h"
#include "acquire_config.h"
#include "acquire_fileutils.h"
#include "acquire_url_utils.h"
#include "libacquire_export.h"

#include <stdio.h>     /* For FILE* */
#include <sys/types.h> /* For off_t */

/* --- Asynchronous Operation Status --- */

/**
 * @brief Describes the current state of an asynchronous operation.
 */
enum acquire_status {
  ACQUIRE_IDLE,            /* The handle is ready but no operation is active. */
  ACQUIRE_IN_PROGRESS,     /* The operation is running. */
  ACQUIRE_COMPLETE,        /* The operation finished successfully. */
  ACQUIRE_ERROR_CANCELLED, /* The operation was cancelled by the user. */
  ACQUIRE_ERROR            /* The operation failed. */
};

/* --- Download Operation Handle --- */

/**
 * @brief A handle managing the state of a single download operation.
 * The definition is public to avoid a typedef, per project constraints.
 * Fields marked "Internal use" should not be modified by the user.
 */
struct acquire_handle {
  /* --- Publicly readable state --- */
  off_t bytes_downloaded;
  off_t total_size;
  enum acquire_status status;
  char error_message[256];

  /* --- Internal use only --- */
  int cancel_flag;
  void *backend_handle; /* Opaque handle for backend-specific data. */
  FILE *output_file;
};

/* --- Handle Management --- */

/**
 * @brief Creates and initializes a new acquire handle.
 * @return A new handle, or NULL on failure. Caller must free with
 * acquire_handle_free.
 */
extern LIBACQUIRE_EXPORT struct acquire_handle *acquire_handle_init(void);

/**
 * @brief Frees all resources associated with a handle.
 * @param handle The handle to free.
 */
extern LIBACQUIRE_EXPORT void
acquire_handle_free(struct acquire_handle *handle);

/**
 * @brief Gets a human-readable error string for the last error on the handle.
 * @param handle The handle.
 * @return A constant string describing the last error.
 */
extern LIBACQUIRE_EXPORT const char *
acquire_handle_get_error(struct acquire_handle *handle);

/* --- Synchronous API --- */

/**
 * @brief Downloads a file synchronously (blocking).
 * @param handle A configured acquire handle.
 * @param url The URL to download.
 * @param dest_path The local path to save the file to.
 * @return 0 on success, non-zero on failure. Get details from the handle.
 */
extern LIBACQUIRE_EXPORT int
acquire_download_sync(struct acquire_handle *handle, const char *url,
                      const char *dest_path);

/* --- Asynchronous API --- */

/**
 * @brief Begins an asynchronous download (non-blocking).
 * @param handle A configured acquire handle.
 * @param url The URL to download.
 * @param dest_path The local path to save the file to.
 * @return 0 on success, non-zero on failure.
 */
extern LIBACQUIRE_EXPORT int
acquire_download_async_start(struct acquire_handle *handle, const char *url,
                             const char *dest_path);

/**
 * @brief Polls the status of an asynchronous operation, performing work.
 * @param handle The handle for the operation.
 * @return The current status of the operation (e.g., ACQUIRE_IN_PROGRESS).
 */
extern LIBACQUIRE_EXPORT enum acquire_status
acquire_download_async_poll(struct acquire_handle *handle);

/**
 * @brief Requests cancellation of an asynchronous operation.
 * @param handle The handle for the operation.
 */
extern LIBACQUIRE_EXPORT void
acquire_download_async_cancel(struct acquire_handle *handle);

/* --- Other Helpers --- */

extern LIBACQUIRE_EXPORT const char *get_download_dir(void);

extern LIBACQUIRE_EXPORT bool is_downloaded(const char *, enum Checksum,
                                            const char *, const char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_DOWNLOAD_H */

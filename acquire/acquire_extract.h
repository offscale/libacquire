/* acquire/acquire_extract.h */
#ifndef LIBACQUIRE_ACQUIRE_EXTRACT_H
#define LIBACQUIRE_ACQUIRE_EXTRACT_H

/**
 * @file acquire_extract.h
 * @brief Public API for synchronous and asynchronous archive extraction.
 *
 * This module provides a handle-based API for both blocking and non-blocking
 * archive extraction. The archive format is detected automatically by the
 * backend (e.g., libarchive).
 */

#include "acquire_handle.h"
#include "libacquire_export.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* --- Asynchronous API --- */

/**
 * @brief Begins an asynchronous extraction (non-blocking).
 *
 * This function initializes the extraction process. The actual work happens
 * during calls to `acquire_extract_async_poll`.
 *
 * @param handle An initialized acquire handle.
 * @param archive_path The path to the source archive file (e.g., .zip,
 * .tar.gz).
 * @param dest_path The directory to extract files into.
 * @return 0 on success, non-zero on failure.
 */
extern LIBACQUIRE_EXPORT int
acquire_extract_async_start(struct acquire_handle *handle,
                            const char *archive_path, const char *dest_path);

/**
 * @brief Polls the status of an asynchronous extraction, processing one entry.
 *
 * In a non-blocking application, this should be called repeatedly until it
 * no longer returns `ACQUIRE_IN_PROGRESS`. Each call processes one file or
 * directory entry from the archive.
 *
 * @param handle The handle for the in-progress operation.
 * @return The current status of the operation.
 */
extern LIBACQUIRE_EXPORT enum acquire_status
acquire_extract_async_poll(struct acquire_handle *handle);

/**
 * @brief Requests cancellation of an asynchronous extraction.
 *
 * The cancellation will be processed on the next call to
 * `acquire_extract_async_poll`.
 *
 * @param handle The handle for the operation to be cancelled.
 */
extern LIBACQUIRE_EXPORT void
acquire_extract_async_cancel(struct acquire_handle *handle);

/* --- Synchronous API --- */

/**
 * @brief Extracts an archive synchronously (blocking).
 *
 * This is a helper function that wraps the asynchronous API, calling `poll`
 * in a loop until the operation is complete.
 *
 * @param handle An initialized acquire handle.
 * @param archive_path The path to the source archive file.
 * @param dest_path The directory to extract files into.
 * @return 0 on success, non-zero on failure. Details can be retrieved from the
 * handle.
 */
extern LIBACQUIRE_EXPORT int acquire_extract_sync(struct acquire_handle *handle,
                                                  const char *archive_path,
                                                  const char *dest_path);

/* --- Deprecated Symbols --- */

/**
 * @deprecated This enum is no longer used in the public API and will be
 * removed in a future version. Archive format is now auto-detected.
 */
enum Archive {
  LIBACQUIRE_ZIP,
  LIBACQUIRE_INFER,
  LIBACQUIRE_UNSUPPORTED_ARCHIVE
};

/**
 * @deprecated This function is no longer needed for the public API and will be
 * removed in a future version.
 */
extern LIBACQUIRE_EXPORT enum Archive extension2archive(const char *extension);

/**
 * @deprecated This function is deprecated in favor of `acquire_extract_sync`.
 * It will be removed in a future version.
 */
extern LIBACQUIRE_EXPORT int extract_archive(enum Archive archive,
                                             const char *archive_filepath,
                                             const char *output_folder);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_EXTRACT_H */

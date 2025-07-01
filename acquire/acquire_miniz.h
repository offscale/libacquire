/* acquire/acquire_miniz.h */
#ifndef LIBACQUIRE_ACQUIRE_MINIZ_H
#define LIBACQUIRE_ACQUIRE_MINIZ_H

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_EXTRACT_IMPL)

#include <stdio.h>
#include <stdlib.h>

#include <kubazip/zip.h>

#include "acquire_extract.h"
#include "acquire_handle.h"

/* --- Helper Callbacks --- */

static int on_extract_entry(const char *filename, void *arg) {
  struct acquire_handle *handle = (struct acquire_handle *)arg;

  /* Update handle with current file being processed */
  strncpy(handle->current_file, filename, sizeof(handle->current_file) - 1);

  /* Cancellation check (best-effort) */
  return handle->cancel_flag ? -1 : 0;
}

/* --- Public API Implementation --- */

int acquire_extract_sync(struct acquire_handle *handle,
                         const char *archive_path, const char *dest_path) {
  if (!handle)
    return -1;
  /* Since this backend is synchronous, start and sync are the same. */
  return acquire_extract_async_start(handle, archive_path, dest_path);
}

int acquire_extract_async_start(struct acquire_handle *handle,
                                const char *archive_path,
                                const char *dest_path) {
  int result;
  if (!handle || !archive_path || !dest_path) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT, NULL);
    return -1;
  }

  handle->status = ACQUIRE_IN_PROGRESS;

  /* This call is blocking and does all the work. */
  result = zip_extract(archive_path, dest_path, on_extract_entry, handle);

  if (result == 0) {
    handle->status = ACQUIRE_COMPLETE;
  } else {
    if (handle->cancel_flag) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                               "Extraction cancelled");
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_ARCHIVE_EXTRACT_FAILED,
                               "miniz failed with code %d", result);
    }
  }

  /* For this backend, polling is not needed, so we return the final status. */
  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

enum acquire_status acquire_extract_async_poll(struct acquire_handle *handle) {
  /* No-op: all work is done in _start for this sync backend. */
  return handle ? handle->status : ACQUIRE_ERROR;
}

void acquire_extract_async_cancel(struct acquire_handle *handle) {
  if (handle) {
    handle->cancel_flag = 1;
  }
}

/* --- Deprecated API Implementation --- */

enum Archive extension2archive(const char *const extension) {
  if (strncasecmp(extension, ".zip", 4) == 0)
    return LIBACQUIRE_ZIP;
  return LIBACQUIRE_UNSUPPORTED_ARCHIVE;
}

int extract_archive(enum Archive archive, const char *archive_filepath,
                    const char *output_folder) {
  struct acquire_handle *handle = acquire_handle_init();
  int result;
  (void)archive; /* miniz only supports zip, which is auto-detected */
  if (!handle)
    return EXIT_FAILURE;
  result = acquire_extract_sync(handle, archive_filepath, output_folder);
  acquire_handle_free(handle);
  return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_EXTRACT_IMPL) */
#endif /* !LIBACQUIRE_ACQUIRE_MINIZ_H */

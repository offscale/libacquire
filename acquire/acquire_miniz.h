#ifndef LIBACQUIRE_ACQUIRE_MINIZ_H
#define LIBACQUIRE_ACQUIRE_MINIZ_H

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_EXTRACT_IMPL)

#include <stdio.h>
#include <stdlib.h>

#include <kubazip/zip.h>

#include "acquire_extract.h"
#include "acquire_fileutils.h"
#include "acquire_handle.h"

static int on_extract_entry(const char *filename, void *arg) {
  struct acquire_handle *handle = (struct acquire_handle *)arg;
  strncpy(handle->current_file, filename, sizeof(handle->current_file) - 1);
  return handle->cancel_flag ? -1 : 0;
}

int acquire_extract_sync(struct acquire_handle *handle,
                         const char *archive_path, const char *dest_path) {
  if (!handle)
    return -1;
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

  if (!is_file(archive_path)) {
    acquire_handle_set_error(
        handle, ACQUIRE_ERROR_ARCHIVE_OPEN_FAILED,
        "Archive file does not exist or is not a regular file: %s",
        archive_path);
    return -1;
  }

  handle->status = ACQUIRE_IN_PROGRESS;
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
  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

enum acquire_status acquire_extract_async_poll(struct acquire_handle *handle) {
  return handle ? handle->status : ACQUIRE_ERROR;
}

void acquire_extract_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_EXTRACT_IMPL) */
#endif /* !LIBACQUIRE_ACQUIRE_MINIZ_H */

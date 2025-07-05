#ifndef LIBACQUIRE_ACQUIRE_WINCOMPRESSAPI_H
#define LIBACQUIRE_ACQUIRE_WINCOMPRESSAPI_H

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_EXTRACT_IMPL)

#include <compressapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "acquire_extract.h"
#include "acquire_handle.h"

/*
 * NOTE: This is a synchronous wrapper around a complex API. True async would
 * require a significant state machine. This implementation processes the
 * entire archive in the _start call. It only supports uncompressed entries.
 */

#pragma pack(push, 1)
typedef struct _ZIP_LOCAL_FILE_HEADER {
  DWORD signature;
  WORD version_needed;
  WORD flags;
  WORD compression_method;
  WORD last_mod_time;
  WORD last_mod_date;
  DWORD crc32;
  DWORD compressed_size;
  DWORD uncompressed_size;
  WORD filename_length;
  WORD extra_field_length;
} ZIP_LOCAL_FILE_HEADER;
#pragma pack(pop)

/* --- Synchronous "Asynchronous" Implementation --- */
static int extract_all_entries(struct acquire_handle *handle,
                               const char *archive_path,
                               const char *dest_path) {
  acquire_handle_set_error(handle, ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
                           "WinCompressAPI backend is a placeholder and does "
                           "not support extraction.");
  return -1;
}

int acquire_extract_sync(struct acquire_handle *handle,
                         const char *archive_path, const char *dest_path) {
  return acquire_extract_async_start(handle, archive_path, dest_path);
}

int acquire_extract_async_start(struct acquire_handle *handle,
                                const char *archive_path,
                                const char *dest_path) {
  if (!handle || !archive_path || !dest_path) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT, NULL);
    return -1;
  }

  handle->status = ACQUIRE_IN_PROGRESS;

  if (extract_all_entries(handle, archive_path, dest_path) == 0) {
    handle->status = ACQUIRE_COMPLETE;
    return 0;
  }

  /* extract_all_entries sets the error on the handle */
  return -1;
}

enum acquire_status acquire_extract_async_poll(struct acquire_handle *handle) {
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
  (void)archive;
  struct acquire_handle *handle = acquire_handle_init();
  if (!handle)
    return EXIT_FAILURE;
  int result = acquire_extract_sync(handle, archive_filepath, output_folder);
  acquire_handle_free(handle);
  return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_EXTRACT_IMPL) */
#endif /* !LIBACQUIRE_ACQUIRE_WINCOMPRESSAPI_H */

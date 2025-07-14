#ifndef LIBACQUIRE_ACQUIRE_LIBARCHIVE_H
#define LIBACQUIRE_ACQUIRE_LIBARCHIVE_H

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_EXTRACT_IMPL)

#include <archive.h>
#include <archive_entry.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "acquire_extract.h"
#include "acquire_fileutils.h"
#include "acquire_handle.h"

struct libarchive_backend {
  struct archive *a;
  struct archive *ext;
  char dest_path[NAME_MAX + 1];
};

static int copy_entry_data(struct archive *ar, struct archive *aw) {
  int r;
  const void *buff;
  size_t size;
  off_t offset;
  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF)
      return (ARCHIVE_OK);
    if (r < ARCHIVE_OK)
      return (r);
    r = archive_write_data_block(aw, buff, size, offset);
    if (r < ARCHIVE_OK)
      return (r);
  }
}

static void cleanup_libarchive_backend(struct acquire_handle *handle) {
  struct libarchive_backend *be;
  if (!handle || !handle->backend_handle)
    return;
  be = (struct libarchive_backend *)handle->backend_handle;
  if (be->a) {
    archive_read_close(be->a);
    archive_read_free(be->a);
  }
  if (be->ext) {
    archive_write_close(be->ext);
    archive_write_free(be->ext);
  }
  free(be);
  handle->backend_handle = NULL;
}

int acquire_extract_sync(struct acquire_handle *handle,
                         const char *archive_path, const char *dest_path) {
  if (!handle)
    return -1;
  if (acquire_extract_async_start(handle, archive_path, dest_path) != 0)
    return -1;
  while (acquire_extract_async_poll(handle) == ACQUIRE_IN_PROGRESS)
    ;
  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

int acquire_extract_async_start(struct acquire_handle *handle,
                                const char *archive_path,
                                const char *dest_path) {
  struct libarchive_backend *be;
  int flags;

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

  be =
      (struct libarchive_backend *)calloc(1, sizeof(struct libarchive_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY,
                             "Failed to allocate backend state");
    return -1;
  }
  handle->backend_handle = be;
  strncpy(be->dest_path, dest_path, sizeof(be->dest_path) - 1);
  be->dest_path[sizeof(be->dest_path) - 1] = '\0';

  flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL |
          ARCHIVE_EXTRACT_FFLAGS;
  be->a = archive_read_new();
  be->ext = archive_write_disk_new();
  archive_read_support_format_all(be->a);
  archive_read_support_filter_all(be->a);
  archive_write_disk_set_options(be->ext, flags);
  archive_write_disk_set_standard_lookup(be->ext);

  if (archive_read_open_filename(be->a, archive_path, 10240) != ARCHIVE_OK) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_ARCHIVE_OPEN_FAILED, "%s",
                             archive_error_string(be->a));
    cleanup_libarchive_backend(handle);
    return -1;
  }
  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status acquire_extract_async_poll(struct acquire_handle *handle) {
  struct libarchive_backend *be;
  struct archive_entry *entry;
  int r;

  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Extraction cancelled by user");
    cleanup_libarchive_backend(handle);
    return handle->status;
  }
  be = (struct libarchive_backend *)handle->backend_handle;
  r = archive_read_next_header(be->a, &entry);
  if (r == ARCHIVE_EOF) {
    handle->status = ACQUIRE_COMPLETE;
    cleanup_libarchive_backend(handle);
    return ACQUIRE_COMPLETE;
  }
  if (r < ARCHIVE_OK) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_ARCHIVE_READ_HEADER_FAILED,
                             "%s", archive_error_string(be->a));
    cleanup_libarchive_backend(handle);
    return handle->status;
  }
  strncpy(handle->current_file, archive_entry_pathname(entry),
          sizeof(handle->current_file) - 1);
  handle->current_file[sizeof(handle->current_file) - 1] = '\0';
  handle->bytes_processed += archive_entry_size(entry);
  {
    char full_path[NAME_MAX * 2];
    snprintf(full_path, sizeof(full_path), "%s%s%s", be->dest_path, PATH_SEP,
             archive_entry_pathname(entry));
    archive_entry_set_pathname(entry, full_path);
  }
  r = archive_write_header(be->ext, entry);
  if (r < ARCHIVE_OK) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_ARCHIVE_EXTRACT_FAILED,
                             "Failed to write entry header: %s",
                             archive_error_string(be->ext));
    cleanup_libarchive_backend(handle);
    return handle->status;
  }
  if (archive_entry_size(entry) > 0) {
    if (copy_entry_data(be->a, be->ext) != ARCHIVE_OK) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_ARCHIVE_EXTRACT_FAILED,
                               "Failed to write entry data: %s",
                               archive_error_string(be->ext));
      cleanup_libarchive_backend(handle);
      return handle->status;
    }
  }
  r = archive_write_finish_entry(be->ext);
  if (r < ARCHIVE_OK) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_ARCHIVE_EXTRACT_FAILED,
                             "Failed to finalize entry: %s",
                             archive_error_string(be->ext));
    cleanup_libarchive_backend(handle);
    return handle->status;
  }
  return ACQUIRE_IN_PROGRESS;
}

void acquire_extract_async_cancel(struct acquire_handle *handle) {
  if (handle)
    handle->cancel_flag = 1;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_EXTRACT_IMPL) */
#endif /* !LIBACQUIRE_ACQUIRE_LIBARCHIVE_H */

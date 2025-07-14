#ifndef LIBACQUIRE_LIBFETCH_H
#define LIBACQUIRE_LIBFETCH_H

#if defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_LIBFETCH &&             \
    defined(LIBACQUIRE_IMPLEMENTATION)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "acquire_download.h"
#include "fetch.h"

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

/* --- Synchronous API --- */

/**
 * @brief Downloads a file synchronously (blocking) using libfetch.
 */
int acquire_download_sync(struct acquire_handle *handle, const char *url,
                          const char *dest_path) {
  struct url *u;
  FILE *f;
  char buffer[4096];
  size_t bytes_read;
  struct url_stat st;

  if (handle == NULL)
    return -1;

  u = fetchParseURL(url);
  if (u == NULL) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_URL_PARSE_FAILED,
                             fetchLastErrString);
    return -1;
  }

  /* Stat first to get the total size for progress reporting. */
  if (fetchStat(u, &st, "") == 0) {
    handle->total_size = st.size;
  }

  f = fetchGet(u, "");
  if (f == NULL) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_URL_PARSE_FAILED,
                             fetchLastErrString);
    fetchFreeURL(u);
    return -1;
  }

  handle->output_file = fopen(dest_path, "wb");
  if (!handle->output_file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                             "Failed to open destination file");
    fclose(f);
    fetchFreeURL(u);
    return -1;
  }

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
    if (handle->cancel_flag) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                               "Download cancelled");
      break;
    }
    fwrite(buffer, 1, bytes_read, handle->output_file);
    handle->bytes_processed += (off_t)bytes_read;
  }

  fclose(handle->output_file);
  handle->output_file = NULL;
  fclose(f);
  fetchFreeURL(u);

  if (handle->cancel_flag) {
    handle->status = ACQUIRE_ERROR_CANCELLED;
    return -1;
  }

  handle->status = ACQUIRE_COMPLETE;
  return 0;
}

/* --- Asynchronous API (Faked) --- */

/**
 * @brief Starts an async download by calling the blocking sync function.
 * TODO: Implement true async with non-blocking sockets and select/poll.
 */
int acquire_download_async_start(struct acquire_handle *handle, const char *url,
                                 const char *dest_path) {
  if (!handle || !url || !dest_path) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                               "Invalid arguments");
    return -1;
  }
  handle->status = ACQUIRE_IN_PROGRESS;
  /* This implementation is blocking, all work done here. */
  return acquire_download_sync(handle, url, dest_path);
}

/**
 * @brief Polls an async download. Since start is blocking, this just returns
 * the final status.
 */
enum acquire_status acquire_download_async_poll(struct acquire_handle *handle) {
  if (handle == NULL)
    return ACQUIRE_ERROR;
  return handle->status;
}

/**
 * @brief Requests cancellation.
 * TODO: True cancellation is impossible here as the sync function blocks.
 */
void acquire_download_async_cancel(struct acquire_handle *handle) {
  if (handle) {
    handle->cancel_flag = 1;
  }
}

#endif /* defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_LIBFETCH &&       \
          defined(LIBACQUIRE_IMPLEMENTATION) */

#endif /* !LIBACQUIRE_LIBFETCH_H */

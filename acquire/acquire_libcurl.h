/* acquire/acquire_libcurl.h */
#ifndef LIBACQUIRE_LIBCURL_H
#define LIBACQUIRE_LIBCURL_H

#if defined(LIBACQUIRE_USE_LIBCURL) && LIBACQUIRE_USE_LIBCURL &&               \
    defined(LIBACQUIRE_IMPLEMENTATION)

#include <curl/curl.h>
#include <errno.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h>
#else
#include <unistd.h> /* For usleep */
#endif

#include "acquire_download.h"

#if defined(LIBACQUIRE_DOWNLOAD_DIR_IMPL)
const char *get_download_dir(void) { return ".downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

/* --- Internal State --- */

/* Internal backend-specific state for libcurl. */
struct curl_backend {
  CURLM *multi_handle;
  CURL *easy_handle;
};

/* --- Common Handle Management (Repeated in each backend for simplicity) --- */

struct acquire_handle *acquire_handle_init(void) {
  struct acquire_handle *handle =
      (struct acquire_handle *)calloc(1, sizeof(struct acquire_handle));
  if (handle) {
    handle->total_size = -1; /* -1 means unknown size */
    handle->status = ACQUIRE_IDLE;
  }
  return handle;
}

void acquire_handle_free(struct acquire_handle *handle) {
  struct curl_backend *be;
  if (!handle)
    return;
  if (handle->backend_handle) {
    be = (struct curl_backend *)handle->backend_handle;
    if (be->easy_handle)
      curl_easy_cleanup(be->easy_handle);
    if (be->multi_handle)
      curl_multi_cleanup(be->multi_handle);
    free(be);
    curl_global_cleanup();
  }
  if (handle->output_file)
    fclose(handle->output_file);
  free(handle);
}

const char *acquire_handle_get_error(struct acquire_handle *handle) {
  return handle ? handle->error_message : "Invalid handle.";
}

/* --- Internal Callbacks --- */

/**
 * @brief libcurl callback to write received data to our output file.
 */
static size_t write_callback(void *ptr, size_t size, size_t nmemb,
                             void *userdata) {
  struct acquire_handle *handle = (struct acquire_handle *)userdata;
  if (handle->output_file) {
    return fwrite(ptr, size, nmemb, handle->output_file);
  }
  return 0;
}

/**
 * @brief libcurl callback to update progress information in the handle.
 */
static int progress_callback(void *clientp, curl_off_t dltotal,
                             curl_off_t dlnow, curl_off_t ultotal,
                             curl_off_t ulnow) {
  struct acquire_handle *handle = (struct acquire_handle *)clientp;
  (void)ultotal; /* Unused */
  (void)ulnow;   /* Unused */
  if (handle->cancel_flag)
    return 1; /* Non-zero return aborts transfer */
  if (dltotal > 0)
    handle->total_size = (off_t)dltotal;
  handle->bytes_downloaded = (off_t)dlnow;
  return 0;
}

/* --- Synchronous API --- */

/**
 * @brief Downloads a file synchronously (blocking) using libcurl.
 */
int acquire_download_sync(struct acquire_handle *handle, const char *url,
                          const char *dest_path) {
  int result;
  if (handle == NULL)
    return -1;
  result = acquire_download_async_start(handle, url, dest_path);
  if (result != 0)
    return result;
  while (acquire_download_async_poll(handle) == ACQUIRE_IN_PROGRESS) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(10);
#else
    usleep(10000);
#endif
  }
  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

/* --- Asynchronous API --- */

/**
 * @brief Begins an asynchronous download (non-blocking) using libcurl.
 */
int acquire_download_async_start(struct acquire_handle *handle, const char *url,
                                 const char *dest_path) {
  struct curl_backend *be;
  if (handle == NULL)
    return -1;

  be = (struct curl_backend *)calloc(1, sizeof(struct curl_backend));
  if (be == NULL) {
    strcpy(handle->error_message, "Out of memory");
    return -1;
  }
  handle->backend_handle = be;

  curl_global_init(CURL_GLOBAL_ALL);
  be->easy_handle = curl_easy_init();
  if (!be->easy_handle) {
    strcpy(handle->error_message, "curl_easy_init() failed");
    return -1;
  }

  handle->output_file = fopen(dest_path, "wb");
  if (!handle->output_file) {
    strcpy(handle->error_message, "Failed to open destination file");
    return -1;
  }

  curl_easy_setopt(be->easy_handle, CURLOPT_URL, url);
  curl_easy_setopt(be->easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(be->easy_handle, CURLOPT_WRITEDATA, handle);
  curl_easy_setopt(be->easy_handle, CURLOPT_XFERINFOFUNCTION,
                   progress_callback);
  curl_easy_setopt(be->easy_handle, CURLOPT_XFERINFODATA, handle);
  curl_easy_setopt(be->easy_handle, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(be->easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(be->easy_handle, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(be->easy_handle, CURLOPT_SSLVERSION,
                   CURL_SSLVERSION_TLSv1_2);

  be->multi_handle = curl_multi_init();
  curl_multi_add_handle(be->multi_handle, be->easy_handle);

  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

/**
 * @brief Polls the status of an async operation using curl_multi_perform.
 */
enum acquire_status acquire_download_async_poll(struct acquire_handle *handle) {
  struct curl_backend *be;
  int still_running = 0;
  CURLMsg *msg;
  if (handle == NULL || handle->backend_handle == NULL)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;

  be = (struct curl_backend *)handle->backend_handle;
  if (handle->cancel_flag) {
    handle->status = ACQUIRE_ERROR_CANCELLED;
    strcpy(handle->error_message, "Download cancelled by user");
    return handle->status;
  }

  curl_multi_perform(be->multi_handle, &still_running);
  if (still_running == 0) {
    int queued;
    msg = curl_multi_info_read(be->multi_handle, &queued);
    if (msg) {
      if (msg->msg == CURLMSG_DONE) {
        if (msg->data.result == CURLE_OK) {
          handle->status = ACQUIRE_COMPLETE;
        } else {
          handle->status = ACQUIRE_ERROR;
          strncpy(handle->error_message, curl_easy_strerror(msg->data.result),
                  sizeof(handle->error_message) - 1);
        }
      }
    } else {
      handle->status = ACQUIRE_ERROR;
      strcpy(handle->error_message, "Unknown multi handle state");
    }
    /* The transfer is done, close the file handle immediately to flush buffers.
     */
    if (handle->output_file) {
      fclose(handle->output_file);
      handle->output_file = NULL;
    }
  }
  return handle->status;
}

/**
 * @brief Requests cancellation of a libcurl transfer.
 */
void acquire_download_async_cancel(struct acquire_handle *handle) {
  if (handle) {
    handle->cancel_flag = 1;
  }
}

#endif /* defined(LIBACQUIRE_USE_LIBCURL) && LIBACQUIRE_USE_LIBCURL &&         \
defined(LIBACQUIRE_IMPLEMENTATION) */
#endif /* LIBACQUIRE_LIBCURL_H */

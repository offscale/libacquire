#ifndef LIBACQUIRE_LIBCURL_H
#define LIBACQUIRE_LIBCURL_H

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_DOWNLOAD_IMPL)

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
#include <unistd.h>
#endif

#include "acquire_download.h"
#include "acquire_handle.h"

#if defined(LIBACQUIRE_DOWNLOAD_DIR_IMPL)
const char *get_download_dir(void) { return ".downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

/* --- Internal State --- */
struct curl_backend {
  CURLM *multi_handle;
  CURL *easy_handle;
};

/* --- Internal Callbacks --- */
static size_t write_callback(void *ptr, size_t size, size_t nmemb,
                             void *userdata) {
  struct acquire_handle *handle = (struct acquire_handle *)userdata;
  return handle->output_file ? fwrite(ptr, size, nmemb, handle->output_file)
                             : 0;
}

static int progress_callback(void *clientp, curl_off_t dltotal,
                             curl_off_t dlnow, curl_off_t ultotal,
                             curl_off_t ulnow) {
  struct acquire_handle *handle = (struct acquire_handle *)clientp;
  (void)ultotal;
  (void)ulnow;
  if (handle->cancel_flag)
    return 1;
  if (dltotal > 0)
    handle->total_size = (off_t)dltotal;
  handle->bytes_processed = (off_t)dlnow;
  return 0;
}

/* --- API Implementation --- */
int acquire_download_sync(struct acquire_handle *handle, const char *url,
                          const char *dest_path) {
  if (!handle)
    return -1;
  if (acquire_download_async_start(handle, url, dest_path) != 0)
    return -1;
  while (acquire_download_async_poll(handle) == ACQUIRE_IN_PROGRESS)
    ;
  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

int acquire_download_async_start(struct acquire_handle *handle, const char *url,
                                 const char *dest_path) {
  struct curl_backend *be;
  if (!handle)
    return -1;

  be = (struct curl_backend *)calloc(1, sizeof(struct curl_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY,
                             "curl backend memory allocation failed");
    return -1;
  }
  handle->backend_handle = be;

  curl_global_init(CURL_GLOBAL_ALL);
  be->easy_handle = curl_easy_init();
  if (!be->easy_handle) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_NETWORK_INIT_FAILED,
                             "curl_easy_init() failed");
    return -1;
  }

  handle->output_file = fopen(dest_path, "wb");
  if (!handle->output_file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                             "Failed to open destination file: %s", dest_path);
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

enum acquire_status acquire_download_async_poll(struct acquire_handle *handle) {
  struct curl_backend *be;
  int still_running = 0;
  CURLMsg *msg;
  if (!handle || !handle->backend_handle)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;

  be = (struct curl_backend *)handle->backend_handle;
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Download cancelled by user");
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
          acquire_handle_set_error(handle, ACQUIRE_ERROR_NETWORK_FAILURE,
                                   "curl error: %s",
                                   curl_easy_strerror(msg->data.result));
        }
      }
    } else {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_UNKNOWN,
                               "Unknown curl multi handle state");
    }
    if (handle->output_file) {
      fclose(handle->output_file);
      handle->output_file = NULL;
    }
  }
  return handle->status;
}

void acquire_download_async_cancel(struct acquire_handle *handle) {
  if (handle) {
    handle->cancel_flag = 1;
  }
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_DOWNLOAD_IMPL) */
#endif /* !LIBACQUIRE_LIBCURL_H */

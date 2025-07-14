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

#include "acquire_config.h"
#include "acquire_download.h"
#include "acquire_handle.h"

/* --- Global cURL State Management --- */
static int g_acquire_curl_ref_count = 0;

static void acquire_curl_global_init(void) {
  if (g_acquire_curl_ref_count == 0) {
    curl_global_init(CURL_GLOBAL_ALL);
  }
  g_acquire_curl_ref_count++;
}

static void acquire_curl_global_cleanup(void) {
  g_acquire_curl_ref_count--;
  if (g_acquire_curl_ref_count == 0) {
    curl_global_cleanup();
  }
}
/* --- */

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

/* --- Internal State --- */
struct curl_backend {
  CURLM *multi_handle;
  CURL *easy_handle;
};

/* --- Internal Helpers --- */
static void cleanup_curl_backend(struct acquire_handle *handle) {
  if (!handle)
    return;
  if (handle->backend_handle) {
    struct curl_backend *be = (struct curl_backend *)handle->backend_handle;
    if (be->multi_handle && be->easy_handle) {
      curl_multi_remove_handle(be->multi_handle, be->easy_handle);
    }
    if (be->easy_handle) {
      curl_easy_cleanup(be->easy_handle);
    }
    if (be->multi_handle) {
      curl_multi_cleanup(be->multi_handle);
    }
    acquire_curl_global_cleanup(); /* Decrement ref count and maybe cleanup */
    free(be);
    handle->backend_handle = NULL;
  }
  if (handle->output_file) {
    fclose(handle->output_file);
    handle->output_file = NULL;
  }
}

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
  if (!handle || !url || !dest_path) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT, NULL);
    return -1;
  }
  if (acquire_download_async_start(handle, url, dest_path) != 0)
    return -1;
  while (acquire_download_async_poll(handle) == ACQUIRE_IN_PROGRESS)
    ;
  return (handle->status == ACQUIRE_COMPLETE) ? 0 : -1;
}

int acquire_download_async_start(struct acquire_handle *handle, const char *url,
                                 const char *dest_path) {
  struct curl_backend *be;
  if (!handle || !url || !dest_path) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                               "Invalid arguments");
    return -1;
  }

  be = (struct curl_backend *)calloc(1, sizeof(struct curl_backend));
  if (!be) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_OUT_OF_MEMORY,
                             "curl backend memory allocation failed");
    return -1;
  }

  acquire_curl_global_init();

  be->easy_handle = curl_easy_init();
  if (!be->easy_handle) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_NETWORK_INIT_FAILED,
                             "curl_easy_init() failed");
    free(be);
    acquire_curl_global_cleanup(); /* Cleanup on failure */
    return -1;
  }
  handle->backend_handle = be; /* Assign only after successful init */

  handle->output_file = fopen(dest_path, "wb");
  if (!handle->output_file) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                             "Failed to open destination file: %s", dest_path);
    cleanup_curl_backend(handle);
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
  curl_easy_setopt(be->easy_handle, CURLOPT_USERAGENT,
                   "libacquire/" LIBACQUIRE_VERSION);
  curl_easy_setopt(be->easy_handle, CURLOPT_SSLVERSION,
                   CURL_SSLVERSION_TLSv1_2);

  be->multi_handle = curl_multi_init();
  if (!be->multi_handle) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_NETWORK_INIT_FAILED,
                             "curl_multi_init() failed");
    cleanup_curl_backend(handle);
    return -1;
  }
  curl_multi_add_handle(be->multi_handle, be->easy_handle);

  handle->status = ACQUIRE_IN_PROGRESS;
  return 0;
}

enum acquire_status acquire_download_async_poll(struct acquire_handle *handle) {
  struct curl_backend *be;
  CURLMcode mc;
  int still_running = 0;

  /* 1. Basic sanity checks and state validation */
  if (handle == NULL)
    return ACQUIRE_ERROR;
  if (handle->status != ACQUIRE_IN_PROGRESS)
    return handle->status;
  if (handle->backend_handle == NULL) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                             "Polling on an uninitialized backend.");
    return ACQUIRE_ERROR;
  }

  be = (struct curl_backend *)handle->backend_handle;

  /* 2. Check for user cancellation first */
  if (handle->cancel_flag) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                             "Download cancelled by user.");
    cleanup_curl_backend(handle);
    return ACQUIRE_ERROR;
  }

  /* 3. Drive the multi stack to perform I/O */
  mc = curl_multi_perform(be->multi_handle, &still_running);
  if (mc != CURLM_OK) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_NETWORK_FAILURE,
                             "curl_multi_perform() failed: %s",
                             curl_multi_strerror(mc));
    cleanup_curl_backend(handle);
    return ACQUIRE_ERROR;
  }

  /* 4. Check for transfer completion messages */
  {
    CURLMsg *msg;
    int msgs_left;
    while ((msg = curl_multi_info_read(be->multi_handle, &msgs_left))) {
      if (msg->msg == CURLMSG_DONE) {
        /* This transfer is finished. Since we only manage one, the whole
         * operation is done. */
        if (msg->data.result == CURLE_OK) {
          handle->status = ACQUIRE_COMPLETE;
        } else {
          long response_code = 0;
          curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE,
                            &response_code);

          if (msg->data.result == CURLE_COULDNT_RESOLVE_HOST) {
            acquire_handle_set_error(handle, ACQUIRE_ERROR_HOST_NOT_FOUND,
                                     "Could not resolve host: %s",
                                     curl_easy_strerror(msg->data.result));
          } else if (response_code >= 400) {
            acquire_handle_set_error(handle, ACQUIRE_ERROR_HTTP_FAILURE,
                                     "HTTP error: %ld", response_code);
          } else {
            acquire_handle_set_error(handle, ACQUIRE_ERROR_NETWORK_FAILURE,
                                     "cURL error: %s",
                                     curl_easy_strerror(msg->data.result));
          }
        }
        break; /* Exit loop, we only care about our one transfer */
      }
    }
  }

  /* 5. If it's not running, the operation is over */
  if (still_running == 0) {
    if (handle->status == ACQUIRE_IN_PROGRESS) {
      /* If curl reports not running but we haven't received a DONE message,
       * it implies success. */
      handle->status = ACQUIRE_COMPLETE;
    }
    cleanup_curl_backend(handle);
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

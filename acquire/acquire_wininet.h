#ifndef LIBACQUIRE_WININET_H
#define LIBACQUIRE_WININET_H

#if defined(LIBACQUIRE_USE_WININET) && LIBACQUIRE_USE_WININET &&               \
    defined(LIBACQUIRE_IMPLEMENTATION)

#include <stdio.h>
#include <string.h>

#include "acquire_windows.h"

#include <wininet.h>

#include "acquire_download.h"

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

/* --- Synchronous API --- */

/**
 * @brief Downloads a file synchronously (blocking) using WinINet.
 * This is the main implementation for this backend.
 */
int acquire_download_sync(struct acquire_handle *handle, const char *url,
                          const char *dest_path) {
  HINTERNET h_internet, h_url;
  DWORD bytes_read, content_len_size = sizeof(handle->total_size),
                    dwStatusCode = 0, dwSize = sizeof(dwStatusCode);
  char buffer[4096];

  if (!handle || !url || !dest_path) {
    if (handle)
      acquire_handle_set_error(handle, ACQUIRE_ERROR_INVALID_ARGUMENT,
                               "Invalid arguments for sync download");
    return -1;
  }

  h_internet = InternetOpen("acquire_wininet", INTERNET_OPEN_TYPE_PRECONFIG,
                            NULL, NULL, 0);
  if (h_internet == NULL) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_NETWORK_INIT_FAILED,
                             "InternetOpen failed");
    return -1;
  }

  h_url = InternetOpenUrl(h_internet, url, NULL, 0,
                          INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
  if (h_url == NULL) {
    acquire_handle_set_error(handle, ACQUIRE_ERROR_HOST_NOT_FOUND,
                             "InternetOpenUrl failed");
    InternetCloseHandle(h_internet);
    return -1;
  }

  if (HttpQueryInfo(h_url, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                    &dwStatusCode, &dwSize, NULL)) {
    if (dwStatusCode >= 400) {
      acquire_handle_set_error(handle, ACQUIRE_ERROR_HTTP_FAILURE,
                               "HTTP error: %lu", dwStatusCode);
      goto fail;
    }
  }

  {
    const errno_t err = fopen_s(&handle->output_file, dest_path, "wb");
    if (err != 0 || handle->output_file == NULL) {
      fprintf(stderr, "couldn't open file for reading %s\n", dest_path);
      acquire_handle_set_error(handle, ACQUIRE_ERROR_FILE_OPEN_FAILED,
                               "Failed to open destination file: %s",
                               dest_path);
      goto fail;
    }
  }

  /* Query file size for progress reporting */
  HttpQueryInfo(h_url, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                (LPVOID)&handle->total_size, &content_len_size, NULL);

  while (InternetReadFile(h_url, buffer, sizeof(buffer), &bytes_read) &&
         bytes_read > 0) {
    if (handle->cancel_flag) { /* Check for cancellation */
      acquire_handle_set_error(handle, ACQUIRE_ERROR_CANCELLED,
                               "Download cancelled");
      goto fail;
    }
    fwrite(buffer, 1, bytes_read, handle->output_file);
    handle->bytes_processed += (off_t)bytes_read;
  }

  fclose(handle->output_file);
  handle->output_file = NULL;
  InternetCloseHandle(h_url);
  InternetCloseHandle(h_internet);
  handle->status = ACQUIRE_COMPLETE;
  return 0;

fail:
  if (handle->output_file)
    fclose(handle->output_file);
  handle->output_file = NULL;
  InternetCloseHandle(h_url);
  InternetCloseHandle(h_internet);
  return -1;
}

/* --- Asynchronous API (Faked) --- */

/**
 * @brief Starts an async download by calling the blocking sync function.
 * TODO: Implement true async WinINet with INTERNET_FLAG_ASYNC and callbacks.
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
  /* This implementation is blocking, so all work completes here. */
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

#endif /* defined(LIBACQUIRE_USE_WININET) && LIBACQUIRE_USE_WININET &&         \
          defined(LIBACQUIRE_IMPLEMENTATION) */

#endif /* !LIBACQUIRE_WININET_H */

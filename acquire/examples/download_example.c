#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h>
#else
#include <unistd.h>
#endif

/* Include the main library header */
#include <acquire_download.h>

/**
 * @brief Demonstrates the simple, blocking synchronous API.
 */
void run_synchronous_download(void) {
  struct acquire_handle *handle;
  int result;

  printf("--- Testing Synchronous Download ---\n");
  handle = acquire_handle_init();
  if (handle == NULL) {
    fprintf(stderr, "Failed to create acquire handle!\n");
    return;
  }

  printf("Starting blocking download of 'https://httpbin.org/get'...\n");
  result = acquire_download_sync(handle, "https://httpbin.org/get",
                                 "./sync_download.json");

  if (result == 0) {
    printf("Sync download complete! Saved to 'sync_download.json'.\n");
  } else {
    fprintf(stderr, "Sync download failed: %s\n",
            acquire_handle_get_error(handle));
  }

  acquire_handle_free(handle);
  printf("\n");
}

/**
 * @brief Demonstrates the non-blocking asynchronous API with progress
 * reporting.
 */
void run_asynchronous_download(void) {
  struct acquire_handle *handle;
  enum acquire_status status;

  printf("--- Testing Asynchronous Download ---\n");
  handle = acquire_handle_init();
  if (handle == NULL) {
    fprintf(stderr, "Failed to create acquire handle!\n");
    return;
  }

  /*
   * We'll download a 2MB test file from a reliable source.
   * This is large enough to see progress but small enough to be quick.
   */
  printf("Starting async download of a 2MB test file...\n");
  if (acquire_download_async_start(
          handle, "http://ipv4.download.thinkbroadband.com/2MB.zip",
          "./async_download.zip") != 0) {
    fprintf(stderr, "Failed to start async download: %s\n",
            acquire_handle_get_error(handle));
    acquire_handle_free(handle);
    return;
  }

  printf("Async download in progress. Polling for updates...\n");
  do {
    off_t current, total;

    /* Poll the handle to perform a chunk of work */
    status = acquire_download_async_poll(handle);

    /* Query progress from the handle */
    current = handle->bytes_downloaded;
    total = handle->total_size;

    if (total > 0) {
      printf("\rProgress: %ld / %ld bytes (%.0f%%)", (long)current, (long)total,
             100.0 * (double)current / (double)total);
    } else {
      printf("\rProgress: %ld bytes downloaded (total size unknown)",
             (long)current);
    }
    fflush(stdout);

    /* In a real app, this is where you'd yield to your event loop.
       Here, we just sleep to avoid a busy-wait. */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(100); /* 100ms */
#else
    usleep(100000); /* 100ms */
#endif

  } while (status == ACQUIRE_IN_PROGRESS);

  printf("\n"); /* Newline after progress bar finishes */

  switch (status) {
  case ACQUIRE_COMPLETE:
    printf("Async download complete! Saved to 'async_download.zip'.\n");
    break;
  case ACQUIRE_ERROR_CANCELLED:
    printf("Async download was cancelled.\n");
    break;
  case ACQUIRE_ERROR:
    fprintf(stderr, "Async download failed: %s\n",
            acquire_handle_get_error(handle));
    break;
  default:
    fprintf(stderr, "Async download ended in an unexpected state.\n");
    break;
  }

  acquire_handle_free(handle);
  printf("\n");
}

int main(void) {
  run_synchronous_download();
  run_asynchronous_download();
  return 0;
}

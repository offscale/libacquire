#ifndef TEST_CURL_BACKEND_H
#define TEST_CURL_BACKEND_H

#include <greatest.h>
#include <stdio.h>
#include <string.h>

#include "acquire_config.h"
#include "acquire_download.h"
#include "config_for_tests.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h> /* For Sleep() */
#else
#include <unistd.h> /* For usleep() */
#endif

/*
 * This test suite focuses on behaviors specific to the libcurl backend.
 * It is only compiled and run when LIBACQUIRE_USE_LIBCURL is enabled.
 */

/**
 * @brief Helper function to poll a handle to completion.
 */
static enum acquire_status poll_to_completion(struct acquire_handle *handle) {
  enum acquire_status status;
  do {
    status = acquire_download_async_poll(handle);
    if (status == ACQUIRE_IN_PROGRESS) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
      Sleep(20);
#else
      usleep(20000);
#endif
    }
  } while (status == ACQUIRE_IN_PROGRESS);
  return status;
}

/**
 * @brief Test that a download from a non-existent host fails correctly.
 */
TEST test_curl_download_fails_on_bad_host(void) {
  struct acquire_handle *handle = acquire_handle_init();
  char local_path[] = DOWNLOAD_DIR PATH_SEP "curl_bad_host.tmp";
  const char *error_msg;

  ASSERT(handle != NULL);

  ASSERT_EQ_FMT(0,
                acquire_download_async_start(
                    handle, "https://zipppppppppppppppp.com", local_path),
                "%d");

  ASSERT_EQ_FMT(ACQUIRE_ERROR, poll_to_completion(handle), "%d");
  error_msg = acquire_handle_get_error(handle);
  ASSERT(error_msg != NULL);
  ASSERT(strlen(error_msg) > 0);
  printf("  -> Received expected error: %s\n", error_msg);

  acquire_handle_free(handle);
  PASS();
}

/**
 * @brief Test that progress information is correctly updated.
 */
TEST test_curl_progress_reporting(void) {
  struct acquire_handle *handle = acquire_handle_init();
  char local_path[] = DOWNLOAD_DIR PATH_SEP "curl_progress_test.zip";
  off_t last_bytes_downloaded = 0;
  int progress_seen = 0;

  ASSERT(handle != NULL);

  ASSERT_EQ(0,
            acquire_download_async_start(handle, GREATEST_ZIP_URL, local_path));

  while (acquire_download_async_poll(handle) == ACQUIRE_IN_PROGRESS) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(50);
#else
    usleep(50000);
#endif
    if (handle->bytes_downloaded > last_bytes_downloaded) {
      progress_seen = 1;
    }
    ASSERT(handle->bytes_downloaded >= last_bytes_downloaded);
    last_bytes_downloaded = handle->bytes_downloaded;
  }

  ASSERTm("Download did not make any progress", progress_seen == 1);
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, handle->status, "%d");

  ASSERT_EQ(handle->total_size, 58545L);
  ASSERT_EQ_FMT(handle->total_size, handle->bytes_downloaded, "%ld");

  acquire_handle_free(handle);
  PASS();
}

/**
 * @brief Test a successful HTTPS download that involves a redirect.
 */
TEST test_curl_https_and_redirect_success(void) {
  struct acquire_handle *handle = acquire_handle_init();
  char local_path[] = DOWNLOAD_DIR PATH_SEP "greatest_curl.h";

  ASSERT(handle != NULL);

  acquire_download_sync(handle, GREATEST_URL, local_path);

  ASSERTm("Download failed", handle->status == ACQUIRE_COMPLETE);
  ASSERT(is_file(local_path));
  ASSERT(sha256(local_path, GREATEST_SHA256));

  acquire_handle_free(handle);
  PASS();
}

SUITE(curl_backend_suite) {
  RUN_TEST(test_curl_https_and_redirect_success);
  RUN_TEST(test_curl_download_fails_on_bad_host);
  RUN_TEST(test_curl_progress_reporting);
}

#endif /* !TEST_CURL_BACKEND_H */

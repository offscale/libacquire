#ifndef TEST_CURL_BACKEND_H
#define TEST_CURL_BACKEND_H

#include <greatest.h>
#include <stdio.h>
#include <string.h>

#include "acquire_checksums.h"
#include "acquire_common_defs.h"
#include "acquire_config.h"
#include "acquire_download.h"
#include "acquire_fileutils.h"
#include "config_for_tests.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h> /* For Sleep() */
#else
#include <unistd.h> /* For usleep() */
#endif

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

TEST test_curl_download_fails_on_bad_host(void) {
  struct acquire_handle *handle = acquire_handle_init();
  ASSERT(handle);
  ASSERT_EQ(0,
            acquire_download_async_start(
                handle, "https://nonexistent-host-for-testing.com", "bad.tmp"));
  ASSERT_EQ_FMT(ACQUIRE_ERROR, poll_to_completion(handle), "%d");
  acquire_handle_free(handle);
  PASS();
}

TEST test_curl_progress_reporting(void) {
  struct acquire_handle *handle = acquire_handle_init();
  char local_path[] = DOWNLOAD_DIR PATH_SEP "curl_progress_test.zip";
  ASSERT(handle);
  ASSERT_EQ(0,
            acquire_download_async_start(handle, GREATEST_ZIP_URL, local_path));
  while (acquire_download_async_poll(handle) == ACQUIRE_IN_PROGRESS)
    ;
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, handle->status, "%d");
  /*
   * The server may use chunked encoding, in which case the Content-Length
   * header is not sent and handle->total_size will remain -1.
   * Instead, we verify handle->bytes_processed, which is always updated
   * and should equal the final file size.
   */
  ASSERT_EQ_FMT(58545L, (long)handle->bytes_processed, "%ld");
  /* As a sanity check, also verify the size of the file on disk. */
  ASSERT_EQ_FMT(58545L, (long)filesize(local_path), "%ld");
  acquire_handle_free(handle);
  PASS();
}

TEST test_curl_https_and_redirect_success(void) {
  struct acquire_handle *dl_handle = acquire_handle_init();
  struct acquire_handle *verify_handle = acquire_handle_init();
  char local_path[] = DOWNLOAD_DIR PATH_SEP "greatest_curl.h";
  int result;
  ASSERT(dl_handle && verify_handle);

  result = acquire_download_sync(dl_handle, GREATEST_URL, local_path);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT(is_file(local_path));
  result = acquire_verify_sync(verify_handle, local_path, LIBACQUIRE_SHA256,
                               GREATEST_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");

  acquire_handle_free(dl_handle);
  acquire_handle_free(verify_handle);
  PASS();
}

SUITE(curl_backend_suite) {
  RUN_TEST(test_curl_https_and_redirect_success);
  RUN_TEST(test_curl_download_fails_on_bad_host);
  RUN_TEST(test_curl_progress_reporting);
}

#endif /* !TEST_CURL_BACKEND_H */

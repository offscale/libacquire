/* acquire/tests/test_download.h */
#ifndef TEST_DOWNLOAD_H
#define TEST_DOWNLOAD_H

#include <greatest.h>
#include <string.h> /* For memset */

#include "acquire_common_defs.h"
#include "acquire_config.h"
#include "config_for_tests.h"

/*
 * This test suite verifies the public download API.
 * It is implementation-agnostic and should pass with any backend
 * (libcurl, wininet, libfetch).
 */

/**
 * @brief Tests the synchronous download API.
 * This is a blocking call and is the simplest way to use the library.
 */
TEST test_sync_download(void) {
  struct acquire_handle *handle = acquire_handle_init();
  char local_path[NAME_MAX + 1];
  int result;

  ASSERT(handle != NULL);

  snprintf(local_path, sizeof(local_path), "%s%sgreatest_sync.h", DOWNLOAD_DIR,
           PATH_SEP);

  /* Perform the synchronous download */
  result = acquire_download_sync(handle, GREATEST_URL, local_path);

  /* Check for success */
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, handle->status, "%d");

  /* Verify the downloaded file */
  ASSERT(is_file(local_path));
  ASSERT(sha256(local_path, GREATEST_SHA256));

  acquire_handle_free(handle);
  PASS();
}

/**
 * @brief Tests the asynchronous download API.
 * This test polls the handle until the operation is complete.
 */
TEST test_async_download(void) {
  struct acquire_handle *handle = acquire_handle_init();
  char local_path[NAME_MAX + 1];
  int result;

  ASSERT(handle != NULL);

  snprintf(local_path, sizeof(local_path), "%s%sgreatest_async.h", DOWNLOAD_DIR,
           PATH_SEP);

  /* Start the asynchronous download */
  result = acquire_download_async_start(handle, GREATEST_URL, local_path);
  ASSERT_EQ_FMT(0, result, "%d");

  /* Poll until the download is no longer in progress */
  while (acquire_download_async_poll(handle) == ACQUIRE_IN_PROGRESS) {
    /* In a real application, we would yield here. For this test, we
       can just loop. A small sleep avoids a tight loop on true async backends.
     */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(10);
#else
    usleep(10000);
#endif
  }

  /* Check for successful completion */
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, handle->status, "%d");

  /* Verify the downloaded file */
  ASSERT(is_file(local_path));
  ASSERT(sha256(local_path, GREATEST_SHA256));

  acquire_handle_free(handle);
  PASS();
}

/**
 * @brief Tests the asynchronous cancellation logic.
 */
TEST test_async_cancellation(void) {
  struct acquire_handle *handle = acquire_handle_init();
  char local_path[NAME_MAX + 1];
  int i;
  enum acquire_status status;

  ASSERT(handle != NULL);

  snprintf(local_path, sizeof(local_path), "%s%sgreatest_cancelled.h",
           DOWNLOAD_DIR, PATH_SEP);

  /* Start the async download */
  ASSERT_EQ(0, acquire_download_async_start(handle, GREATEST_URL, local_path));

  /* Poll a few times to let it start, then cancel */
  for (i = 0; i < 5; ++i) {
    status = acquire_download_async_poll(handle);
    if (status != ACQUIRE_IN_PROGRESS)
      break;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(10);
#else
    usleep(10000);
#endif
  }

  /* Request cancellation */
  acquire_download_async_cancel(handle);

  /* Continue polling until the operation terminates */
  while ((status = acquire_download_async_poll(handle)) ==
         ACQUIRE_IN_PROGRESS) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(10);
#else
    usleep(10000);
#endif
  }

  /* Assert that it terminated with the CANCELLED status */
  ASSERT_EQ_FMT(ACQUIRE_ERROR_CANCELLED, status, "%d");

  /* The file may or may not exist, and if it exists, it should be incomplete.
   */
  /* We don't assert on its state here, just that the cancellation was
   * acknowledged. */

  acquire_handle_free(handle);
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(downloads_suite) {
  RUN_TEST(test_sync_download);
  RUN_TEST(test_async_download);
  RUN_TEST(test_async_cancellation);
}

#endif /* !TEST_DOWNLOAD_H */

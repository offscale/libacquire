#ifndef TEST_DOWNLOAD_H
#define TEST_DOWNLOAD_H

#include <string.h>

#include <greatest.h>

#include "acquire_checksums.h"
#include "acquire_common_defs.h"
#include "acquire_download.h"
#include "config_for_tests.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <unistd.h>
#endif

TEST test_sync_download(void) {
  struct acquire_handle *dl_handle = acquire_handle_init();
  struct acquire_handle *verify_handle = acquire_handle_init();
  const char local_path[] = DOWNLOAD_DIR PATH_SEP "greatest_sync.h";
  int result;
  ASSERT(dl_handle != NULL && verify_handle != NULL);

  result = acquire_download_sync(dl_handle, GREATEST_URL, local_path);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, dl_handle->status, "%d");

  ASSERT(is_file(local_path));
  result = acquire_verify_sync(verify_handle, local_path, LIBACQUIRE_SHA256,
                               GREATEST_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");

  acquire_handle_free(dl_handle);
  acquire_handle_free(verify_handle);
  PASS();
}

TEST test_async_download(void) {
  struct acquire_handle *dl_handle = acquire_handle_init();
  struct acquire_handle *verify_handle = acquire_handle_init();
  const char local_path[] = DOWNLOAD_DIR PATH_SEP "greatest_async.h";
  int result;
  ASSERT(dl_handle != NULL && verify_handle != NULL);

  result = acquire_download_async_start(dl_handle, GREATEST_URL, local_path);
  ASSERT_EQ_FMT(0, result, "%d");

  while (acquire_download_async_poll(dl_handle) == ACQUIRE_IN_PROGRESS) {
#if defined(WIN32)
    Sleep(10);
#else
    usleep(10000);
#endif
  }

  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, dl_handle->status, "%d");
  ASSERT(is_file(local_path));

  result = acquire_verify_sync(verify_handle, local_path, LIBACQUIRE_SHA256,
                               GREATEST_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");

  acquire_handle_free(dl_handle);
  acquire_handle_free(verify_handle);
  PASS();
}

TEST test_async_cancellation(void) {
  struct acquire_handle *handle = acquire_handle_init();
  const char local_path[] = DOWNLOAD_DIR PATH_SEP "greatest_cancelled.h";
  enum acquire_status status;

  ASSERT(handle != NULL);
  ASSERT_EQ(0, acquire_download_async_start(handle, GREATEST_URL, local_path));

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  Sleep(20);
#else
  usleep(20000);
#endif
  acquire_download_async_cancel(handle);

  while ((status = acquire_download_async_poll(handle)) == ACQUIRE_IN_PROGRESS)
    ;

  ASSERT_EQ_FMT(ACQUIRE_ERROR, status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_CANCELLED, acquire_handle_get_error_code(handle),
                "%d");

  acquire_handle_free(handle);
  PASS();
}

TEST test_download_404_error(void) {
  struct acquire_handle *handle = acquire_handle_init();
  const char *url = "https://httpbin.org/status/404";
  const char *dest = DOWNLOAD_DIR PATH_SEP "404.tmp";

  ASSERT(handle != NULL);
  acquire_download_sync(handle, url, dest);

  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_HTTP_FAILURE,
                acquire_handle_get_error_code(handle), "%d");

  acquire_handle_free(handle);
  PASS();
}

TEST test_download_to_invalid_path(void) {
  struct acquire_handle *handle = acquire_handle_init();
  const char *url = GREATEST_URL;
  const char *dest = "non/existent/dir/file.h";

  ASSERT(handle != NULL);
  acquire_download_sync(handle, url, dest);

  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_FILE_OPEN_FAILED,
                acquire_handle_get_error_code(handle), "%d");

  acquire_handle_free(handle);
  PASS();
}

SUITE(downloads_suite) {
  RUN_TEST(test_sync_download);
  RUN_TEST(test_async_download);
  RUN_TEST(test_async_cancellation);
  RUN_TEST(test_download_404_error);
  RUN_TEST(test_download_to_invalid_path);
}

#endif /* !TEST_DOWNLOAD_H */

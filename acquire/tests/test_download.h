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
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
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

TEST test_async_download_invalid_args(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  result = acquire_download_async_start(NULL, GREATEST_URL, "file.tmp");
  ASSERT_EQ(-1, result);
  result = acquire_download_async_start(h, NULL, "file.tmp");
  ASSERT_EQ(-1, result);
  result = acquire_download_async_start(h, GREATEST_URL, NULL);
  ASSERT_EQ(-1, result);
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));
  acquire_handle_free(h);
  PASS();
}

TEST test_async_cancellation(void) {
  struct acquire_handle *handle = acquire_handle_init();
  const char local_path[] = DOWNLOAD_DIR PATH_SEP "5MB_cancelled.zip";
  enum acquire_status status;

  ASSERT(handle != NULL);
  if (acquire_download_async_start(handle, LARGE_FILE_URL, local_path) != 0) {
    char fail_msg[512];
    snprintf(fail_msg, sizeof(fail_msg),
             "Failed to start download for cancellation test. Error: [%d] %s",
             acquire_handle_get_error_code(handle),
             acquire_handle_get_error_string(handle));
    if (acquire_handle_get_error_code(handle) == ACQUIRE_ERROR_HOST_NOT_FOUND) {
      SKIPm(fail_msg);
    }
    FAILm(fail_msg);
  }

  /* Let it download for a bit to ensure it's in progress */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  Sleep(200); /* 200ms */
#else
  usleep(200000); /* 200ms */
#endif

  status = acquire_download_async_poll(handle);
  if (status == ACQUIRE_IN_PROGRESS) {
    acquire_download_async_cancel(handle);
  }

  while ((status = acquire_download_async_poll(handle)) == ACQUIRE_IN_PROGRESS)
    ;

  ASSERT_EQ_FMT(ACQUIRE_ERROR, status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_CANCELLED, acquire_handle_get_error_code(handle),
                "%d");

  acquire_handle_free(handle);
  remove(local_path);
  PASS();
}

TEST test_download_404_error(void) {
  struct acquire_handle *handle = acquire_handle_init();
  const char *const url = "https://httpbin.org/status/404";
  const char *const dest = DOWNLOAD_DIR PATH_SEP "404.tmp";

  ASSERT(handle != NULL);
  acquire_download_sync(handle, url, dest);

  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_HTTP_FAILURE,
                acquire_handle_get_error_code(handle), "%d");

  acquire_handle_free(handle);
  PASS();
}

TEST test_download_bad_host(void) {
  struct acquire_handle *handle = acquire_handle_init();
  const char *const url = "http://this-is-not-a-real-domain.invalid/";
  const char *const dest = DOWNLOAD_DIR PATH_SEP "bad_host.tmp";

  ASSERT(handle != NULL);
  acquire_download_sync(handle, url, dest);

  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_HOST_NOT_FOUND,
                acquire_handle_get_error_code(handle), "%d");

  acquire_handle_free(handle);
  PASS();
}

TEST test_download_to_invalid_path(void) {
  struct acquire_handle *handle = acquire_handle_init();
  const char *const url = GREATEST_URL;
  const char *const dest =
      "non" PATH_SEP "existent" PATH_SEP "dir" PATH_SEP "file.h";

  ASSERT(handle != NULL);
  acquire_download_sync(handle, url, dest);

  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_FILE_OPEN_FAILED,
                acquire_handle_get_error_code(handle), "%d");

  acquire_handle_free(handle);
  PASS();
}

TEST test_sync_download_invalid_args(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;

  result = acquire_download_sync(NULL, GREATEST_URL, "file.tmp");
  ASSERT_EQ(-1, result);

  result = acquire_download_sync(h, NULL, "file.tmp");
  ASSERT_EQ(-1, result);
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));

  result = acquire_download_sync(h, GREATEST_URL, NULL);
  ASSERT_EQ(-1, result);
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));

  acquire_handle_free(h);
  PASS();
}

TEST test_download_reusability(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);

  /* Success */
  result =
      acquire_download_sync(h, GREATEST_URL, DOWNLOAD_DIR PATH_SEP "reuse1.h");
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT(is_file(DOWNLOAD_DIR PATH_SEP "reuse1.h"));

  /* Failure */
  result = acquire_download_sync(h, "https://httpbin.org/status/404",
                                 DOWNLOAD_DIR PATH_SEP "reuse2.tmp");
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ(ACQUIRE_ERROR_HTTP_FAILURE, acquire_handle_get_error_code(h));

  /* Success again */
  result =
      acquire_download_sync(h, GREATEST_URL, DOWNLOAD_DIR PATH_SEP "reuse3.h");
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT(is_file(DOWNLOAD_DIR PATH_SEP "reuse3.h"));

  acquire_handle_free(h);
  remove(DOWNLOAD_DIR PATH_SEP "reuse1.h");
  remove(DOWNLOAD_DIR PATH_SEP "reuse2.tmp");
  remove(DOWNLOAD_DIR PATH_SEP "reuse3.h");
  PASS();
}

SUITE(downloads_suite) {
  RUN_TEST(test_sync_download);
  RUN_TEST(test_async_download);
  RUN_TEST(test_sync_download_invalid_args);
  RUN_TEST(test_async_download_invalid_args);
  RUN_TEST(test_async_cancellation);
  RUN_TEST(test_download_404_error);
  RUN_TEST(test_download_bad_host);
  RUN_TEST(test_download_to_invalid_path);
  RUN_TEST(test_download_reusability);
}
#endif /* !TEST_DOWNLOAD_H */

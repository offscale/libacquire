#ifndef TEST_CHECKSUM_H
#define TEST_CHECKSUM_H

#include <acquire_common_defs.h>
#include <config_for_tests.h>
#include <greatest.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#else
#include <unistd.h>
#endif

TEST test_verify_sync_success(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  result =
      acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA256, GREATEST_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, h->status, "%d");
  acquire_handle_free(h);
  PASS();
}

TEST test_verify_sync_failure_bad_hash(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  result = acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA256, "deadbeef");
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR, h->status, "%d");
  acquire_handle_free(h);
  PASS();
}

TEST test_verify_sync_failure_bad_file(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  result =
      acquire_verify_sync(h, "nonexistent.file", LIBACQUIRE_SHA256, "hash");
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR, h->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_FILE_OPEN_FAILED,
                acquire_handle_get_error_code(h), "%d");
  acquire_handle_free(h);
  PASS();
}

TEST test_verify_async_success(void) {
  struct acquire_handle *h = acquire_handle_init();
  enum acquire_status status;
  ASSERT(h != NULL);

  acquire_verify_async_start(h, GREATEST_FILE, LIBACQUIRE_SHA256,
                             GREATEST_SHA256);
  do {
    status = acquire_verify_async_poll(h);
  } while (status == ACQUIRE_IN_PROGRESS);

  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, status, "%d");
  acquire_handle_free(h);
  PASS();
}

TEST test_verify_async_cancellation(void) {
  struct acquire_handle *h = acquire_handle_init();
  enum acquire_status status;
  ASSERT(h != NULL);

  /* Start hashing a larger file to ensure we can cancel it mid-process */
  acquire_verify_async_start(h, GREATEST_ARCHIVE, LIBACQUIRE_SHA256,
                             GREATEST_ARCHIVE_SHA256);

  /* Poll once to get it started */
  status = acquire_verify_async_poll(h);
  if (status == ACQUIRE_IN_PROGRESS) {
    acquire_verify_async_cancel(h);
    status = acquire_verify_async_poll(h);
  }

  ASSERT_EQ_FMT(ACQUIRE_ERROR, status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_CANCELLED, acquire_handle_get_error_code(h),
                "%d");
  acquire_handle_free(h);
  PASS();
}

SUITE(checksums_suite) {
  RUN_TEST(test_verify_sync_success);
  RUN_TEST(test_verify_sync_failure_bad_hash);
  RUN_TEST(test_verify_sync_failure_bad_file);
  RUN_TEST(test_verify_async_success);
  RUN_TEST(test_verify_async_cancellation);
}

#endif /* !TEST_CHECKSUM_H */

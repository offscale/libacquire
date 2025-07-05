#ifndef TEST_CHECKSUMS_DISPATCH_H
#define TEST_CHECKSUMS_DISPATCH_H

#include "acquire_checksums.h"
#include "acquire_handle.h"
#include <greatest.h>

TEST test_string2checksum(void) {
  ASSERT_EQ(LIBACQUIRE_CRC32C, string2checksum("CRC32C"));
  ASSERT_EQ(LIBACQUIRE_CRC32C, string2checksum("crc32c"));
  ASSERT_EQ(LIBACQUIRE_SHA256, string2checksum("SHA256"));
  ASSERT_EQ(LIBACQUIRE_SHA256, string2checksum("sha256"));
  ASSERT_EQ(LIBACQUIRE_SHA512, string2checksum("SHA512"));
  ASSERT_EQ(LIBACQUIRE_SHA512, string2checksum("sHa512"));
  ASSERT_EQ(LIBACQUIRE_UNSUPPORTED_CHECKSUM, string2checksum("UnknownAlgo"));
  ASSERT_EQ(LIBACQUIRE_UNSUPPORTED_CHECKSUM, string2checksum(NULL));
  PASS();
}

TEST test_dispatch_verify_start_invalid_args(void) {
  struct acquire_handle *h = acquire_handle_init();
  int res;

  res = acquire_verify_async_start(NULL, "file", LIBACQUIRE_SHA256, "hash");
  ASSERT_EQ(-1, res);

  res = acquire_verify_async_start(h, NULL, LIBACQUIRE_SHA256, "hash");
  ASSERT_EQ(-1, res);
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));

  res = acquire_verify_async_start(h, "file", LIBACQUIRE_SHA256, NULL);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));

  acquire_handle_free(h);
  PASS();
}

TEST test_dispatch_poll_no_backend(void) {
  struct acquire_handle *h = acquire_handle_init();
  enum acquire_status status;

  h->status = ACQUIRE_IN_PROGRESS;
  h->active_backend = ACQUIRE_BACKEND_NONE;

  status = acquire_verify_async_poll(h);

  ASSERT_EQ(ACQUIRE_ERROR, status);
  ASSERT_EQ(ACQUIRE_ERROR_UNKNOWN, acquire_handle_get_error_code(h));
  ASSERT(strstr(acquire_handle_get_error_string(h), "No active backend") !=
         NULL);

  acquire_handle_free(h);
  PASS();
}

TEST test_dispatch_poll_on_non_running_handle(void) {
  struct acquire_handle *h = acquire_handle_init();
  h->active_backend = ACQUIRE_BACKEND_NONE; /* to hit default */

  h->status = ACQUIRE_IDLE;
  ASSERT_EQ(ACQUIRE_IDLE, acquire_verify_async_poll(h));

  h->status = ACQUIRE_COMPLETE;
  ASSERT_EQ(ACQUIRE_COMPLETE, acquire_verify_async_poll(h));

  h->status = ACQUIRE_ERROR;
  ASSERT_EQ(ACQUIRE_ERROR, acquire_verify_async_poll(h));

  acquire_handle_free(h);
  PASS();
}

TEST test_dispatch_cancel_null_handle(void) {
  acquire_verify_async_cancel(NULL);
  PASS();
}

SUITE(checksum_dispatch_suite) {
  RUN_TEST(test_string2checksum);
  RUN_TEST(test_dispatch_verify_start_invalid_args);
  RUN_TEST(test_dispatch_poll_no_backend);
  RUN_TEST(test_dispatch_poll_on_non_running_handle);
  RUN_TEST(test_dispatch_cancel_null_handle);
}

#endif /* !TEST_CHECKSUMS_DISPATCH_H */

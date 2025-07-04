#ifndef TEST_HANDLE_H
#define TEST_HANDLE_H

#include "acquire_handle.h"
#include <greatest.h>
#include <string.h>

TEST test_handle_initialization(void) {
  struct acquire_handle *h = acquire_handle_init();
  ASSERT(h != NULL);
  ASSERT_EQ_FMT(-1L, (long)h->total_size, "%ld");
  ASSERT_EQ_FMT(ACQUIRE_IDLE, h->status, "%d");
  ASSERT_EQ(ACQUIRE_OK, h->error.code);
  ASSERT_EQ_FMT(0, h->cancel_flag, "%d");
  ASSERT_EQ(NULL, h->backend_handle);
  acquire_handle_free(h);
  PASS();
}

TEST test_handle_set_and_get_error(void) {
  struct acquire_handle *h = acquire_handle_init();
  const char *test_msg = "A test error occurred";
  ASSERT(h != NULL);

  acquire_handle_set_error(h, ACQUIRE_ERROR_UNKNOWN, "%s", test_msg);

  ASSERT_EQ_FMT(ACQUIRE_ERROR, h->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_UNKNOWN, acquire_handle_get_error_code(h), "%d");
  ASSERT_STR_EQ(test_msg, acquire_handle_get_error_string(h));

  acquire_handle_free(h);
  PASS();
}

TEST test_handle_null_safety(void) {
  /* These functions should not crash when given a NULL handle. */
  acquire_handle_free(NULL);
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT,
            acquire_handle_get_error_code(NULL));
  ASSERT_STR_EQ("Invalid handle provided.",
                acquire_handle_get_error_string(NULL));
  acquire_handle_set_error(NULL, ACQUIRE_ERROR_UNKNOWN, "test");
  /* No crash is a pass. */
  PASS();
}

TEST test_handle_set_error_with_formatting(void) {
  struct acquire_handle *h = acquire_handle_init();
  ASSERT(h != NULL);

  acquire_handle_set_error(h, ACQUIRE_ERROR_HTTP_FAILURE, "HTTP status was %d",
                           404);

  ASSERT_EQ_FMT(ACQUIRE_ERROR_HTTP_FAILURE, acquire_handle_get_error_code(h),
                "%d");
  ASSERT_STR_EQ("HTTP status was 404", acquire_handle_get_error_string(h));

  acquire_handle_free(h);
  PASS();
}

TEST test_handle_set_error_no_fmt(void) {
  struct acquire_handle *h = acquire_handle_init();
  ASSERT(h != NULL);

  acquire_handle_set_error(h, ACQUIRE_ERROR_UNKNOWN, NULL);

  ASSERT_EQ_FMT(ACQUIRE_ERROR, h->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_UNKNOWN, acquire_handle_get_error_code(h), "%d");
  ASSERT_STR_EQ("", acquire_handle_get_error_string(h));

  acquire_handle_free(h);
  PASS();
}

SUITE(handle_suite) {
  RUN_TEST(test_handle_initialization);
  RUN_TEST(test_handle_set_and_get_error);
  RUN_TEST(test_handle_null_safety);
  RUN_TEST(test_handle_set_error_with_formatting);
  RUN_TEST(test_handle_set_error_no_fmt);
}

#endif /* !TEST_HANDLE_H */

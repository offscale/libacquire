#ifndef TEST_OPENSSL_H
#define TEST_OPENSSL_H

#include <greatest.h>
#include <stdio.h>

#include "acquire_checksums.h"
#include "config_for_tests.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <unistd.h>
#endif

#if (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL) ||             \
    (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO) || \
    (defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL)

TEST test_openssl_backend_unsupported_algo(void) {
  struct acquire_handle *h = acquire_handle_init();
  int res;

  /*
   * The OpenSSL backend doesn't support CRC32C.
   * This should fail in the `_start` function when it's dispatched.
   */
  h->active_backend = ACQUIRE_BACKEND_CHECKSUM_OPENSSL;
  res = acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_INVALID_CHECKSUM_ALGO,
                            GREATEST_CRC32C);
  ASSERT_EQ(-1, res);
  ASSERT_EQ(ACQUIRE_ERROR_UNSUPPORTED_CHECKSUM_FORMAT,
            acquire_handle_get_error_code(h));
  acquire_handle_free(h);
  PASS();
}

TEST test_openssl_backend_poll_on_finished_handle(void) {
  struct acquire_handle *h = acquire_handle_init();
  enum acquire_status status;
  h->active_backend = ACQUIRE_BACKEND_CHECKSUM_OPENSSL;

  /* Run a successful verification to completion */
  acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA256, GREATEST_SHA256);
  ASSERT_EQ(ACQUIRE_COMPLETE, h->status);

  /* Polling again on a completed handle should return the same status */
  h->active_backend = ACQUIRE_BACKEND_CHECKSUM_OPENSSL;
  status = acquire_verify_async_poll(h);
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, status, "%d");

  acquire_handle_free(h);
  PASS();
}

TEST test_openssl_backend_poll_on_null_backend(void) {
  struct acquire_handle *h = acquire_handle_init();
  h->status = ACQUIRE_IN_PROGRESS;
  h->active_backend = ACQUIRE_BACKEND_CHECKSUM_OPENSSL; /* Force backend */
  /* Do not create backend handle */

  ASSERT_EQ(ACQUIRE_ERROR, acquire_verify_async_poll(h));
  ASSERT_EQ(ACQUIRE_ERROR_UNKNOWN, acquire_handle_get_error_code(h));
  ASSERT(strstr(acquire_handle_get_error_string(h),
                "In-progress poll with NULL backend") != NULL);

  acquire_handle_free(h);
  PASS();
}

TEST test_openssl_backend_cancel_null(void) {
  acquire_verify_async_cancel(NULL);
  PASS();
}

SUITE(openssl_backend_suite) {
  RUN_TEST(test_openssl_backend_unsupported_algo);
  RUN_TEST(test_openssl_backend_poll_on_finished_handle);
  RUN_TEST(test_openssl_backend_poll_on_null_backend);
  RUN_TEST(test_openssl_backend_cancel_null);
}

#endif /* (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL) ||       \
          (defined(LIBACQUIRE_USE_COMMON_CRYPTO) &&                            \
          LIBACQUIRE_USE_COMMON_CRYPTO) || (defined(LIBACQUIRE_USE_LIBRESSL)   \
          && LIBACQUIRE_USE_LIBRESSL) */

#endif /* !TEST_OPENSSL_H */

#ifndef TEST_LIBRHASH_H
#define TEST_LIBRHASH_H

#if defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH

#include <greatest.h>
#include <stdio.h>

#include "acquire_checksums.h" /* For string2checksum and verify API */
#include "acquire_fileutils.h" /* For is_file */
#include "acquire_handle.h"    /* For handle functions */
#include "config_for_tests.h"

TEST test_rhash_backend_invalid_crc_hash_len(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  result = acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_CRC32C, "short");
  ASSERT_EQ(-1, result);
  ASSERT_EQ(ACQUIRE_ERROR_UNSUPPORTED_CHECKSUM_FORMAT,
            acquire_handle_get_error_code(h));
  acquire_handle_free(h);
  PASS();
}

TEST test_rhash_backend_poll_null(void) {
  ASSERT_EQ(ACQUIRE_ERROR, _librhash_verify_async_poll(NULL));
  PASS();
}

TEST test_rhash_backend_poll_on_null_backend(void) {
  struct acquire_handle *h = acquire_handle_init();
  h->status = ACQUIRE_IN_PROGRESS;
  h->backend_handle = NULL;
  h->active_backend = ACQUIRE_BACKEND_CHECKSUM_LIBRHASH;

  ASSERT_EQ(ACQUIRE_ERROR, _librhash_verify_async_poll(h));
  ASSERT_EQ(ACQUIRE_ERROR_UNKNOWN, acquire_handle_get_error_code(h));
  ASSERT(strstr(acquire_handle_get_error_string(h),
                "In-progress poll with NULL backend") != NULL);

  acquire_handle_free(h);
  PASS();
}

TEST test_rhash_backend_poll_on_non_running_handle(void) {
  struct acquire_handle *h = acquire_handle_init();
  h->active_backend = ACQUIRE_BACKEND_CHECKSUM_LIBRHASH; /* Force backend */

  h->status = ACQUIRE_IDLE;
  ASSERT_EQ(ACQUIRE_IDLE, _librhash_verify_async_poll(h));

  h->status = ACQUIRE_COMPLETE;
  ASSERT_EQ(ACQUIRE_COMPLETE, _librhash_verify_async_poll(h));

  h->status = ACQUIRE_ERROR;
  ASSERT_EQ(ACQUIRE_ERROR, _librhash_verify_async_poll(h));

  acquire_handle_free(h);
  PASS();
}

TEST test_rhash_backend_cancel_null(void) {
  _librhash_verify_async_cancel(NULL);
  PASS();
}

SUITE(librhash_backend_suite) {
  RUN_TEST(test_rhash_backend_invalid_crc_hash_len);
  RUN_TEST(test_rhash_backend_poll_null);
  RUN_TEST(test_rhash_backend_poll_on_null_backend);
  RUN_TEST(test_rhash_backend_poll_on_non_running_handle);
  RUN_TEST(test_rhash_backend_cancel_null);
}
#endif /* defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH */
#endif /* !TEST_LIBRHASH_H */

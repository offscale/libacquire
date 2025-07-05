#ifndef TEST_CHECKSUM_H
#define TEST_CHECKSUM_H

#include <greatest.h>
#include <stdio.h>

#include "acquire_checksums.h" /* For string2checksum and verify API */
#include "acquire_fileutils.h" /* For is_file */
#include "acquire_handle.h"    /* For handle functions */
#include "config_for_tests.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <unistd.h>
#endif

#define EMPTY_FILE_SHA256                                                      \
  "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
#define GREATEST_SHA512                                                        \
  "3dd506fcf7b60d46e3a3865b9159f19ad35b359398bcb736c9f8de239059187ae625fb0041" \
  "7f16a8bb8520c7bbeb388e974a3294aa3eb4c4f93185831ed2b6a2e"
static const char *EMPTY_FILE_PATH = DOWNLOAD_DIR PATH_SEP "empty.txt";

TEST test_verify_sync_success_sha256(void) {
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

TEST test_verify_sync_success_sha512(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  result =
      acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA512, GREATEST_SHA512);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, h->status, "%d");
  acquire_handle_free(h);
  PASS();
}

#if defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH ||             \
    defined(LIBACQUIRE_USE_CRC32C) && LIBACQUIRE_USE_CRC32C
TEST test_verify_sync_success_crc32c(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  result =
      acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_CRC32C, GREATEST_CRC32C);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, h->status, "%d");
  acquire_handle_free(h);
  PASS();
}
#endif /* defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH ||       \
          defined(LIBACQUIRE_USE_CRC32C) && LIBACQUIRE_USE_CRC32C */

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
  result = acquire_verify_sync(h, "nonexistent.file", LIBACQUIRE_SHA256,
                               GREATEST_SHA256);
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

TEST test_verify_empty_file(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  FILE *f = fopen(EMPTY_FILE_PATH, "w");
  if (f)
    fclose(f);

  ASSERT(h != NULL);
  result = acquire_verify_sync(h, EMPTY_FILE_PATH, LIBACQUIRE_SHA256,
                               EMPTY_FILE_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, h->status, "%d");
  acquire_handle_free(h);
  remove(EMPTY_FILE_PATH);
  PASS();
}

TEST test_unsupported_algorithm(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  result = acquire_verify_sync(h, GREATEST_FILE,
                               LIBACQUIRE_UNSUPPORTED_CHECKSUM, "hash");
  ASSERT_EQ(-1, result);
  ASSERT_EQ_FMT(ACQUIRE_ERROR_UNSUPPORTED_CHECKSUM_FORMAT,
                acquire_handle_get_error_code(h), "%d");
  acquire_handle_free(h);
  PASS();
}

TEST test_invalid_hash_length(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  /* SHA256 should be 64 chars, provide a shorter one */
  result =
      acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA256, "short_hash");
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ(ACQUIRE_ERROR, h->status);
  ASSERT_EQ(ACQUIRE_ERROR_UNSUPPORTED_CHECKSUM_FORMAT,
            acquire_handle_get_error_code(h));
  acquire_handle_free(h);
  PASS();
}

TEST test_verify_reusability(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);

  /* First, a successful verification */
  result =
      acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA256, GREATEST_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ(ACQUIRE_COMPLETE, h->status);

  /* Resetting state is done inside acquire_verify_sync. Now, a failing one. */
  result = acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA256, "badhash");
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ(ACQUIRE_ERROR, h->status);
  ASSERT_NEQ(ACQUIRE_OK, acquire_handle_get_error_code(h));

  /* And another successful one, showing the handle is clean for reuse */
  result =
      acquire_verify_sync(h, GREATEST_FILE, LIBACQUIRE_SHA256, GREATEST_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ(ACQUIRE_COMPLETE, h->status);

  acquire_handle_free(h);
  PASS();
}

SUITE(checksums_suite) {
  RUN_TEST(test_unsupported_algorithm);
  RUN_TEST(test_invalid_hash_length);
  RUN_TEST(test_verify_sync_success_sha256);
  RUN_TEST(test_verify_sync_success_sha512);
#if defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH ||             \
    defined(LIBACQUIRE_USE_CRC32C) && LIBACQUIRE_USE_CRC32C
  RUN_TEST(test_verify_sync_success_crc32c);
#endif /* defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH ||       \
          defined(LIBACQUIRE_USE_CRC32C) && LIBACQUIRE_USE_CRC32C */
  RUN_TEST(test_verify_sync_failure_bad_hash);
  RUN_TEST(test_verify_sync_failure_bad_file);
  RUN_TEST(test_verify_async_success);
  RUN_TEST(test_verify_async_cancellation);
  RUN_TEST(test_verify_empty_file);
  RUN_TEST(test_verify_reusability);
}

#endif /* !TEST_CHECKSUM_H */

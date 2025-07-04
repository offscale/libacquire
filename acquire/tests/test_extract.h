#ifndef LIBACQUIRE_TEST_EXTRACT_H
#define LIBACQUIRE_TEST_EXTRACT_H

#include <stdio.h>
#include <string.h>

#include <greatest.h>

#include "acquire_checksums.h"
#include "acquire_common_defs.h"
#include "acquire_config.h"
#include "acquire_download.h"
#include "acquire_extract.h"
#include "acquire_fileutils.h"
#include "acquire_handle.h"
#include "config_for_tests.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <synchapi.h>
#else
#include <unistd.h>
#endif

#define TO_STRING(x) #x
#define STR(x) TO_STRING(x)

#define EXTRACT_DIR DOWNLOAD_DIR PATH_SEP STR(ARCHIVE_LIB) "_test_extract"
#define EXTRACTED_CONTENT                                                      \
  EXTRACT_DIR PATH_SEP "greatest-cmake-and-msvc" PATH_SEP "greatest.h"

TEST test_extract_sync_success(void) {
  struct acquire_handle *handle = acquire_handle_init();
  struct acquire_handle *verify_handle = acquire_handle_init();
  int result;
  ASSERT(handle != NULL && verify_handle != NULL);

  result = acquire_extract_sync(handle, GREATEST_ARCHIVE, EXTRACT_DIR);

  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, handle->status, "%d");
  ASSERT(is_file(EXTRACTED_CONTENT));
  result = acquire_verify_sync(verify_handle, EXTRACTED_CONTENT,
                               LIBACQUIRE_SHA256, GREATEST_SHA256);
  ASSERT_EQ_FMT(0, result, "%d");

  acquire_handle_free(handle);
  acquire_handle_free(verify_handle);
  PASS();
}

TEST test_extract_non_existent_archive(void) {
  struct acquire_handle *handle = acquire_handle_init();
  int result;
  ASSERT(handle != NULL);
  result =
      acquire_extract_sync(handle, "non-existent-archive.zip", EXTRACT_DIR);
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_ARCHIVE_OPEN_FAILED,
                acquire_handle_get_error_code(handle), "%d");
  acquire_handle_free(handle);
  PASS();
}

TEST test_extract_corrupted_archive(void) {
  struct acquire_handle *handle = acquire_handle_init();
  int result;
  ASSERT(handle != NULL);
  result = acquire_extract_sync(handle, CORRUPT_ARCHIVE, EXTRACT_DIR);
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT(acquire_handle_get_error_code(handle) >=
         ACQUIRE_ERROR_ARCHIVE_OPEN_FAILED);
  acquire_handle_free(handle);
  PASS();
}
TEST test_extract_invalid_args(void) {
  struct acquire_handle *h = acquire_handle_init();
  ASSERT_EQ(-1, acquire_extract_sync(NULL, "archive.zip", "dest"));
  ASSERT_EQ(-1, acquire_extract_sync(h, NULL, "dest"));
  ASSERT_EQ(-1, acquire_extract_sync(h, "archive.zip", NULL));
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));
  acquire_handle_free(h);
  PASS();
}

TEST test_extract_reusability(void) {
  struct acquire_handle *h = acquire_handle_init();
  int result;
  ASSERT(h != NULL);
  result = acquire_extract_sync(h, GREATEST_ARCHIVE, EXTRACT_DIR "_reuse1");
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT(is_file(EXTRACT_DIR "_reuse1" PATH_SEP
                             "greatest-cmake-and-msvc" PATH_SEP "greatest.h"));
  result = acquire_extract_sync(h, CORRUPT_ARCHIVE, EXTRACT_DIR "_reuse2");
  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT(acquire_handle_get_error_code(h) >= ACQUIRE_ERROR_ARCHIVE_OPEN_FAILED);
  result = acquire_extract_sync(h, GREATEST_ARCHIVE, EXTRACT_DIR "_reuse3");
  ASSERT_EQ_FMT(0, result, "%d");
  ASSERT(is_file(EXTRACT_DIR "_reuse3" PATH_SEP
                             "greatest-cmake-and-msvc" PATH_SEP "greatest.h"));
  acquire_handle_free(h);
  PASS();
}

TEST test_async_extract_invalid_args(void) {
  struct acquire_handle *h = acquire_handle_init();
  enum acquire_status status;

  ASSERT_EQ(-1, acquire_extract_async_start(NULL, "archive.zip", "dest"));
  ASSERT_EQ(-1, acquire_extract_async_start(h, NULL, "dest"));
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));
  ASSERT_EQ(-1, acquire_extract_async_start(h, "archive.zip", NULL));
  ASSERT_EQ(ACQUIRE_ERROR_INVALID_ARGUMENT, acquire_handle_get_error_code(h));

  status = acquire_extract_async_poll(NULL);
  ASSERT_EQ(ACQUIRE_ERROR, status);

  acquire_extract_async_cancel(NULL);

  acquire_handle_free(h);
  PASS();
}

/* --- Backend specific tests --- */

#if defined(LIBACQUIRE_USE_WINCOMPRESSAPI) && LIBACQUIRE_USE_WINCOMPRESSAPI
TEST test_extract_wincompressapi_placeholder(void) {
  struct acquire_handle *handle = acquire_handle_init();
  int result;
  ASSERT(handle != NULL);

  result = acquire_extract_sync(handle, GREATEST_ARCHIVE, EXTRACT_DIR);

  ASSERT_EQ_FMT(-1, result, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR, handle->status, "%d");
  ASSERT_EQ_FMT(ACQUIRE_ERROR_UNSUPPORTED_ARCHIVE_FORMAT,
                acquire_handle_get_error_code(handle), "%d");
  acquire_handle_free(handle);
  PASS();
}
#endif /* defined(LIBACQUIRE_USE_WINCOMPRESSAPI) &&                            \
          LIBACQUIRE_USE_WINCOMPRESSAPI */

#if defined(LIBACQUIRE_USE_MINIZ) && LIBACQUIRE_USE_MINIZ
TEST test_extract_miniz_cancellation(void) {
  struct acquire_handle *handle = acquire_handle_init();
  int result;
  ASSERT(handle != NULL);

  /* The miniz backend is synchronous, but cancellation is checked
     via a callback. We can trigger it before starting. */
  acquire_extract_async_cancel(handle); /* Sets the flag */

  result = acquire_extract_sync(handle, GREATEST_ARCHIVE, EXTRACT_DIR);

  ASSERT_EQ(-1, result);
  ASSERT_EQ(ACQUIRE_ERROR, handle->status);
  ASSERT_EQ(ACQUIRE_ERROR_CANCELLED, acquire_handle_get_error_code(handle));

  acquire_handle_free(handle);
  PASS();
}
#endif /* defined(LIBACQUIRE_USE_MINIZ) && LIBACQUIRE_USE_MINIZ */

#if defined(LIBACQUIRE_USE_LIBARCHIVE) && LIBACQUIRE_USE_LIBARCHIVE
TEST test_extract_async_success(void) {
  struct acquire_handle *handle = acquire_handle_init();
  enum acquire_status status;
  ASSERT(handle != NULL);

  ASSERT_EQ(0,
            acquire_extract_async_start(handle, GREATEST_ARCHIVE, EXTRACT_DIR));

  do {
    status = acquire_extract_async_poll(handle);
    if (status == ACQUIRE_IN_PROGRESS) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
      Sleep(1);
#else
      usleep(1000);
#endif
    }
  } while (status == ACQUIRE_IN_PROGRESS);

  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, handle->status, "%d");
  ASSERT(is_file(EXTRACTED_CONTENT));

  acquire_handle_free(handle);
  /* TODO: cleanup extracted files */
  PASS();
}

TEST test_extract_async_cancellation(void) {
  struct acquire_handle *handle = acquire_handle_init();
  enum acquire_status status = ACQUIRE_IDLE;
  int was_in_progress = 0;

  ASSERT(handle != NULL);

  ASSERT_EQ(0,
            acquire_extract_async_start(handle, GREATEST_ARCHIVE, EXTRACT_DIR));

  /* Poll once to get it started */
  status = acquire_extract_async_poll(handle);
  if (status == ACQUIRE_IN_PROGRESS)
    was_in_progress = 1;

  if (status == ACQUIRE_IN_PROGRESS) {
    acquire_extract_async_cancel(handle);
    /* Poll until no longer in progress */
    while ((status = acquire_extract_async_poll(handle)) ==
           ACQUIRE_IN_PROGRESS) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
      Sleep(10);
#else
      usleep(10000);
#endif
    }
  }

  /*
   * Due to the small size of the test archive, the first poll might complete
   * the entire operation. We only run the assertions if the cancellation
   * was actually attempted (i.e., if the operation was still in progress
   * after the first poll).
   */
  if (was_in_progress) {
    ASSERT_EQ_FMT(ACQUIRE_ERROR, status, "%d");
    ASSERT_EQ_FMT(ACQUIRE_ERROR_CANCELLED,
                  acquire_handle_get_error_code(handle), "%d");
  } else {
    /* If the test was too fast to cancel, we just log it as a pass. */
    ASSERT(status == ACQUIRE_COMPLETE || status == ACQUIRE_ERROR);
  }

  acquire_handle_free(handle);
  PASS();
}

#endif /* defined(LIBACQUIRE_USE_LIBARCHIVE) && LIBACQUIRE_USE_LIBARCHIVE */

SUITE(extract_suite) {
  RUN_TEST(test_extract_sync_success);
  RUN_TEST(test_extract_non_existent_archive);
  RUN_TEST(test_extract_corrupted_archive);
  RUN_TEST(test_extract_invalid_args);
  RUN_TEST(test_async_extract_invalid_args);
  RUN_TEST(test_extract_reusability);

#if defined(LIBACQUIRE_USE_WINCOMPRESSAPI) && LIBACQUIRE_USE_WINCOMPRESSAPI
  RUN_TEST(test_extract_wincompressapi_placeholder);
#endif /* defined(LIBACQUIRE_USE_WINCOMPRESSAPI) &&                            \
          LIBACQUIRE_USE_WINCOMPRESSAPI */
#if defined(LIBACQUIRE_USE_MINIZ) && LIBACQUIRE_USE_MINIZ
  RUN_TEST(test_extract_miniz_cancellation);
#endif /* defined(LIBACQUIRE_USE_MINIZ) && LIBACQUIRE_USE_MINIZ */

#if defined(LIBACQUIRE_USE_LIBARCHIVE) && LIBACQUIRE_USE_LIBARCHIVE
  RUN_TEST(test_extract_async_success);
  RUN_TEST(test_extract_async_cancellation);
#endif /* defined(LIBACQUIRE_USE_LIBARCHIVE) && LIBACQUIRE_USE_LIBARCHIVE */
}
#endif /* !LIBACQUIRE_TEST_EXTRACT_H */

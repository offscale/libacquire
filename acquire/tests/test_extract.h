#ifndef LIBACQUIRE_TEST_EXTRACT_H
#define LIBACQUIRE_TEST_EXTRACT_H

#include <stdio.h>
#include <string.h>

#include <greatest.h>

#include "acquire_checksums.h"
#include "acquire_common_defs.h"
#include "acquire_config.h"
#include "acquire_extract.h"
#include "acquire_fileutils.h"
#include "acquire_handle.h"
#include "config_for_tests.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

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

  /* Verify content */
  ASSERT(is_file(EXTRACTED_CONTENT));
  ASSERT_EQ_FMT(0,
                acquire_verify_sync(verify_handle, EXTRACTED_CONTENT,
                                    LIBACQUIRE_SHA256, GREATEST_SHA256),
                "%d");

  acquire_handle_free(handle);
  acquire_handle_free(verify_handle);
  PASS();
}

TEST test_extract_async_success(void) {
  struct acquire_handle *handle = acquire_handle_init();
  struct acquire_handle *verify_handle = acquire_handle_init();
  enum acquire_status status;

  ASSERT(handle != NULL && verify_handle != NULL);

  ASSERT_EQ(0,
            acquire_extract_async_start(handle, GREATEST_ARCHIVE, EXTRACT_DIR));

  do {
    status = acquire_extract_async_poll(handle);
#if defined(LIBACQUIRE_USE_LIBARCHIVE) && LIBACQUIRE_USE_LIBARCHIVE
    /* Give libarchive a moment between polling */
    usleep(1000);
#endif
  } while (status == ACQUIRE_IN_PROGRESS);

  ASSERT_EQ_FMT(ACQUIRE_COMPLETE, handle->status, "%d");
  ASSERT(is_file(EXTRACTED_CONTENT));
  ASSERT_EQ_FMT(0,
                acquire_verify_sync(verify_handle, EXTRACTED_CONTENT,
                                    LIBACQUIRE_SHA256, GREATEST_SHA256),
                "%d");

  acquire_handle_free(handle);
  acquire_handle_free(verify_handle);
  PASS();
}

TEST test_extract_async_cancellation(void) {
  struct acquire_handle *handle = acquire_handle_init();
  enum acquire_status status;

  ASSERT(handle != NULL);

#if defined(LIBACQUIRE_USE_MINIZ) && LIBACQUIRE_USE_MINIZ
  /* miniz backend is fully synchronous in _start, so we can't cancel.
     Skip this test for that backend. */
  (void)status;
  SKIPm("miniz backend is synchronous, cannot test async cancellation.");
#else
  {
    int start_result =
        acquire_extract_async_start(handle, GREATEST_ARCHIVE, EXTRACT_DIR);
    ASSERT_EQ_FMT(0, start_result, "%d");
  }

  /* Poll once, then cancel. This assumes extract isn't instantaneous. */
  status = acquire_extract_async_poll(handle);
  if (status == ACQUIRE_IN_PROGRESS) {
    acquire_extract_async_cancel(handle);
    status = acquire_extract_async_poll(handle);
  }

  /* The operation might finish before cancel is processed, which is ok.
     We only fail if it's neither COMPLETE nor ERROR. */
  ASSERT(status == ACQUIRE_COMPLETE || status == ACQUIRE_ERROR);
  if (status == ACQUIRE_ERROR) {
    ASSERT_EQ_FMT(ACQUIRE_ERROR_CANCELLED,
                  acquire_handle_get_error_code(handle), "%d");
  } else {
    printf(" (Note: Extraction completed before cancellation took effect)\n");
  }

  acquire_handle_free(handle);
  PASS();
#endif
}

SUITE(extract_suite) {
  RUN_TEST(test_extract_sync_success);
  RUN_TEST(test_extract_async_success);
  RUN_TEST(test_extract_async_cancellation);
}

#endif /* !LIBACQUIRE_TEST_EXTRACT_H */

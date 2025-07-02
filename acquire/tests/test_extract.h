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

SUITE(extract_suite) {
  RUN_TEST(test_extract_sync_success);
  RUN_TEST(test_extract_non_existent_archive);
  RUN_TEST(test_extract_corrupted_archive);
}

#endif /* !LIBACQUIRE_TEST_EXTRACT_H */

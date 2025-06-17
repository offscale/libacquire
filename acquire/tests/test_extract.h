#ifndef LIBACQUIRE_TEST_EXTRACT_H
#define LIBACQUIRE_TEST_EXTRACT_H

#include <greatest.h>
#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#define TO_STRING(x) #x
#define STR(x) TO_STRING(x)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#include <acquire_config.h>
#include <acquire_fileutils.h>

#ifdef ARCHIVE_LIB
#include STR(ARCHIVE_HEADER_NAME)
#elif defined(LIBACQUIRE_USE_MINIZ) && LIBACQUIRE_USE_MINIZ
#include <acquire_miniz.h>
#elif defined(LIBACQUIRE_USE_LIBARCHIVE) && LIBACQUIRE_USE_LIBARCHIVE
#include <acquire_libarchive.h>
#else
#error                                                                         \
    "ARCHIVE_LIB must be defined to the backend name (e.g. miniz or libarchive)" \
    "or LIBACQUIRE_USE_MINIZ or LIBACQUIRE_USE_LIBARCHIVE must be defined";
#endif /* ARCHIVE_LIB */

#include <acquire_common_defs.h>
#include <acquire_extract.h>
#include <config_for_tests.h>

TEST x_test_extract_archive(void) {
#define EXTRACT_DIR DOWNLOAD_DIR PATH_SEP "extract" PATH_SEP STR(ARCHIVE_LIB)
  puts("\"test_extract.h\" for ARCHIVE_LIB: \"" STR(
      ARCHIVE_LIB) "\" into EXTRACT_DIR: \"" EXTRACT_DIR "\"");

  ASSERT_FALSE(extract_archive(LIBACQUIRE_ZIP, MINIZ_ZIP_FILE, EXTRACT_DIR) !=
               EXIT_SUCCESS);

  /* Could add a comprehensive file check here */
  ASSERT_FALSE(!is_file(EXTRACT_DIR PATH_SEP "readme.md"));
  ASSERT_FALSE(!is_file(EXTRACT_DIR PATH_SEP "miniz.h"));
  ASSERT_FALSE(!is_file(EXTRACT_DIR PATH_SEP "miniz.c"));
  ASSERT_FALSE(!is_file(EXTRACT_DIR PATH_SEP "ChangeLog.md"));
  ASSERT_FALSE(!is_file(EXTRACT_DIR PATH_SEP "LICENSE"));
  ASSERT_FALSE(!is_directory(EXTRACT_DIR PATH_SEP "examples"));

#undef EXTRACT_DIR
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(extract_suite) { RUN_TEST(x_test_extract_archive); }

#endif /* ! LIBACQUIRE_TEST_EXTRACT_H */

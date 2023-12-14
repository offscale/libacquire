#ifndef LIBACQUIRE_TEST_EXTRACT_H
#define LIBACQUIRE_TEST_EXTRACT_H

#include <greatest.h>
#include <stdbool.h>

#define TO_STRING(x) #x
#define STR(x) TO_STRING(x)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#define LIBACQUIRE_IMPLEMENTATION
#include <acquire_fileutils.h>
#include ARCHIVE_LIB_NAME
#undef LIBACQUIRE_IMPLEMENTATION

#include <acquire_common_defs.h>
#include <acquire_config.h>
#include <acquire_extract.h>
#include <config_for_tests.h>

TEST x_test_extract_archive(void) {
#define EXTRACT_DIR DOWNLOAD_DIR PATH_SEP "extract" PATH_SEP ARCHIVE_LIB
  puts("\"test_extract.h\" for ARCHIVE_LIB: \"" ARCHIVE_LIB
       "\" into EXTRACT_DIR: \"" EXTRACT_DIR "\"");

  ASSERT_FALSE(extract_archive(LIBACQUIRE_ZIP, MINIZ_ZIP_FILE, EXTRACT_DIR) !=
               EXIT_SUCCESS);

  /* Could do the whole `find zlib-1.2.11 -type f | sha256sum` test to be proper
   */
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

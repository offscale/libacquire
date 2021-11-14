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

#include <config_for_tests.h>
#include <acquire_config.h>

#define LIBACQUIRE_IMPLEMENTATION
#include <acquire_fileutils.h>
#ifdef ON_libarchive
#define USE_LIBARCHIVE 1
#include <acquire_libarchive.h>
#elif defined(ON_miniz)
#define USE_MINIZ 1
#include <acquire_miniz.h>
#elif defined(ON_zlib)
#define USE_ZLIB 1
#include <acquire_zlib>
#endif /* ON_libarchive */

TEST x_test_extract_archive(void) {
    ASSERT_FALSE(extract_archive(LIBACQUIRE_ZIP, MINIZ_ZIP_FILE, DOWNLOAD_DIR PATH_SEP "examples") != EXIT_SUCCESS);

#define EXTRACT_DIR DOWNLOAD_DIR PATH_SEP "examples" PATH_SEP

    /* Could do the whole `find zlib-1.2.11 -type f | sha256sum` test to be proper */
    ASSERT_FALSE(!is_file(EXTRACT_DIR "readme.md"));
    ASSERT_FALSE(!is_file(EXTRACT_DIR "miniz.h"));
    ASSERT_FALSE(!is_file(EXTRACT_DIR "miniz.c"));
    ASSERT_FALSE(!is_file(EXTRACT_DIR "ChangeLog.md"));
    ASSERT_FALSE(!is_file(EXTRACT_DIR "LICENSE"));
    ASSERT_FALSE(!is_directory(EXTRACT_DIR "examples"));

#undef EXTRACT_DIR
    PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE (extract_suite) {
    RUN_TEST(x_test_extract_archive);
}

#endif /* ! LIBACQUIRE_TEST_EXTRACT_H */

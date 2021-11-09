#include <greatest.h>
#include <stdbool.h>

#include <config_for_tests.h>
#include <acquire_config.h>
#define LIBACQUIRE_IMPLEMENTATION
#define USE_MINIZ
#include <acquire_miniz.h>
#include <acquire_fileutils.h>

TEST x_test_should_extract_zip_with_miniz(void) {
    ASSERT_FALSE(extract_archive(LIBACQUIRE_ZIP, MINIZ_ZIP_FILE, DOWNLOAD_DIR) != EXIT_SUCCESS);

    /* Could do the whole `find zlib-1.2.11 -type f | sha256sum` test to be proper */
    ASSERT_FALSE(!is_file(DOWNLOAD_DIR PATH_SEP MINIZ_BASENAME_NO_EXT "readme.md"));
    ASSERT_FALSE(!is_file(DOWNLOAD_DIR PATH_SEP MINIZ_BASENAME_NO_EXT "miniz.h"));
    ASSERT_FALSE(!is_file(DOWNLOAD_DIR PATH_SEP MINIZ_BASENAME_NO_EXT "miniz.c"));
    ASSERT_FALSE(!is_file(DOWNLOAD_DIR PATH_SEP MINIZ_BASENAME_NO_EXT "ChangeLog.md"));
    ASSERT_FALSE(!is_file(DOWNLOAD_DIR PATH_SEP MINIZ_BASENAME_NO_EXT "LICENSE"));

    PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE (miniz_suite) {
    RUN_TEST(x_test_should_extract_zip_with_miniz);
}

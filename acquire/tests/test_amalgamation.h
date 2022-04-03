#include <greatest.h>
#include <stdbool.h>

#include <acquire_config.h>
#include <config_for_tests.h>
#define LIBACQUIRE_IMPLEMENTATION
#include <acquire.h>

TEST x_test_file_downloads(void) {
  const int download_resp =
      download(GREATEST_URL, LIBACQUIRE_SHA256, GREATEST_SHA256, DOWNLOAD_DIR,
               false, 0, 0);
  if (download_resp != EXIT_SUCCESS)
    printf("download_resp: %d\n", download_resp);
  ASSERT_EQ(download_resp, EXIT_SUCCESS);
  ASSERT_FALSE(
      !sha256(DOWNLOAD_DIR PATH_SEP GREATEST_BASEFILENAME, GREATEST_SHA256));
  ASSERT_FALSE(!sha256(GREATEST_FILE, GREATEST_SHA256));
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(downloads_suite) { RUN_TEST(x_test_file_downloads); }

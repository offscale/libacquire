#ifndef TEST_NET_COMMON_H
#define TEST_NET_COMMON_H

#include <acquire_download.h>
#include <acquire_fileutils.h>
#include <acquire_net_common.h>
#include <config_for_tests.h>
#include <greatest.h>

#define NET_COMMON_TEST_FILE (DOWNLOAD_DIR PATH_SEP "greatest.h")

static void setup_net_common_suite(void *arg) {
  (void)arg;
  /* Ensure the file needed for tests exists. */
  if (!is_file(NET_COMMON_TEST_FILE)) {
    struct acquire_handle *h = acquire_handle_init();
    acquire_download_sync(h, GREATEST_URL, NET_COMMON_TEST_FILE);
    acquire_handle_free(h);
  }
}

TEST test_is_downloaded_success(void) {
  /* This test uses the URL to determine the filename "greatest.h", then
     checks for it in DOWNLOAD_DIR. */
  bool result = is_downloaded(GREATEST_URL, LIBACQUIRE_SHA256, GREATEST_SHA256,
                              DOWNLOAD_DIR);
  ASSERT(result);
  PASS();
}

TEST test_is_downloaded_file_missing(void) {
  /* Use a URL for a file that does not exist locally. */
  const char *DUMMY_URL = "http://example.com/non_existent_file.txt";
  bool result =
      is_downloaded(DUMMY_URL, LIBACQUIRE_SHA256, "somehash", DOWNLOAD_DIR);
  ASSERT_FALSE(result);
  PASS();
}

TEST test_is_downloaded_bad_hash(void) {
  /* Use correct URL/file but provide an incorrect hash. */
  bool result = is_downloaded(GREATEST_URL, LIBACQUIRE_SHA256, "incorrect_hash",
                              DOWNLOAD_DIR);
  ASSERT_FALSE(result);
  PASS();
}

TEST test_is_downloaded_bad_algorithm(void) {
  /*
   * Provide the correct SHA256 hash but ask the function to verify it using
   * CRC32C, which should fail.
   */
  bool result =
      is_downloaded(GREATEST_URL, LIBACQUIRE_CRC32C, /* Mismatched algo */
                    GREATEST_SHA256, DOWNLOAD_DIR);
  ASSERT_FALSE(result);
  PASS();
}

SUITE(net_common_suite) {
  setup_net_common_suite(NULL); /* Manually call setup. */
  RUN_TEST(test_is_downloaded_success);
  RUN_TEST(test_is_downloaded_file_missing);
  RUN_TEST(test_is_downloaded_bad_hash);
  RUN_TEST(test_is_downloaded_bad_algorithm);
}

#endif /* !TEST_NET_COMMON_H */

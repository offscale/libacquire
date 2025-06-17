#ifndef TEST_DOWNLOAD_H
#define TEST_DOWNLOAD_H

#include <greatest.h>

#include <acquire_common_defs.h>
#include <acquire_config.h>
#include <config_for_tests.h>

#if (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO) || \
    (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL)
#include <acquire_openssl.h>
#elif defined(LIBACQUIRE_USE_WINCRYPT) && LIBACQUIRE_USE_WINCRYPT
#include <acquire_wincrypt.h>
#endif /* (defined(LIBACQUIRE_USE_COMMON_CRYPTO) &&                            \
          LIBACQUIRE_USE_COMMON_CRYPTO) || (defined(LIBACQUIRE_USE_OPENSSL) &&                                           \
          LIBACQUIRE_USE_OPENSSL) */

#include <acquire_download.h>

#if defined(LIBACQUIRE_USE_LIBCURL) && LIBACQUIRE_USE_LIBCURL
#include <acquire_libcurl.h>
#elif defined(LIBACQUIRE_USE_WININET) && LIBACQUIRE_USE_WININET
#include <acquire_wininet.h>
#elif defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_LIBFETCH
#include <acquire_libfetch.h>
#else
#error "No networking library named"
#endif /* LIBACQUIRE_USE_LIBCURL */

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

#endif /* !TEST_DOWNLOAD_H */

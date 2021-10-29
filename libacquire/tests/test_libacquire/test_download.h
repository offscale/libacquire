#include <greatest.h>
#include <stdbool.h>

#include <config_for_tests.h>
#include <acquire_config.h>

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)
#include <acquire_openssl.h>
#elif defined(USE_WINCRYPT)
#include <acquire_wincrypt.h>
#endif

#if defined(USE_LIBCURL)
#include <acquire_libcurl.h>
#elif defined(USE_WININET)
#include <acquire_wininet.h>
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

TEST x_test_file_downloads(void) {
    const int download_resp = download(GREATEST_URL, LIBACQUIRE_SHA256,
                                       GREATEST_SHA256, DOWNLOAD_DIR,
                                       false, 0, 0);
    ASSERT_EQ(download_resp, EXIT_SUCCESS);
    ASSERT_FALSE(!sha256(DOWNLOAD_DIR PATH_SEP GREATEST_BASEFILENAME, GREATEST_SHA256));
    ASSERT_FALSE(!sha256(GREATEST_FILE, GREATEST_SHA256));
    PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE (downloads_suite) {
    RUN_TEST(x_test_file_downloads);
}
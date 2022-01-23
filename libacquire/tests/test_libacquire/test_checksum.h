#include <greatest.h>
#include <stdbool.h>

#include <config_for_tests.h>
#include <acquire_config.h>

#define LIBACQUIRE_IMPLEMENTATION

#include <acquire_crc32c.h>

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)
#include <acquire_openssl.h>
#elif defined(USE_WINCRYPT)
#include <acquire_wincrypt.h>
#endif

#ifdef _MSC_VER
#define NUM_FORMAT "%zu"
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define NUM_FORMAT "%d"
#else
#define NUM_FORMAT "%lu"
#endif

TEST x_test_crc32c_should_be_true(void) {
    ASSERT_FALSE(!crc32c(GREATEST_FILE, GREATEST_CRC32C));
    PASS();
}

TEST x_test_sha256_should_be_true(void) {
    ASSERT_FALSE(!sha256(GREATEST_FILE, GREATEST_SHA256));
    PASS();
}

TEST x_test_sha256_file_should_be_false(void) {
    printf("sha256(GREATEST_FILE, \"wrong sha256 sum here\"): "NUM_FORMAT"\n", sha256(GREATEST_FILE, "wrong sha256 sum here"));
    ASSERT_FALSE(sha256(GREATEST_FILE, "wrong sha256 sum here"));
    PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE (checksums_suite) {
    /* RUN_TEST(x_test_crc32c_should_be_true); */
    RUN_TEST(x_test_sha256_should_be_true);
    RUN_TEST(x_test_sha256_file_should_be_false);
}

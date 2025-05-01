#ifndef TEST_CHECKSUM_H
#define TEST_CHECKSUM_H

#include <stdbool.h>

#include <greatest.h>

#include <acquire_common_defs.h>
#include <acquire_config.h>
#include <config_for_tests.h>

#ifdef USE_CRC32C
#include <acquire_crc32c.h>
#elif defined(USE_LIBRHASH)
#include <acquire_librhash.h>
#endif

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)
#include <acquire_openssl.h>
#elif defined(USE_WINCRYPT)
#include <acquire_wincrypt.h>
#endif

#ifndef NUM_FORMAT
#ifdef _MSC_VER
#define NUM_FORMAT "zu"
#define BOOL_FORMAT NUM_FORMAT
typedef size_t num_type;
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define NUM_FORMAT "d"
typedef int num_type;
#else
#define NUM_FORMAT "d"
#define BOOL_FORMAT "lu"
typedef unsigned long num_type;
#endif /* _MSC_VER */
#endif /* !NUM_FORMAT */

TEST x_test_crc32c_should_be_true(void) {
  printf("crc32c(GREATEST_FILE, \"%s\"): %" BOOL_FORMAT "\n", GREATEST_CRC32C,
         crc32c(GREATEST_FILE, GREATEST_CRC32C));
  ASSERT_FALSE(!crc32c(GREATEST_FILE, GREATEST_CRC32C));
  PASS();
}

TEST x_test_sha256_should_be_true(void) {
  ASSERT_FALSE(!sha256(GREATEST_FILE, GREATEST_SHA256));
  PASS();
}

TEST x_test_sha256_file_should_be_false(void) {
  ASSERT_FALSE(sha256(GREATEST_FILE, "wrong sha256 sum here"));
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(checksums_suite) {
  RUN_TEST(x_test_crc32c_should_be_true);
  RUN_TEST(x_test_sha256_should_be_true);
  RUN_TEST(x_test_sha256_file_should_be_false);
}

#endif /* !TEST_CHECKSUM_H */

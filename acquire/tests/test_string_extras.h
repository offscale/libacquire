#ifndef TEST_STRING_EXTRAS_H
#define TEST_STRING_EXTRAS_H

#include <greatest.h>

#include "config_for_tests.h"

#include <acquire_string_extras.h>

static const char *const test_buffer = "hello world";
static const char *const test_target = "hello\0\0\0";

TEST x_strnstr_should_succeed(void) {
  ASSERT_EQ_FMT(test_buffer,
                strnstr(test_buffer, test_target, strlen(test_buffer)), "%s");
  PASS();
}

TEST x_strnstr_should_succeed_with_embedded_null(void) {
  ASSERT_EQ(test_buffer,
            strnstr(test_buffer, test_target, strlen(test_buffer)));
  PASS();
}

TEST x_strnstr_should_fail(void) {
  ASSERT_EQ(NULL, strnstr(test_buffer, "world!", strlen(test_buffer)));
  PASS();
}

TEST test_strcasestr_found(void) {
  const char *haystack = "Hello, World! Welcome to the new WORLD.";
  ASSERT_STR_EQ("World! Welcome to the new WORLD.",
                strcasestr(haystack, "world"));
  ASSERT_STR_EQ("WORLD.", strcasestr(haystack, "WORLD."));
  ASSERT_STR_EQ(haystack,
                strcasestr(haystack, "")); /* Empty string should match start */
  PASS();
}

TEST test_strcasestr_not_found(void) {
  const char *haystack = "Hello, World!";
  ASSERT_EQ(NULL, strcasestr(haystack, "goodbye"));
  ASSERT_EQ(NULL, strcasestr(haystack, "worlds")); /* 's' too many */
  PASS();
}

TEST test_strncasecmp_impl(void) {
  ASSERT_EQ(0, strncasecmp("hello", "HELLO", 5));
  ASSERT_EQ(0, strncasecmp("hello", "HELLO", 10)); /* Test over string length */
  ASSERT_EQ(0, strncasecmp("hello", "HELLo", 5));
  ASSERT_EQ(0, strncasecmp("", "", 5));
  ASSERT_NEQ(0, strncasecmp("hello", "HELLA", 5));
  ASSERT(strncasecmp("apple", "banana", 5) < 0);
  ASSERT(strncasecmp("Banana", "Apple", 5) > 0);
  ASSERT_EQ(0, strncasecmp("Test", "test", 4));
  ASSERT_EQ('i' - 'a', strncasecmp("Testing", "Testa", 5)); /* test n limit */
  PASS();
}

TEST test_strerrorlen_s_impl(void) {
  enum { errnum = 99999 };
  const size_t len = strerrorlen_s(EDOM);
  ASSERT(len > 0);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    char err_str[256];
    const size_t buffer_size = sizeof(err_str);
    const errno_t result = strerror_s(err_str, buffer_size, errnum);
    if (result == 0) {
      printf("Error %d: %s\n", errnum, err_str); // Print the error message
      PASS();
    } else {
      printf("strerror_s failed to get error message for %d\n", errnum);
      FAIL();
    }
  }
#else
  {
    const char *err_str;
    err_str = strerror(errnum);
    if (strstr(err_str, "Unknown error") != NULL) {
      ASSERT_EQ_FMT(strlen(err_str), strerrorlen_s(errnum), "%zu");
      PASS();
    } else {
      FAIL();
    }
  }
#endif
}

/* Suites can group multiple tests with common setup. */
SUITE(string_extras_suite) {
  RUN_TEST(x_strnstr_should_succeed);
  RUN_TEST(x_strnstr_should_succeed_with_embedded_null);
  RUN_TEST(x_strnstr_should_fail);
  RUN_TEST(test_strcasestr_found);
  RUN_TEST(test_strcasestr_not_found);
  RUN_TEST(test_strncasecmp_impl);
  RUN_TEST(test_strerrorlen_s_impl);
}

#endif /* !TEST_STRING_EXTRAS_H */

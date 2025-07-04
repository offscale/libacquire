#ifndef TEST_STRING_EXTRAS_H
#define TEST_STRING_EXTRAS_H

#include <greatest.h>

#include "config_for_tests.h"

#include <acquire_string_extras.h>

static const char *test_buffer = "hello world";
static const char *test_target = "hello\0\0\0";

TEST x_strnstr_should_succeed(void) {
  ASSERT_EQ_FMT(test_buffer,
                strnstr(test_buffer, test_target, strlen(test_buffer)), "%s");
  PASS();
}

TEST x_strnstr_should_fail(void) {
  ASSERT_EQ(strcmp(test_buffer,
                   strnstr(test_buffer, test_target, strlen(test_buffer))),
            0);
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

/* Suites can group multiple tests with common setup. */
SUITE(string_extras_suite) {
  RUN_TEST(x_strnstr_should_succeed);
  RUN_TEST(x_strnstr_should_fail);
  RUN_TEST(test_strcasestr_found);
  RUN_TEST(test_strcasestr_not_found);
}

#endif /* !TEST_STRING_EXTRAS_H */

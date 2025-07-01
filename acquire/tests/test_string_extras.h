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

/* Suites can group multiple tests with common setup. */
SUITE(strnstr_suite) {
  RUN_TEST(x_strnstr_should_succeed);
  RUN_TEST(x_strnstr_should_fail);
}

#endif /* !TEST_STRING_EXTRAS_H */

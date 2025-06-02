#ifndef TEST_STRING_EXTRAS_H
#define TEST_STRING_EXTRAS_H

#include <greatest.h>

#include "config_for_tests.h"

#ifdef HAVE_STRNSTR
#undef HAVE_STRNSTR
#endif /* HAVE_STRNSTR */
#ifndef STRNSTR_IMPL
#define STRNSTR_IMPL
#endif /* !STRNSTR_IMPL */

#include <acquire_string_extras.h>

static const char *buffer = "hello world";
static const char *target = "hello\0\0\0";

TEST x_strnstr_should_succeed(void) {
  ASSERT_EQ_FMT(buffer, strnstr(buffer, target, strlen(buffer)), "%s");
  PASS();
}

TEST x_strnstr_should_fail(void) {
  ASSERT_EQ(strcmp(buffer, strnstr(buffer, target, strlen(buffer))), 0);
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(strnstr_suite) {
  RUN_TEST(x_strnstr_should_succeed);
  RUN_TEST(x_strnstr_should_fail);
}

#endif /* !TEST_STRING_EXTRAS_H */

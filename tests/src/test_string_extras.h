#include <string.h>
#include <greatest.h>
#include <StringExtras.h>
#include <config_for_tests.h>

static const char *buffer = "hello world";
static const char *target = "hello\0\0\0";

TEST x_strnstr_should_succeed(void) {
    const char *substr = strnstr(buffer, target, strlen(buffer));
    ASSERT_EQ_FMT(buffer, substr, "%s");
    PASS();
}

TEST x_strnstr_should_fail(void) {
    const char *substr = strnstr(buffer, target, strlen(buffer));
    ASSERT_EQ(strcmp(buffer, substr), 0);
    PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(strnstr_suite) {
    RUN_TEST(x_strnstr_should_succeed);
    RUN_TEST(x_strnstr_should_fail);
}

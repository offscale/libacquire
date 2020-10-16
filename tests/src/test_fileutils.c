#include <greatest.h>
#include <stdbool.h>
#include <fileutils.h>
#include <config_for_tests.h>

TEST x_is_directory_should_be_true(void) {
    const bool x = is_directory(CMAKE_CURRENT_SOURCE_DIR);
    ASSERT_EQ_FMT((size_t)true, x, "%lu");
    PASS();
}

TEST x_is_directory_should_be_false(void) {
    const bool x  = is_directory(BAD_DIR);
    const bool y = is_directory(CMAKE_CURRENT_LIST_FILE);
    ASSERT_FALSE(x);
    ASSERT_FALSE(y);
    PASS();
}

TEST x_is_file_should_be_true(void) {
    const bool x = is_file(CMAKE_CURRENT_LIST_FILE);
    ASSERT_EQ_FMT((size_t)true, x, "%lu");
    PASS();
}

TEST x_is_file_should_be_false(void) {
    const bool x  = is_file(CMAKE_CURRENT_SOURCE_DIR);
    const bool y  = is_directory(BAD_DIR);
    ASSERT_FALSE(x);
    ASSERT_FALSE(y);
    PASS();
}

TEST x_exists_should_be_true(void) {
    const bool x = exists(CMAKE_CURRENT_SOURCE_DIR);
    const bool y = exists(CMAKE_CURRENT_LIST_FILE);
    ASSERT_EQ_FMT((size_t)true, x, "%lu");
    ASSERT_EQ_FMT((size_t)true, y, "%lu");
    PASS();
}

TEST x_exists_should_be_false(void) {
    const bool x  = exists(BAD_DIR);
    ASSERT_FALSE(x);
    PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(the_suite) {
    RUN_TEST(x_is_directory_should_be_true);
    RUN_TEST(x_is_directory_should_be_false);
    RUN_TEST(x_is_file_should_be_true);
    RUN_TEST(x_is_file_should_be_false);
    RUN_TEST(x_exists_should_be_true);
    RUN_TEST(x_exists_should_be_false);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(the_suite);
    GREATEST_MAIN_END();
}

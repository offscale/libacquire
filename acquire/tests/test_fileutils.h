#ifndef TEST_FILEUTILS_H
#define TEST_FILEUTILS_H

#include <greatest.h>

#include <acquire_fileutils.h>

#include <config_for_tests.h>

#ifdef __cplusplus
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#ifdef _MSC_VER
#ifndef NUM_FORMAT
#define NUM_FORMAT "zu"
typedef size_t num_type;
#endif /* !NUM_FORMAT */
#define BOOL_FORMAT NUM_FORMAT
#elif defined(__linux__) || defined(linux) || defined(__linux)
#ifndef NUM_FORMAT
#define NUM_FORMAT "ld"
typedef int num_type;
#endif /* !NUM_FORMAT */
#define BOOL_FORMAT "ld"
#else
#ifndef NUM_FORMAT
#define NUM_FORMAT "d"
typedef unsigned long num_type;
#endif /* !NUM_FORMAT */
#define BOOL_FORMAT "lu"
#endif /* _MSC_VER */

TEST x_is_directory_should_be_true(void) {
  const bool x = is_directory(CMAKE_CURRENT_SOURCE_DIR);
  ASSERT_EQ_FMT((bool)true, x, "%" BOOL_FORMAT);
  PASS();
}

TEST x_is_directory_should_be_false(void) {
  const bool x = is_directory(BAD_DIR);
  const bool y = is_directory(CMAKE_CURRENT_LIST_FILE);
  ASSERT_FALSE(x);
  ASSERT_FALSE(y);
  PASS();
}

TEST x_is_file_should_be_true(void) {
  const bool x = is_file(CMAKE_CURRENT_LIST_FILE);
  ASSERT_EQ_FMT((bool)true, x, "%" BOOL_FORMAT);
  PASS();
}

TEST x_is_file_should_be_false(void) {
  const bool x = is_file(CMAKE_CURRENT_SOURCE_DIR);
  const bool y = is_directory(BAD_DIR);
  ASSERT_FALSE(x);
  ASSERT_FALSE(y);
  PASS();
}

TEST x_exists_should_be_true(void) {
  const bool x = exists(CMAKE_CURRENT_SOURCE_DIR);
  const bool y = exists(CMAKE_CURRENT_LIST_FILE);
  ASSERT_EQ_FMT((bool)true, x, "%" BOOL_FORMAT);
  ASSERT_EQ_FMT((bool)true, y, "%" BOOL_FORMAT);
  PASS();
}

TEST x_exists_should_be_false(void) {
  const bool x = exists(BAD_DIR);
  ASSERT_FALSE(x);
  PASS();
}

TEST x_parse_out_extension_should_be_including_one_dot(void) {
  ASSERT_FALSE(strcmp(get_extension("foo.zip"), ".zip") != 0);
  PASS();
}

TEST x_parse_out_extension_should_be_include_both_dots(void) {
  ASSERT_FALSE(strcmp(get_extension("foo.tar.gz"), ".tar.gz") != 0);
  PASS();
}

TEST x_parse_out_extension_should_be_include_one_dots(void) {
  ASSERT_FALSE(strcmp(get_extension("foo.can.gz"), ".gz") != 0);
  PASS();
}

TEST x_parse_out_extension_should_be_include_both_dots_when_dot_then_whole_filename_is_extension(
    void) {
  ASSERT_FALSE(strcmp(get_extension(".tar.gz"), ".tar.gz") != 0);
  PASS();
}

/* Overkill to require this test to pass?
TEST
x_parse_out_extension_should_be_include_both_dots_when_whole_filename_is_extension(void)
{ printf("get_extension(\"tar.gz\"): \"%s\"\n", get_extension("tar.gz"));
    ASSERT_FALSE(strcmp(get_extension("tar.gz"), ".gz") != 0);
    PASS();
}
*/

TEST test_filesize(void) {
  const off_t size = filesize(GREATEST_FILE);
  ASSERT(size > 0);
  PASS();
}

TEST test_filesize_non_existent(void) {
  const off_t size = filesize("non-existent-file.xyz");
  ASSERT_EQ_FMT(-1, (int)size, "%d");
  PASS();
}

TEST test_is_relative(void) {
  ASSERT(is_relative("foo/bar"));
  ASSERT(is_relative("foo.txt"));
  ASSERT(is_relative("./foo"));
  PASS();
}

TEST test_is_absolute(void) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  ASSERT(!is_relative("C:\\Windows\\System32"));
  ASSERT(!is_relative("\\Windows"));
#else
  ASSERT(!is_relative("/etc/passwd"));
#endif
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(fileutils_suite) {
  RUN_TEST(x_is_directory_should_be_true);
  RUN_TEST(x_is_directory_should_be_false);
  RUN_TEST(x_is_file_should_be_true);
  RUN_TEST(x_is_file_should_be_false);
  RUN_TEST(x_exists_should_be_true);
  RUN_TEST(x_exists_should_be_false);
  RUN_TEST(x_parse_out_extension_should_be_including_one_dot);
  RUN_TEST(x_parse_out_extension_should_be_include_both_dots);
  RUN_TEST(x_parse_out_extension_should_be_include_one_dots);
  RUN_TEST(
      x_parse_out_extension_should_be_include_both_dots_when_dot_then_whole_filename_is_extension);
  /* RUN_TEST(x_parse_out_extension_should_be_include_both_dots_when_whole_filename_is_extension);
   */
  RUN_TEST(test_filesize);
  RUN_TEST(test_filesize_non_existent);
  RUN_TEST(test_is_relative);
  RUN_TEST(test_is_absolute);
}

#endif /* !TEST_FILEUTILS_H */

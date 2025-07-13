#ifndef TEST_ACQUIRE_URL_UTILS_H
#define TEST_ACQUIRE_URL_UTILS_H

#include <greatest.h>
#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#include "libacquire_export.h"
#endif /* __cplusplus */

#include <acquire.h>
#include <acquire_config.h>
#include <config_for_tests.h>

/* Test get_path_from_url behavior */
TEST test_get_path_valid_url(void) {
  const char *const url = "https://example.com/path/to/file.txt?query=123#frag";
  char *const path = get_path_from_url(url);
  ASSERT(path != NULL);
  ASSERT_STR_EQ("file.txt", path);
  free(path);

  PASS();
}

TEST test_get_path_no_path(void) {
  const char *const url = "https://example.com";
  char *const path = get_path_from_url(url);
  ASSERT(path != NULL);
  ASSERT_STR_EQ("", path);
  free(path);

  PASS();
}

TEST test_get_path_trailing_slash(void) {
  const char *const url = "https://example.com/some/dir/";
  char *const path = get_path_from_url(url);

  ASSERT(path != NULL);
  ASSERT_STR_EQ("", path);
  free(path);

  PASS();
}

TEST test_get_path_no_slash(void) {
  const char *const url = "filename_only";
  char *const path = get_path_from_url(url);
  ASSERT(path != NULL);
  ASSERT_STR_EQ("filename_only", path);
  free(path);

  PASS();
}

TEST test_get_path_empty_string(void) {
  char *const path = get_path_from_url("");
  ASSERT_EQ(NULL, path);
  free(path);
  PASS();
}

TEST test_get_path_null(void) {
  char *const path = get_path_from_url(NULL);
  ASSERT_EQ(NULL, path);
  free(path);
  PASS();
}

/* Test is_url behavior */
TEST test_is_url_http(void) {
  const char *const url = "http://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_https(void) {
  const char *const url = "https://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_ftp(void) {
  const char *const url = "ftp://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_ftps(void) {
  const char *const url = "ftps://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_too_short(void) {
  const char *const url = "http:/";
  ASSERT(!is_url(url));
  PASS();
}

TEST test_is_url_no_scheme(void) {
  const char *const url = "just/a/path/file.txt";
  ASSERT(!is_url(url));
  PASS();
}

TEST test_is_url_case_insensitive(void) {
  ASSERT(is_url("HTTP://EXAMPLE.COM"));
  ASSERT(is_url("Ftps://EXAMPLE.COM"));
  PASS();
}

TEST test_is_url_file(void) {
  ASSERT(is_url("file:///path/to/thing"));
  PASS();
}

SUITE(url_utils_suite) {
  RUN_TEST(test_get_path_valid_url);
  RUN_TEST(test_get_path_no_path);
  RUN_TEST(test_get_path_trailing_slash);
  RUN_TEST(test_get_path_no_slash);
  RUN_TEST(test_get_path_empty_string);
  RUN_TEST(test_get_path_null);

  RUN_TEST(test_is_url_http);
  RUN_TEST(test_is_url_https);
  RUN_TEST(test_is_url_ftp);
  RUN_TEST(test_is_url_ftps);
  RUN_TEST(test_is_url_file);
  RUN_TEST(test_is_url_too_short);
  RUN_TEST(test_is_url_no_scheme);
  RUN_TEST(test_is_url_case_insensitive);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_ACQUIRE_URL_UTILS_H */

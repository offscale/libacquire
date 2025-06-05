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
  const char *url = "https://example.com/path/to/file.txt?query=123#frag";
  const char *path = get_path_from_url(url);
  ASSERT_STR_EQ("file.txt", path);
  PASS();
}

TEST test_get_path_no_slash(void) {
  const char *url = "filename_only";
  const char *path = get_path_from_url(url);
  ASSERT_STR_EQ("filename_only", path);
  PASS();
}

TEST test_get_path_empty_string(void) {
  const char *path = get_path_from_url("");
  ASSERT_EQ(NULL, path);
  PASS();
}

TEST test_get_path_null(void) {
  const char *path = get_path_from_url(NULL);
  ASSERT_EQ(NULL, path);
  PASS();
}

/* Test is_url behavior */
TEST test_is_url_http(void) {
  const char *url = "http://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_https(void) {
  const char *url = "https://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_ftp(void) {
  const char *url = "ftp://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_ftps(void) {
  const char *url = "ftps://example.com";
  ASSERT(is_url(url));
  PASS();
}

TEST test_is_url_too_short(void) {
  const char *url = "http:/";
  ASSERT(!is_url(url));
  PASS();
}

TEST test_is_url_no_scheme(void) {
  const char *url = "just/a/path/file.txt";
  ASSERT(!is_url(url));
  PASS();
}

SUITE(url_utils_suite) {
  RUN_TEST(test_get_path_valid_url);
  RUN_TEST(test_get_path_no_slash);
  RUN_TEST(test_get_path_empty_string);
  RUN_TEST(test_get_path_null);

  RUN_TEST(test_is_url_http);
  RUN_TEST(test_is_url_https);
  RUN_TEST(test_is_url_ftp);
  RUN_TEST(test_is_url_ftps);
  RUN_TEST(test_is_url_too_short);
  RUN_TEST(test_is_url_no_scheme);
}

#endif /* !TEST_ACQUIRE_URL_UTILS_H */

#ifndef TEST_CURL_HELPERS_H
#define TEST_CURL_HELPERS_H

#include "acquire_common_defs.h"
#include <greatest.h>

int get_oname_from_cd(char const *const cd, char *oname);
int get_oname_from_url(char const *url, char *oname);

TEST test_get_oname_from_url_simple(void) {
  char oname[MAX_FILENAME];
  int result = get_oname_from_url("http://example.com/somefile.txt", oname);
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("somefile.txt", oname);
  PASS();
}

TEST test_get_oname_from_url_with_query(void) {
  char oname[MAX_FILENAME];
  /* This implementation is simple and doesn't handle query strings. */
  int result =
      get_oname_from_url("https://site.com/archive.zip?version=1.2", oname);
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("archive.zip?version=1.2", oname);
  PASS();
}

TEST test_get_oname_from_url_no_path(void) {
  char oname[MAX_FILENAME];
  int result = get_oname_from_url("http://example.com", oname);
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("http://example.com", oname);
  PASS();
}

TEST test_get_oname_from_url_trailing_slash(void) {
  char oname[MAX_FILENAME];
  int result = get_oname_from_url("http://example.com/some/path/", oname);
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("", oname);
  PASS();
}

TEST test_get_oname_from_cd_simple(void) {
  char oname[MAX_FILENAME];
  const char *cd = "attachment; filename=simple.dat";
  int result = get_oname_from_cd(cd, oname);
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("simple.dat", oname);
  PASS();
}

TEST test_get_oname_from_cd_case_insensitive(void) {
  char oname[MAX_FILENAME];
  const char *cd = "attachment; FILENAME=case.txt";
  int result = get_oname_from_cd(cd, oname);
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("case.txt", oname);
  PASS();
}

TEST test_get_oname_from_cd_with_other_params(void) {
  char oname[MAX_FILENAME];
  const char *cd = "attachment; filename=ext.zip; charset=UTF-8";
  int result = get_oname_from_cd(cd, oname);
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("ext.zip", oname);
  PASS();
}

TEST test_get_oname_from_cd_no_filename(void) {
  char oname[MAX_FILENAME] = "initial";
  const char *cd = "attachment; something=else";
  int result = get_oname_from_cd(cd, oname);
  ASSERT_EQ(EXIT_FAILURE, result);
  ASSERT_STR_EQ("initial", oname); /* Should not be modified */
  PASS();
}

SUITE(curl_helpers_suite) {
  RUN_TEST(test_get_oname_from_url_simple);
  RUN_TEST(test_get_oname_from_url_with_query);
  RUN_TEST(test_get_oname_from_url_no_path);
  RUN_TEST(test_get_oname_from_url_trailing_slash);

  RUN_TEST(test_get_oname_from_cd_simple);
  RUN_TEST(test_get_oname_from_cd_case_insensitive);
  RUN_TEST(test_get_oname_from_cd_with_other_params);
  RUN_TEST(test_get_oname_from_cd_no_filename);
}

#endif /* !TEST_CURL_HELPERS_H */

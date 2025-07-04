#ifndef TEST_LIBFETCH_H
#define TEST_LIBFETCH_H

#if defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_LIBFETCH

#include "fetch.h"
#include <greatest.h>

/* Forward declare for testing, as it's static in http.c */
char *http_base64(const char *src);

TEST test_libfetch_url_parser(void) {
  struct url *u;

  u = fetchParseURL("http://user:pass@host:8080/path?q=1#f");
  ASSERT(u);
  ASSERT_STR_EQ("http", u->scheme);
  ASSERT_STR_EQ("user", u->user);
  ASSERT_STR_EQ("pass", u->pwd);
  ASSERT_STR_EQ("host", u->host);
  ASSERT_EQ(8080, u->port);
  ASSERT_STR_EQ("/path?q=1#f", u->doc);
  fetchFreeURL(u);

  u = fetchParseURL("ftp://host.com/deep/path/file");
  ASSERT(u);
  ASSERT_STR_EQ("ftp", u->scheme);
  ASSERT_STR_EQ("host.com", u->host);
  ASSERT_STR_EQ("/deep/path/file", u->doc);
  fetchFreeURL(u);

  /* Test malformed URLs */
  u = fetchParseURL("http:://missing-slash.com");
  ASSERT_EQ(NULL, u);
  u = fetchParseURL("http://host:badport/path");
  ASSERT_EQ(NULL, u);

  PASS();
}

TEST test_libfetch_base64_encoder(void) {
  char *res;
  res = http_base64("pleasure.");
  ASSERT_STR_EQ("cGxlYXN1cmUu", res);
  free(res);
  res = http_base64("leasure.");
  ASSERT_STR_EQ("bGVhc3VyZS4=", res);
  free(res);
  res = http_base64("easure.");
  ASSERT_STR_EQ("ZWFzdXJlLg==", res);
  free(res);
  res = http_base64("asure.");
  ASSERT_STR_EQ("YXN1cmUu", res);
  free(res);
  PASS();
}

SUITE(libfetch_suite) {
  RUN_TEST(test_libfetch_url_parser);
  RUN_TEST(test_libfetch_base64_encoder);
}

#endif /* defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_LIBFETCH */

#endif /* TEST_LIBFETCH_H */

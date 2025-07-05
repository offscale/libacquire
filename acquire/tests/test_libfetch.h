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

TEST test_fetchGet_file_nonexistent(void) {
  struct url *u;
  FILE *f;

  u = fetchParseURL("file:///non/existent/path/for/sure");
  ASSERT(u);

  f = fetchGet(u, "");
  ASSERT_EQ(NULL, f);
  ASSERT_EQ_FMT(FETCH_UNAVAIL, fetchLastErrCode, "%d");

  fetchFreeURL(u);
  PASS();
}

/* === FILE.C TESTS === */
TEST test_file_c_get(void) {
  struct url *u = fetchParseURL("file://" GREATEST_FILE);
  struct url_stat st;
  FILE *f;
  ASSERT(u);
  f = fetchXGetFile(u, &st, "");
  ASSERT(f);
  ASSERT(st.size > 0);
  fclose(f);
  fetchFreeURL(u);
  PASS();
}

TEST test_file_c_put(void) {
  char test_path[PATH_MAX];
  struct url *u;
  FILE *f;
  snprintf(test_path, sizeof(test_path), "%s%sput_test.txt", DOWNLOAD_DIR,
           PATH_SEP);

  u = fetchMakeURL("file", NULL, 0, test_path, NULL, NULL);
  ASSERT(u);
  f = fetchPutFile(u, "");
  ASSERT(f);
  fprintf(f, "hello");
  fclose(f);
  ASSERT(is_file(test_path));
  remove(test_path);
  fetchFreeURL(u);
  PASS();
}

SUITE(libfetch_file_suite) {
  RUN_TEST(test_file_c_get);
  RUN_TEST(test_file_c_put);
}

/* === COMMON.C TESTS === */
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) &&              \
    !defined(__NT__)
TEST test_common_no_proxy_match(void) {
  setenv("no_proxy", "*.example.com, .google.com, other.org", 1);
  ASSERT(fetch_no_proxy_match("host.example.com"));
  ASSERT(fetch_no_proxy_match("sub.host.example.com"));
  ASSERT_FALSE(fetch_no_proxy_match("host.example.org"));
  ASSERT(fetch_no_proxy_match("www.google.com"));
  ASSERT_FALSE(fetch_no_proxy_match("google.com.bad"));
  ASSERT(fetch_no_proxy_match("other.org"));

  setenv("no_proxy", "*", 1);
  ASSERT(fetch_no_proxy_match("anything.com"));

  unsetenv("no_proxy");
  PASS();
}

TEST test_common_netrc_auth(void) {
  char netrc_path[PATH_MAX];
  FILE *f;
  struct url *u;

  snprintf(netrc_path, sizeof(netrc_path), "%s%s_test.netrc", DOWNLOAD_DIR,
           PATH_SEP);
  f = fopen(netrc_path, "w");
  ASSERT(f);
  fprintf(f, "machine test-host.com login myuser password mypass\n");
  fclose(f);
  setenv("NETRC", netrc_path, 1);
  u = fetchParseURL("ftp://test-host.com/file");
  ASSERT(u);
  ASSERT_EQ(0, fetch_netrc_auth(u));
  ASSERT_STR_EQ("myuser", u->user);
  ASSERT_STR_EQ("mypass", u->pwd);
  fetchFreeURL(u);
  unsetenv("NETRC");
  remove(netrc_path);
  PASS();
}

#endif /* !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) &&        \
          !defined(__NT__)) */

TEST test_fetchGet_http(void) {
  struct url *u;
  FILE *f;
  char buf[10];

  u = fetchParseURL("http://httpbin.org/get");
  ASSERT(u);

  f = fetchGet(u, "");
  if (f == NULL) {
    if (fetchLastErrCode == FETCH_NETWORK || fetchLastErrCode == FETCH_DOWN ||
        fetchLastErrCode == FETCH_TEMP || fetchLastErrCode == FETCH_RESOLV) {
      fetchFreeURL(u);
      SKIPm("Network may be down, skipping test_fetchGet_http.");
    }
    FAILm("fetchGet failed");
  }

  ASSERT(f);
  ASSERT(fread(buf, 1, sizeof(buf), f) > 0);

  fclose(f);
  fetchFreeURL(u);

  PASS();
}

SUITE(libfetch_suite) {
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) &&              \
    !defined(__NT__)
  RUN_TEST(test_common_no_proxy_match);
  RUN_TEST(test_common_netrc_auth);
#endif /* !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) &&        \
          !defined(__NT__) */
  RUN_TEST(test_libfetch_url_parser);
  RUN_TEST(test_libfetch_base64_encoder);
  RUN_TEST(test_fetchGet_http);
  RUN_TEST(test_fetchGet_file_nonexistent);
}

#endif /* defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_LIBFETCH */

#endif /* TEST_LIBFETCH_H */

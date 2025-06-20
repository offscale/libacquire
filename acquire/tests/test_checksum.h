#ifndef TEST_CHECKSUM_H
#define TEST_CHECKSUM_H

#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#include <greatest.h>

#include <acquire_common_defs.h>
#include <acquire_config.h>
#include <config_for_tests.h>

#if defined(LIBACQUIRE_USE_CRC32C) && LIBACQUIRE_USE_CRC32C
#include <acquire_crc32c.h>
#elif defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH
#include <acquire_librhash.h>
#endif

#if (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO) || \
    (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL)
#include <acquire_openssl.h>
#elif defined(LIBACQUIRE_USE_WINCRYPT) && LIBACQUIRE_USE_WINCRYPT
#include <acquire_wincrypt.h>
#endif

#ifdef _MSC_VER
#ifndef NUM_FORMAT
#define NUM_FORMAT "zu"
typedef size_t num_type;
#endif /* !NUM_FORMAT */
#define BOOL_FORMAT NUM_FORMAT
#elif defined(__linux__) || defined(linux) || defined(__linux)
#ifndef NUM_FORMAT
#define NUM_FORMAT "d"
typedef int num_type;
#endif /* !NUM_FORMAT */
#define BOOL_FORMAT "d"
#else
#ifndef NUM_FORMAT
#define NUM_FORMAT "d"
typedef unsigned long num_type;
#endif /* !NUM_FORMAT */
#define BOOL_FORMAT "lu"
#endif /* _MSC_VER */

TEST x_test_crc32c_should_be_true(void) {
  printf("crc32c(GREATEST_FILE, \"%s\"): %" BOOL_FORMAT "\n", GREATEST_CRC32C,
         crc32c(GREATEST_FILE, GREATEST_CRC32C));
  ASSERT(crc32c(GREATEST_FILE, GREATEST_CRC32C));
  PASS();
}

TEST x_test_sha256_should_be_true(void) {
  ASSERT_FALSE(!sha256(GREATEST_FILE, GREATEST_SHA256));
  PASS();
}

TEST x_test_sha256_file_should_be_false(void) {
  ASSERT_FALSE(sha256(GREATEST_FILE, "wrong sha256 sum here"));
  PASS();
}

/**
 * @brief Create a temporary file with the specified content.
 *
 * On Unix-like systems uses mkstemp for safe temp file creation.
 * On Windows uses _mktemp_s for generating a unique temp filename.
 *
 * @param content Null-terminated string content to write into the file.
 * @return Dynamically allocated filename string on success (caller must free),
 *         NULL on failure.
 */
static char *create_temp_file_with_content(const char *const content) {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  char tmp_path[MAX_PATH];
  char tmp_file[MAX_PATH];
  DWORD path_len;
  if (!content)
    return NULL;
  path_len = GetTempPathA(MAX_PATH, tmp_path);
  if (path_len == 0 || path_len > MAX_PATH) {
    fprintf(stderr, "GetTempPathA failed\n");
    return NULL;
  }

  /* Generate template for _mktemp_s */
  snprintf(tmp_file, MAX_PATH, "%s%sXXXXXX", tmp_path, "tmp");

  if (_mktemp_s(tmp_file, MAX_PATH) != 0) {
    fprintf(stderr, "_mktemp_s failed\n");
    return NULL;
  }

  FILE *f;
  {
    const errno_t err = fopen_s(&f, tmp_file, "w");
    ;
    if (err != 0 || f == NULL) {
      fprintf(stderr, "Failed to open %s for reading\n", tmp_file);
      free(f);
      return NULL;
    }
  }

  if (fputs(content, f) < 0) {
    perror("fputs");
    fclose(f);
    remove(tmp_file);
    return NULL;
  }

  if (fclose(f) != 0) {
    perror("fclose");
    remove(tmp_file);
    return NULL;
  }

  return _strdup(tmp_file);

#else
  /* Unix-like systems */

  /* Template for mkstemp, needs to be modifiable string */
  char template[] = "/tmp/libacquireXXXXXX";
  int fd;
  size_t len;
  ssize_t wr;
  if (!content)
    return NULL;
  fd = mkstemp(template);
  if (fd == -1) {
    perror("mkstemp");
    return NULL;
  }

  /* Write content */
  len = strlen(content);
  wr = write(fd, content, len);
  if (wr < 0 || (size_t)wr != len) {
    perror("write");
    close(fd);
    unlink(template);
    return NULL;
  }

  if (close(fd) == -1) {
    perror("close");
    unlink(template);
    return NULL;
  }

  /* Return dynamically allocated string to filename */
  return strdup(template);
#endif
}

/* Helper function to remove a given file safely */
static void safe_remove_file(const char *const filename) {
  if (filename) {
    remove(filename);
    free((void *)filename);
  }
}

TEST test_crc32c_valid(void) {
  const bool res = crc32c(GREATEST_FILE, GREATEST_CRC32C);
  ASSERT(res);
  PASS();
}

TEST test_crc32c_invalid_hash(void) {
  const bool res = crc32c(GREATEST_FILE, "deadbeef");
  ASSERT_FALSE(res);
  PASS();
}

TEST test_crc32c_wrong_file(void) {
  const bool res = crc32c("nonexistent_file.xyz", GREATEST_CRC32C);
  ASSERT_FALSE(res);
  PASS();
}

TEST test_sha256_valid(void) {
  const bool res = sha256(GREATEST_FILE, GREATEST_SHA256);
  ASSERT(res);
  PASS();
}

TEST test_sha256_invalid_hash(void) {
  const bool res = sha256(
      GREATEST_FILE,
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");
  ASSERT_FALSE(res);
  PASS();
}

TEST test_sha256_wrong_file(void) {
  const bool res = sha256("nonexistent_file.xyz", GREATEST_SHA256);
  ASSERT_FALSE(res);
  PASS();
}

TEST test_string2checksum_known(void) {
  ASSERT_EQ(LIBACQUIRE_CRC32C, string2checksum("crc32c"));
  ASSERT_EQ(LIBACQUIRE_CRC32C, string2checksum("CRC32C"));
  ASSERT_EQ(LIBACQUIRE_SHA256, string2checksum("sha256"));
  ASSERT_EQ(LIBACQUIRE_SHA256, string2checksum("SHA256"));
  ASSERT_EQ(LIBACQUIRE_SHA512, string2checksum("sha512"));
  ASSERT_EQ(LIBACQUIRE_SHA512, string2checksum("SHA512"));
  PASS();
}

TEST test_string2checksum_unknown(void) {
  ASSERT_EQ(LIBACQUIRE_UNSUPPORTED_CHECKSUM, string2checksum("unknown"));
  ASSERT_EQ(LIBACQUIRE_UNSUPPORTED_CHECKSUM, string2checksum(""));
  ASSERT_EQ(LIBACQUIRE_UNSUPPORTED_CHECKSUM, string2checksum(NULL));
  PASS();
}

TEST test_get_checksum_function_known(void) {
  bool (*func)(const char *, const char *);
  func = get_checksum_function(LIBACQUIRE_CRC32C);
  ASSERT(func != NULL);
  func = get_checksum_function(LIBACQUIRE_SHA256);
  ASSERT(func != NULL);
  func = get_checksum_function(LIBACQUIRE_SHA512);
  ASSERT(func != NULL);
  PASS();
}

TEST test_get_checksum_function_unsupported(void) {
  bool (*func)(const char *, const char *);
  func = get_checksum_function(LIBACQUIRE_UNSUPPORTED_CHECKSUM);
  ASSERT_EQ(NULL, func);
  PASS();
}

/* Test with a small temporary file and known precomputed hashes */

TEST test_crc32c_small_temp_file(void) {
  const char *const content = "hello, world\n";
  char *const tmpfile = create_temp_file_with_content(content);
  /* Precomputed CRC32C checksum of "hello, world\n" is: 0xb8ca70d7 (lowercase
   * hex) */
  const bool result = crc32c(tmpfile, "8f00b46e");
  /* f4247453 */
  ASSERT(tmpfile != NULL);
  safe_remove_file(tmpfile);
  ASSERT(result);
  PASS();
}

TEST test_sha256_small_temp_file(void) {
  const char *content = "hello, world\n";
  char *const tmpfile = create_temp_file_with_content(content);
  /* Precomputed SHA256 of "hello, world\n" */
  const char *const expected_sha256 =
      "bbbe3b671e853dfe30a0e60594366f24f02f31ea24ff4651743fd60c73cd6822";
  /* 853ff93762a06ddbf722c4ebe9ddd66d8f63ddaea97f521c3ecc20da7c976020 */
  const bool result = sha256(tmpfile, expected_sha256);
  ASSERT(tmpfile != NULL);
  safe_remove_file(tmpfile);
  ASSERT(result);
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(checksums_suite) {
  RUN_TEST(x_test_crc32c_should_be_true);
  RUN_TEST(x_test_sha256_should_be_true);
  RUN_TEST(x_test_sha256_file_should_be_false);

  RUN_TEST(test_crc32c_valid);
  RUN_TEST(test_crc32c_invalid_hash);
  RUN_TEST(test_crc32c_wrong_file);

  RUN_TEST(test_sha256_valid);
  RUN_TEST(test_sha256_invalid_hash);
  RUN_TEST(test_sha256_wrong_file);

  RUN_TEST(test_string2checksum_known);
  RUN_TEST(test_string2checksum_unknown);

  RUN_TEST(test_get_checksum_function_known);
  RUN_TEST(test_get_checksum_function_unsupported);

  RUN_TEST(test_crc32c_small_temp_file);
  RUN_TEST(test_sha256_small_temp_file);
}

#endif /* !TEST_CHECKSUM_H */

#ifndef TEST_CLI_H
#define TEST_CLI_H

#include "acquire_config.h"
#include "cli.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper to create a mutable argv array for tests */
static char **create_argv(const char *const *src, int count) {
  int i;
  size_t total_len = 0;
  char **argv = (char **)malloc(sizeof(char *) * (count + 1));
  char *data;
  for (i = 0; i < count; i++) {
    total_len += strlen(src[i]) + 1;
  }
  data = (char *)malloc(total_len);
  for (i = 0; i < count; i++) {
    strcpy(data, src[i]);
    argv[i] = data;
    data += strlen(src[i]) + 1;
  }
  argv[count] = NULL;
  return argv;
}

static void free_argv(char **argv) {
  if (argv != NULL) {
    if (argv[0] != NULL) {
      free(argv[0]); /* Free the contiguous block */
    }
    free(argv); /* Free the array of pointers */
  }
}

TEST test_cli_parsing_download_and_verify(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "--directory=./tmp", "--hash=HASH",
                                  "--checksum=sha256",
                                  "http://example.com/file.zip"};
  int argc = 5;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ(0, result);
  ASSERT_STR_EQ("./tmp", args.directory);
  ASSERT_STR_EQ("HASH", args.hash);
  ASSERT_STR_EQ("sha256", args.checksum);
  ASSERT_STR_EQ("http://example.com/file.zip", args.url);
  ASSERT_FALSE(args.check);

  free_argv(argv);
  PASS();
}

TEST test_cli_parsing_check(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire",           "--check",
                                  "--directory=.",     "--hash=H",
                                  "--checksum=crc32c", "url"};
  int argc = 6;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ(0, result);
  ASSERT_EQ_FMT(1, (int)args.check, "%d");
  ASSERT_STR_EQ(".", args.directory);
  ASSERT_STR_EQ("H", args.hash);
  ASSERT_STR_EQ("crc32c", args.checksum);
  ASSERT_STR_EQ("url", args.url);

  free_argv(argv);
  PASS();
}

TEST test_cli_parsing_output_file(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "--output=file.out",
                                  "http://a.com"};
  int argc = 3;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ(0, result);
  ASSERT_STR_EQ("file.out", args.output);
  ASSERT_STR_EQ("http://a.com", args.url);

  free_argv(argv);
  PASS();
}

TEST test_cli_parsing_help(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "--help"};
  int argc = 2;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ(1, result); /* help exits with non-zero */
  ASSERT(args.help);

  free_argv(argv);
  PASS();
}

TEST test_cli_missing_required_arg_value(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "--directory"}; /* Missing value */
  int argc = 2;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ_FMT(1, result, "%d");

  free_argv(argv);
  PASS();
}

TEST test_cli_no_args(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire"};
  int argc = 1;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ(EXIT_FAILURE, result);

  free_argv(argv);
  PASS();
}

SUITE(cli_suite) {
  RUN_TEST(test_cli_parsing_download_and_verify);
  RUN_TEST(test_cli_parsing_check);
  RUN_TEST(test_cli_parsing_output_file);
  RUN_TEST(test_cli_parsing_help);
  RUN_TEST(test_cli_missing_required_arg_value);
  RUN_TEST(test_cli_no_args);
}

#endif /* !TEST_CLI_H */

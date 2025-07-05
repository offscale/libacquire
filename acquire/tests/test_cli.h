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
  const char *const src_argv[] = {"acquire", "-h"};
  int argc = 2;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ(1, result); /* help exits with non-zero */
  ASSERT(args.help);

  free_argv(argv);
  PASS();
}

TEST test_cli_parsing_version(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "--version"};
  int argc = 2;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version-1.2.3");

  ASSERT_EQ(EXIT_FAILURE, result); /* version exits */
  ASSERT(args.version);
  free_argv(argv);

  /* Test with NULL version string */
  argv = create_argv(src_argv, argc);
  result = docopt(&args, argc, argv, 0, NULL);
  ASSERT_EQ(EXIT_FAILURE, result);
  ASSERT(args.version);

  free_argv(argv);
  PASS();
}

TEST test_cli_parsing_short_options_and_terminator(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "-d", "./tmp",         "-o",
                                  "out.zip", "--", "http://url.com"};
  int argc = 7;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "v1");

  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("./tmp", args.directory);
  ASSERT_STR_EQ("out.zip", args.output);
  ASSERT_STR_EQ("http://url.com", args.url);

  free_argv(argv);
  PASS();
}

TEST test_cli_parsing_unknown_option(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "--nonexistent-option"};
  int argc = 2;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "v1");

  ASSERT_EQ(EXIT_FAILURE, result);

  free_argv(argv);
  PASS();
}

TEST test_cli_missing_required_arg_value(void) {
  struct DocoptArgs args;
  char **argv;

  const char *const dir_argv[] = {"acquire", "--directory"};
  argv = create_argv(dir_argv, 2);
  ASSERT_EQ(EXIT_FAILURE, docopt(&args, 2, argv, 0, "v"));
  free_argv(argv);

  {
    const char *const hash_argv[] = {"acquire", "--hash"};
    argv = create_argv(hash_argv, 2);
    ASSERT_EQ(EXIT_FAILURE, docopt(&args, 2, argv, 0, "v"));
    free_argv(argv);
  }

  {
    const char *const checksum_argv[] = {"acquire", "--checksum"};
    argv = create_argv(checksum_argv, 2);
    ASSERT_EQ(EXIT_FAILURE, docopt(&args, 2, argv, 0, "v"));
    free_argv(argv);
  }

  {
    const char *const output_argv[] = {"acquire", "-o"};
    argv = create_argv(output_argv, 2);
    ASSERT_EQ(EXIT_FAILURE, docopt(&args, 2, argv, 0, "v"));
    free_argv(argv);
  }

  PASS();
}

TEST test_cli_no_args_with_help_off(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire"};
  int argc = 1;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 0, "test-version");

  ASSERT_EQ(EXIT_FAILURE, result);

  free_argv(argv);
  PASS();
}

TEST test_cli_no_args_with_help_on(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire"};
  int argc = 1;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, argc, argv, 1, "test-version");

  ASSERT_EQ(EXIT_FAILURE, result);

  free_argv(argv);
  PASS();
}

TEST test_cli_parsing_terminator_cases(void) {
  struct DocoptArgs args;
  char **argv;
  int result;

  const char *const src_argv1[] = {"acquire", "--"};
  argv = create_argv(src_argv1, 2);
  result = docopt(&args, 2, argv, 0, "v1");
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_EQ(NULL, args.url);
  free_argv(argv);

  {
    const char *const src_argv2[] = {"acquire", "url1", "--", "url2"};
    argv = create_argv(src_argv2, 4);
    result = docopt(&args, 4, argv, 0, "v1");
    ASSERT_EQ(EXIT_SUCCESS, result);
    ASSERT_STR_EQ("url1", args.url);
    free_argv(argv);
  }

  PASS();
}

TEST test_cli_multiple_positional_args(void) {
  struct DocoptArgs args;
  const char *const src_argv[] = {"acquire", "url1", "url2"};
  int argc = 3;
  char **argv = create_argv(src_argv, argc);
  int result = docopt(&args, 3, argv, 0, "v1");
  ASSERT_EQ(EXIT_SUCCESS, result);
  ASSERT_STR_EQ("url1", args.url); /* only first one is taken */
  free_argv(argv);
  PASS();
}

SUITE(cli_suite) {
  RUN_TEST(test_cli_parsing_download_and_verify);
  RUN_TEST(test_cli_parsing_check);
  RUN_TEST(test_cli_parsing_output_file);
  RUN_TEST(test_cli_parsing_help);
  RUN_TEST(test_cli_missing_required_arg_value);
  RUN_TEST(test_cli_no_args_with_help_off);
  RUN_TEST(test_cli_no_args_with_help_on);
  RUN_TEST(test_cli_parsing_version);
  RUN_TEST(test_cli_parsing_short_options_and_terminator);
  RUN_TEST(test_cli_parsing_unknown_option);
  RUN_TEST(test_cli_parsing_terminator_cases);
  RUN_TEST(test_cli_multiple_positional_args);
}

#endif /* !TEST_CLI_H */

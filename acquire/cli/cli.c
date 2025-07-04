#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"

enum { HELP_MSG_COUNT = 17 };

int docopt(struct DocoptArgs *args, int argc, char *argv[], const bool help,
           const char *version) {
  int i;
  int url_is_set = 0;

  const char *help_message[HELP_MSG_COUNT] = {
      "acquire: The core for your package manager, minus the dependency graph "
      "components. Download, verify, and extract.",
      "",
      "Usage:",
      "  acquire --check --directory=<d> --hash=<h> --checksum=<sha> <url>...",
      "  acquire --directory=<d> --hash=<h> --checksum=<sha> <url>...",
      "  acquire --output=<f> <url>...",
      "  acquire --help",
      "  acquire --version",
      "",
      "Options:",
      "  -h --help               Show this screen.",
      "  --version               Show version.",
      "  --check                 Check if already downloaded.",
      "  --hash=<h>              Hash to verify.",
      "  --checksum=<sha>        Checksum algorithm, e.g., SHA256 or SHA512.",
      "  -d=<d>, --directory=<d> Location to download files to.",
      "  -o=<f>, --output=<f>    Output file. If not specified, will derive "
      "from URL."};

  /* Initialize arguments to default values */
  memset(args, 0, sizeof(struct DocoptArgs));
  memcpy((void *)args->help_message, (void *)help_message,
         sizeof(help_message));

  if (argc <= 1) {
    if (help) {
      for (i = 0; i < HELP_MSG_COUNT; i++)
        puts(args->help_message[i]);
    }
    return EXIT_FAILURE;
  }

  for (i = 1; i < argc; i++) {
    const char *arg = argv[i];

    if (strcmp(arg, "--") == 0) {
      /* All following arguments are positional */
      i++;
      if (i < argc && !url_is_set) {
        args->url = argv[i];
        url_is_set = 1;
      }
      break; /* Stop parsing options */
    }

    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      args->help = 1;
      if (help) {
        for (i = 0; i < HELP_MSG_COUNT; ++i)
          puts(args->help_message[i]);
      }
      return EXIT_FAILURE;
    }

    if (strcmp(arg, "--version") == 0) {
      args->version = 1;
      if (version) {
        puts(version);
      }
      return EXIT_FAILURE;
    }

    if (strcmp(arg, "--check") == 0) {
      args->check = 1;
      continue;
    }

    /* Options with arguments */
    if (strncmp(arg, "--directory=", 12) == 0) {
      args->directory = (char *)(arg + 12);
    } else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--directory") == 0) {
      if (i + 1 < argc) {
        args->directory = argv[++i];
      } else {
        fprintf(stderr, "Option %s requires an argument.\n", arg);
        return EXIT_FAILURE;
      }
    } else if (strncmp(arg, "--hash=", 7) == 0) {
      args->hash = (char *)(arg + 7);
    } else if (strcmp(arg, "--hash") == 0) {
      if (i + 1 < argc) {
        args->hash = argv[++i];
      } else {
        fprintf(stderr, "Option %s requires an argument.\n", arg);
        return EXIT_FAILURE;
      }
    } else if (strncmp(arg, "--checksum=", 11) == 0) {
      args->checksum = (char *)(arg + 11);
    } else if (strcmp(arg, "--checksum") == 0) {
      if (i + 1 < argc) {
        args->checksum = argv[++i];
      } else {
        fprintf(stderr, "Option %s requires an argument.\n", arg);
        return EXIT_FAILURE;
      }
    } else if (strncmp(arg, "--output=", 9) == 0) {
      args->output = (char *)(arg + 9);
    } else if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0) {
      if (i + 1 < argc) {
        args->output = argv[++i];
      } else {
        fprintf(stderr, "Option %s requires an argument.\n", arg);
        return EXIT_FAILURE;
      }
    } else if (arg[0] == '-') {
      fprintf(stderr, "Unknown option: %s\n", arg);
      return EXIT_FAILURE;
    } else {
      /* Positional argument */
      if (!url_is_set) {
        args->url = (char *)arg;
        url_is_set = 1;
      }
      /* Note: This simple parser only handles one URL. */
    }
  }

  return EXIT_SUCCESS;
}

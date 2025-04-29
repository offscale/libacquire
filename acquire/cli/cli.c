#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"

struct Command {
  const char *name;
  int value; /* Use int for bool in C89 */
};

struct Argument {
  const char *name;
  const char *value;
  const char *array[ARG_MAX]; /* If supporting multi URLs, but the actual list
                                 is unused in your code */
};

struct Option {
  const char *oshort;
  const char *olong;
  int argcount; /* bool stored as int */
  int value;    /* bool stored as int */
  const char *argument;
};

struct Elements {
  int n_commands;
  int n_arguments;
  int n_options;
  struct Command *commands;
  struct Argument *arguments;
  struct Option *options;
};

/* Tokens object */
struct Tokens {
  int argc;
  char **argv;
  int i;
  char *current;
};

const char usage_pattern[] =
    "Usage:\n"
    "  acquire --check --directory=<d> --hash=<h> --checksum=<sha> <url>...\n"
    "  acquire --directory=<d> --hash=<h> --checksum=<sha> <url>...\n"
    "  acquire --output=<f> <url>...\n"
    "  acquire --help\n"
    "  acquire --version";

struct Tokens tokens_new(int argc, char **argv) {
  struct Tokens ts;
  ts.argc = argc;
  ts.argv = argv;
  ts.i = 0;
  ts.current = (argc > 0) ? argv[0] : NULL;
  return ts;
}

struct Tokens *tokens_move(struct Tokens *ts) {
  if (ts->i < ts->argc)
    ts->current = ts->argv[++ts->i];
  else
    ts->current = NULL;
  return ts;
}

/*
 * Implement parsing of remaining arguments after '--' as positional (not yet
 * supported formally)
 */
int parse_doubledash(struct Tokens *ts, struct Elements *elements) {
  /* Skip the '--' token */
  tokens_move(ts);
  while (ts->current != NULL) {
    /* Add remaining tokens as arguments with no name (or name NULL) */
    /* For simplicity: set first argument's value to current */
    if (elements->n_arguments > 0) {
      int idx = elements->n_arguments - 1;
      elements->arguments[idx].value = ts->current;
    }
    tokens_move(ts);
  }
  return 0;
}

/* Modified parse_args to not break on space-separated URLs */
int parse_args(struct Tokens *ts, struct Elements *elements);

/* parse_long, parse_shorts, parse_argcmd are left essentially unchanged,
 * except for replacing bool with int and ensuring consistent return codes
 */

int parse_long(struct Tokens *ts, struct Elements *elements) {
  int i;
  size_t len_prefix;
  int n_options = elements->n_options;
  struct Option *option;
  struct Option *options = elements->options;
  char *eq = strchr(ts->current, '=');

  if (eq == NULL)
    len_prefix = strlen(ts->current);
  else
    len_prefix = (size_t)(eq - ts->current);

  for (i = 0; i < n_options; i++) {
    option = &options[i];
    if (strncmp(ts->current, option->olong, len_prefix) == 0)
      break;
  }
  if (i == n_options) {
    fprintf(stderr, "%s is not recognized\n", ts->current);
    return 1;
  }
  tokens_move(ts);
  if (option->argcount) {
    if (eq == NULL) {
      if (ts->current == NULL) {
        fprintf(stderr, "%s requires argument\n", option->olong);
        return 1;
      }
      option->argument = ts->current;
      tokens_move(ts);
    } else {
      option->argument = eq + 1;
    }
  } else {
    if (eq != NULL) {
      fprintf(stderr, "%s must not have an argument\n", option->olong);
      return 1;
    }
    option->value = 1;
  }
  return 0;
}

int parse_shorts(struct Tokens *ts, struct Elements *elements) {
  char *raw;
  int i;
  int n_options = elements->n_options;
  struct Option *option;
  struct Option *options = elements->options;

  raw = &ts->current[1];
  tokens_move(ts);
  while (*raw) {
    for (i = 0; i < n_options; i++) {
      option = &options[i];
      if (option->oshort != NULL && option->oshort[1] == *raw)
        break;
    }
    if (i == n_options) {
      fprintf(stderr, "-%c is not recognized\n", *raw);
      return 1;
    }
    raw++;
    if (!option->argcount) {
      option->value = 1;
    } else {
      if (*raw == '\0') {
        if (ts->current == NULL) {
          fprintf(stderr, "%s requires argument\n", option->oshort);
          return 1;
        }
        raw = ts->current;
        tokens_move(ts);
      }
      option->argument = raw;
      break;
    }
  }
  return 0;
}

int parse_argcmd(struct Tokens *ts, struct Elements *elements) {
  int i;
  int n_commands = elements->n_commands;
  struct Command *command;
  struct Command *commands = elements->commands;

  for (i = 0; i < n_commands; i++) {
    command = &commands[i];
    if (strcmp(command->name, ts->current) == 0) {
      command->value = 1;
      tokens_move(ts);
      return 0;
    }
  }
  /* Accept as argument */
  if (elements->n_arguments > 0) {
    /* For simplicity: store last argument in last element */
    int idx = elements->n_arguments - 1;
    elements->arguments[idx].value = ts->current;
  }
  tokens_move(ts);
  return 0;
}

int parse_args(struct Tokens *ts, struct Elements *elements) {
  int ret = 0;

  while (ts->current != NULL) {
    if (strcmp(ts->current, "--") == 0) {
      ret = parse_doubledash(ts, elements);
      if (ret != 0)
        break;
    } else if (ts->current[0] == '-' && ts->current[1] == '-') {
      ret = parse_long(ts, elements);
      if (ret != 0)
        break;
    } else if (ts->current[0] == '-' && ts->current[1] != '\0') {
      ret = parse_shorts(ts, elements);
      if (ret != 0)
        break;
    } else {
      ret = parse_argcmd(ts, elements);
      if (ret != 0)
        break;
    }
  }
  return ret;
}

int elems_to_args(struct Elements *elements, struct DocoptArgs *args,
                  const int help, const char *version) {
  struct Command *command;
  struct Argument *argument;
  struct Option *option;
  int i, j;

  /* fix gcc-related compiler warnings (unused) */
  (void)command;
  (void)argument;

  /* options */
  for (i = 0; i < elements->n_options; i++) {
    option = &elements->options[i];
    if (help && option->value && strcmp(option->olong, "--help") == 0) {
      for (j = 0; j < 17; j++)
        puts(args->help_message[j]);
      return 1; /* exit failure */
    } else if (version && option->value &&
               strcmp(option->olong, "--version") == 0) {
      puts(version);
      return 1; /* exit failure */
    } else if (strcmp(option->olong, "--check") == 0) {
      args->check = (size_t)option->value;
    } else if (strcmp(option->olong, "--help") == 0) {
      args->help = (size_t)option->value;
    } else if (strcmp(option->olong, "--version") == 0) {
      args->version = (size_t)option->value;
    } else if (strcmp(option->olong, "--checksum") == 0) {
      if (option->argument) {
        args->checksum = (char *)option->argument;
      }
    } else if (strcmp(option->olong, "--directory") == 0) {
      if (option->argument) {
        args->directory = (char *)option->argument;
      }
    } else if (strcmp(option->olong, "--hash") == 0) {
      if (option->argument) {
        args->hash = (char *)option->argument;
      }
    } else if (strcmp(option->olong, "--output") == 0) {
      if (option->argument) {
        args->output = (char *)option->argument;
      }
    }
  }
  /* commands */
  for (i = 0; i < elements->n_commands; i++) {
    command = &elements->commands[i];
  }
  /* arguments */
  /* Note: only last <url> stored */
  for (i = 0; i < elements->n_arguments; i++) {
    argument = &elements->arguments[i];

    if (strcmp(argument->name, "<url>") == 0) {
      args->url = (char *)argument->value;
    }
  }
  return 0;
}

/*
 * Main docopt function
 */
struct DocoptArgs docopt(int argc, char *argv[], const int help,
                         const char *version) {
  struct DocoptArgs args = {
      NULL,
      0,
      0,
      0,
      NULL,
      NULL,
      NULL,
      NULL,
      usage_pattern,
      {"acquire: The core for your package manager, minus the dependency graph "
       "components. Download, verify, and extract.",
       "", "Usage:",
       "  acquire --check --directory=<d> --hash=<h> --checksum=<sha> <url>...",
       "  acquire --directory=<d> --hash=<h> --checksum=<sha> <url>...",
       "  acquire --output=<f> <url>...", "  acquire --help",
       "  acquire --version", "",
       "Options:", "  -h --help               Show this screen.",
       "  --version               Show version.",
       "  --check                 Check if already downloaded.",
       "  --hash=<h>              Hash to verify.",
       "  --checksum=<sha>        Checksum algorithm, e.g., SHA256 or SHA512.",
       "  -d=<d>, --directory=<d> Location to download files to.",
       "  -o=<f>, --output=<f>    Output file. If not specified, will derive "
       "from URL."}};
  struct Command commands[1];
  struct Argument arguments[] = {{"<url>", NULL, {NULL}}};
  struct Option options[] = {
      {NULL, "--check", 0, 0, NULL},     {"-h", "--help", 0, 0, NULL},
      {NULL, "--version", 0, 0, NULL},   {NULL, "--checksum", 1, 0, NULL},
      {"-d", "--directory", 1, 0, NULL}, {NULL, "--hash", 1, 0, NULL},
      {"-o", "--output", 1, 0, NULL}};
  struct Elements elements;
  int return_code = 0;

  elements.n_commands = 0;
  elements.n_arguments = 1;
  elements.n_options = 7;
  elements.commands = commands;
  elements.arguments = arguments;
  elements.options = options;

  if (argc == 1) {
    /* No arguments, default to --help */
    argv[argc++] = "--help";
    argv[argc++] = NULL;
    return_code = 1;
  }

  {
    struct Tokens ts = tokens_new(argc, argv);
    if (parse_args(&ts, &elements))
      exit(1);
  }
  if (elems_to_args(&elements, &args, help, version))
    exit(return_code);
  return args;
}

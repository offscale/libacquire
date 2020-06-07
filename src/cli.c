#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "cli.h"

struct Command {
    const char *name;
    bool value;
};

struct Argument {
    const char *name;
    const char *value;
    const char *array[ARG_MAX];
};

struct Option {
    const char *oshort;
    const char *olong;
    bool argcount;
    bool value;
    const char *argument;
};

struct Elements {
    size_t n_commands;
    size_t n_arguments;
    size_t n_options;
    struct Command *commands;
    struct Argument *arguments;
    struct Option *options;
};


/*
 * Tokens object
 */

struct Tokens {
    size_t argc;
    char **argv;
    size_t i;
    char *current;
};

const char usage_pattern[] =
"Usage:\n"
"  acquire --check --directory=<d> --hash=<h> --checksum=<sha> <url>...\n"
"  acquire --directory=<d> --hash=<h> --checksum=<sha> <url>...\n"
"  acquire --help\n"
"  acquire --version";

struct Tokens tokens_new(size_t argc, char **argv) {
    struct Tokens ts;
    ts.argc = argc;
    ts.argv = argv;
    ts.i = 0;
    ts.current = argv[0];
    return ts;
}

struct Tokens* tokens_move(struct Tokens *ts) {
    if (ts->i < ts->argc) {
        ts->current = ts->argv[++ts->i];
    }
    if (ts->i == ts->argc) {
        ts->current = NULL;
    }
    return ts;
}


/*
 * ARGV parsing functions
 */

size_t parse_doubledash(struct Tokens *ts, struct Elements *elements) {
    /*
    size_t n_commands = elements->n_commands;
    size_t n_arguments = elements->n_arguments;
    Command *commands = elements->commands;
    Argument *arguments = elements->arguments;

    not implemented yet
    return parsed + [Argument(None, v) for v in tokens]
    */
    return 0;
}

size_t parse_long(struct Tokens *ts, struct Elements *elements) {
    size_t i;
    size_t len_prefix;
    size_t n_options = elements->n_options;
    char *eq = strchr(ts->current, '=');
    struct Option *option;
    struct Option *options = elements->options;

    len_prefix = (eq-(ts->current))/sizeof(char);
    for (i=0; i < n_options; i++) {
        option = &options[i];
        if (!strncmp(ts->current, option->olong, len_prefix))
            break;
    }
    if (i == n_options) {
        /* TODO '%s is not a unique prefix */
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
        option->value = true;
    }
    return 0;
}

size_t parse_shorts(struct Tokens *ts, struct Elements *elements) {
    char *raw;
    size_t i;
    size_t n_options = elements->n_options;
    struct Option *option;
    struct Option *options = elements->options;

    raw = &ts->current[1];
    tokens_move(ts);
    while (raw[0] != '\0') {
        for (i=0; i < n_options; i++) {
            option = &options[i];
            if (option->oshort != NULL && option->oshort[1] == raw[0])
                break;
        }
        if (i == n_options) {
            /* TODO -%s is specified ambiguously %d times */
            fprintf(stderr, "-%c is not recognized\n", raw[0]);
            return 1;
        }
        raw++;
        if (!option->argcount) {
            option->value = true;
        } else {
            if (raw[0] == '\0') {
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

size_t parse_argcmd(struct Tokens *ts, struct Elements *elements) {
    size_t i;
    size_t n_commands = elements->n_commands;
    /* size_t n_arguments = elements->n_arguments; */
    struct Command *command;
    struct Command *commands = elements->commands;
    /* Argument *arguments = elements->arguments; */

    for (i=0; i < n_commands; i++) {
        command = &commands[i];
        if (strcmp(command->name, ts->current) == 0) {
            command->value = true;
            tokens_move(ts);
            return 0;
        }
    }
    /* not implemented yet, just skip for now
       parsed.append(Argument(None, tokens.move())) */
    /*fprintf(stderr, "! argument '%s' has been ignored\n", ts->current);
    fprintf(stderr, "  '");
    for (i=0; i<ts->argc ; i++)
        fprintf(stderr, "%s ", ts->argv[i]);
    fprintf(stderr, "'\n");*/
    tokens_move(ts);
    return 0;
}

size_t parse_args(struct Tokens *ts, struct Elements *elements) {
    size_t ret;

    while (ts->current != NULL) {
        if (strcmp(ts->current, "--") == 0) {
            ret = parse_doubledash(ts, elements);
            if (!ret) break;
        } else if (ts->current[0] == '-' && ts->current[1] == '-') {
            ret = parse_long(ts, elements);
        } else if (ts->current[0] == '-' && ts->current[1] != '\0') {
            ret = parse_shorts(ts, elements);
        } else
            ret = parse_argcmd(ts, elements);
        if (ret) return ret;
    }
    return 0;
}

size_t elems_to_args(struct Elements *elements, struct DocoptArgs *args, const bool help,
                     const char *version){
    struct Command *command;
    struct Argument *argument;
    struct Option *option;
    size_t i, j;

    /* fix gcc-related compiler warnings (unused) */
    (void)command;
    (void)argument;

    /* options */
    for (i=0; i < elements->n_options; i++) {
        option = &elements->options[i];
        if (help && strcmp(option->olong, "--help") == 0) {
            for (j = 0; j < 17; j++)
                puts(args->help_message[j]);
            return 1;
        } else if (version && option->value &&
                   strcmp(option->olong, "--version") == 0) {
            puts(version);
            return 1;
        } else if (strcmp(option->olong, "--check") == 0) {
            args->check = option->value;
        } else if (strcmp(option->olong, "--help") == 0) {
            args->help = option->value;
        } else if (strcmp(option->olong, "--version") == 0) {
            args->version = option->value;
        } else if (strcmp(option->olong, "--checksum") == 0) {
            if (option->argument) {
                args->checksum = (char*) option->argument;
            }
        }
        else if (strcmp(option->olong, "--directory") == 0) {
                    if (option->argument) {
                        args->directory = (char*) option->argument;
                    }
                }
        else if (strcmp(option->olong, "--hash") == 0) {
                    if (option->argument) {
                        args->hash = (char*) option->argument;
                    }
                }
        }
    /* commands */
    for (i=0; i < elements->n_commands; i++) {
        command = &elements->commands[i];
    }
    /* arguments */
    for (i=0; i < elements->n_arguments; i++) {
        argument = &elements->arguments[i];
        if (strcmp(argument->name, "<url>") == 0) {
            args->url = (char*) argument->value;
        }
    }
    return 0;
}


/*
 * Main docopt function
 */

struct DocoptArgs docopt(size_t argc, char *argv[], const bool help, const char *version) {
    struct DocoptArgs args = {
        NULL, 0, 0, 0, NULL, NULL, NULL,
        usage_pattern,
        { "acquire: Downloads using libcurl—if not Windows or built with USE_LIBCURL—or Windows APIs.",
          "",
          "Usage:",
          "  acquire --check --directory=<d> --hash=<h> --checksum=<sha> <url>...",
          "  acquire --directory=<d> --hash=<h> --checksum=<sha> <url>...",
          "  acquire --help",
          "  acquire --version",
          "",
          "Options:",
          "  -h --help               Show this screen.",
          "  --version               Show version.",
          "  --check                 Check if already downloaded.",
          "  --hash<h>               Hash to verify.",
          "  --checksum<sha>         Checksum algorithm, e.g., SHA256 or SHA512.",
          "  -d=<d>, --directory=<d> Location to download files to.",
          "  -o FILE --output=FILE   Output file. If not specified, will derive from URL.",
          ""}
    };
    struct Tokens ts;
    struct Command commands[] = {NULL
    };
    struct Argument arguments[] = {
        {"<url>", NULL, NULL}
    };
    struct Option options[] = {
        {NULL, "--check", 0, 0, NULL},
        {"-h", "--help", 0, 0, NULL},
        {NULL, "--version", 0, 0, NULL},
        {NULL, "--checksum", 1, 0, NULL},
        {"-d", "--directory", 1, 0, NULL},
        {NULL, "--hash", 1, 0, NULL}
    };
    struct Elements elements;
    elements.n_commands = 0;
    elements.n_arguments = 1;
    elements.n_options = 6;
    elements.commands = commands;
    elements.arguments = arguments;
    elements.options = options;

    ts = tokens_new(argc, argv);
    if (parse_args(&ts, &elements))
        exit(EXIT_FAILURE);
    if (elems_to_args(&elements, &args, help, version))
        exit(EXIT_SUCCESS);
    return args;
}

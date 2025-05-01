/*
 * CLI for `acquire`, which links with libacquire
 *
 * File is generated. See "README.md" for details.
 * */

#ifndef LIBACQUIRE_DOCOPT_CLI_H
#define LIBACQUIRE_DOCOPT_CLI_H

#include "acquire_cli_lib_export.h"

#ifdef __cplusplus
extern "C" {
#elif defined(__STDC__) && defined(__STDC_VERSION__) &&                        \
    __STDC_VERSION__ >= 199901L

#include <stdbool.h>

#elif !defined(_STDBOOL_H)
#define _STDBOOL_H

#include <stdlib.h>

#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif
#ifdef bool
#undef bool
#endif

#define true 1
#define false (!true)
typedef size_t bool;

#endif

#include <stddef.h>
#include <string.h>

/* ARG_MAX definition block kept as-is for compatibility */
#if defined(_AIX)

#include <sys/limits.h>

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||   \
    defined(__bsdi__) || defined(__DragonFly__) || defined(macintosh) ||       \
    defined(__APPLE__) || defined(__APPLE_CC__)

#ifdef __CC_SUPPORTS_WARNING
#define ____CC_SUPPORTS_WARNING __CC_SUPPORTS_WARNING
#undef __CC_SUPPORTS_WARNING
#include <sys/syslimits.h>
#define __CC_SUPPORTS_WARNING ____CC_SUPPORTS_WARNING
#undef ____CC_SUPPORTS_WARNING
#else
#include <sys/syslimits.h>
#endif /* __CC_SUPPORTS_WARNING */

#elif defined(__HAIKU__)

#include <system/user_runtime.h>

#elif defined(__linux__) || defined(linux) || defined(__linux)

#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 22)

#include <linux/limits.h>

#else

#define ARG_MAX 131072 /* # bytes of args + environ for exec() */

#endif

#elif (defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||  \
       defined(__bsdi__) || defined(__DragonFly__) || defined(macintosh) ||    \
       defined(__APPLE__) || defined(__APPLE_CC__))

#include <sys/param.h>

#if defined(__APPLE__) || defined(__APPLE_CC__)
/* ARG_MAX might cause issues on macOS, so we undefine */
#undef ARG_MAX
#undef NCARGS
#endif

#else

#include <limits.h>

#endif

#ifndef ARG_MAX
#ifdef NCARGS
#define ARG_MAX NCARGS
#else
#define ARG_MAX 131072
#endif
#endif

struct DocoptArgs {
  char *url;
  size_t check;
  size_t help;
  size_t version;
  char *checksum;
  char *directory;
  char *hash;
  char *output;
  const char *usage_pattern;
  const char *help_message[17];
};

extern ACQUIRE_CLI_LIB_EXPORT int docopt(struct DocoptArgs *, int, char *[],
                                         bool, const char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_DOCOPT_CLI_H */

#ifndef DOCOPT_CLI_H
#define DOCOPT_CLI_H

#include <stddef.h>

#include "acquire_cli_export.h"

#if defined(HAS_STDBOOL) && !defined(bool)

#include <stdbool.h>

#else

#include <acquire_stdbool.h>

#endif

#if defined(_AIX)

#include <sys/limits.h>

#elif defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__bsdi__) \
 || defined(__DragonFly__) || defined(macintosh) \
 || defined(__APPLE__) || defined(__APPLE_CC__)

#ifdef __CC_SUPPORTS_WARNING
#define ____CC_SUPPORTS_WARNING __CC_SUPPORTS_WARNING
#undef __CC_SUPPORTS_WARNING
#include <sys/syslimits.h>
#define __CC_SUPPORTS_WARNING ____CC_SUPPORTS_WARNING
#undef ____CC_SUPPORTS_WARNING
#else
#include <sys/syslimits.h>
#endif

#elif defined(__HAIKU__)

#include <system/user_runtime.h>

#elif defined(__linux__) || defined(linux) || defined(__linux)

#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)

#include <linux/limits.h>

#else

#define ARG_MAX       131072    /* # bytes of args + environ for exec() */
/* it's no longer defined, see this example and more at https://unix.stackexchange.com/q/120642 */

#endif

#elif (defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) \
 || defined(__DragonFly__) || defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__))

#include <sys/param.h>

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

struct ACQUIRE_CLI_EXPORT DocoptArgs {

    /* arguments */
    char *url;
    /* options without arguments */
    size_t check;
    size_t help;
    size_t version;
    /* options with arguments */
    char *checksum;
    char *directory;
    char *hash;
    /* special */
    const char *usage_pattern;
    const char *help_message[16];
};

struct DocoptArgs ACQUIRE_CLI_EXPORT docopt(size_t, char *[], bool, const char *);

#endif

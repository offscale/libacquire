/*
 * Common definitions used through libacquire
 *
 * Defines things like `PATH_SEP`, `MAX_FILENAME`, and `NAME_MAX`
 * */

#ifndef LIBACQUIRE_ACQUIRE_COMMON_DEFS_H
#define LIBACQUIRE_ACQUIRE_COMMON_DEFS_H

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* defined(HAS_STDBOOL) && !defined(bool) */

#include "acquire_errors.h"
#include "acquire_url_utils.h"
#include "acquire_fileutils.h"
#include "acquire_checksums.h"
#include "acquire_download.h"

#ifdef _AIX
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
#endif /* __CC_SUPPORTS_WARNING */

#elif defined(__HAIKU__)
#include <system/user_runtime.h>
#elif defined(__linux__) || defined(linux) || defined(__linux)

#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 22)

#include <linux/limits.h>

#elif defined(sun) || defined(__sun) || defined(__SVR4) || defined(__svr4__)

#include <sys/param.h>

#else

#include <limits.h>

#endif /* LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 22) */

#endif /* _AIX */

#ifndef NAME_MAX

#ifdef PATH_MAX
#define NAME_MAX PATH_MAX
#else
#define NAME_MAX 4096
#endif /* PATH_MAX */

#endif /* NAME_MAX */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#ifndef MAX_FILENAME
#define MAX_FILENAME 255
#endif /* ! MAX_FILENAME */

#ifndef NAME_MAX
#define NAME_MAX 4096
#endif /* ! NAME_MAX */

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define _AMD64_
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__i386__) || defined(_M_IX86)
#define _X86_
#elif defined(__arm__) || defined(_M_ARM) || defined(_M_ARMT)
#define _ARM_
#endif

#endif /* ! LIBACQUIRE_ACQUIRE_COMMON_DEFS_H */

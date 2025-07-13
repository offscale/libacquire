/*
 * string functions helpful on Linux (and sometimes BSD)
 * are now made available on other platforms (Windows, SunOS, &etc.)
 * */

#ifndef LIBACQUIRE_ACQUIRE_STRING_EXTRAS_H
#define LIBACQUIRE_ACQUIRE_STRING_EXTRAS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef HAVE_LIBBSD
#include <bsd/string.h>
#else
#include <string.h>
#endif /* HAVE_LIBBSD 1 */

#include "libacquire_export.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||     \
    defined(__bsdi__) || defined(__DragonFly__) || defined(BSD)
#define ANY_BSD 1
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

/* snprintf is implemented in VS 2015 */
#if _MSC_VER >= 1900

#define HAVE_SNPRINTF 1

#endif /* _MSC_VER >= 1900 */

#else

#define HAVE_STRNCASECMP 1

#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#else

#include <sys/param.h>

#if _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE ||                   \
    _POSIX_C_SOURCE >= 200112L
#define HAVE_SNPRINTF 1
#endif /* _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE ||             \
          _POSIX_C_SOURCE >= 200112L */
#if defined(_GNU_SOURCE) || defined(ANY_BSD)
#define HAVE_STRCASESTR 1
#endif /* defined(_GNU_SOURCE) || defined(ANY_BSD) */

#if defined(__APPLE__) && defined(__MACH__)
#define HAVE_SNPRINTF 1
#define HAVE_STRNCASECMP 1
#endif /* defined(__APPLE__) && defined(__MACH__) */

#if defined(BSD) && (BSD >= 199306) && !defined(__linux__) &&                  \
    !defined(linux) && !defined(__linux) && !defined(HAVE_STRNSTR)
#define HAVE_STRNSTR 1
#endif /* defined(BSD) && (BSD >= 199306) && !defined(__linux__) &&            \
          !defined(linux) && !defined(__linux) && !defined(HAVE_STRNSTR) */

#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

#if defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__

#define HAVE_STRERRORLEN_S 1

#else

#if !defined(__APPLE__) && !defined(__APPLE_CC__)
typedef int errno_t;
#endif /* !defined(__APPLE__) && !defined(__APPLE_CC__) */

#if defined(__linux__) || defined(linux) || defined(__linux) || defined(ANY_BSD)

#ifndef strerror_s
#define strerror_s strerror_r
#endif /* !strerror_s */

/*#define HAVE_STRERRORLEN_S 1*/
#endif /* defined(__linux__) || defined(linux) || defined(__linux) ||          \
          defined(ANY_BSD) */

#endif /* defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ */

#ifndef _MSC_VER
#define HAVE_STRINGS_H 1
/*#define HAVE_STRNCASECMP*/
#endif /* !_MSC_VER */

#if defined(ANY_BSD) || defined(__APPLE__) && defined(__MACH__) ||             \
    defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define HAVE_ASPRINTF 1
#define HAVE_SNPRINTF 1
#endif /* defined(ANY_BSD) || defined(__APPLE__) && defined(__MACH__) ||       \
          defined(_GNU_SOURCE) || defined(_BSD_SOURCE) */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#if !defined(HAVE_STRNCASECMP) || defined(STRNCASECMP_IMPL) && STRNCASECMP_IMPL

extern LIBACQUIRE_EXPORT int strncasecmp(const char *, const char *, size_t);

extern LIBACQUIRE_EXPORT int strcasecmp(const char *, const char *);

#endif /* !defined(HAVE_STRNCASECMP) || defined(STRNCASECMP_IMPL) &&           \
          STRNCASECMP_IMPL */

#if !defined(HAVE_STRNSTR) || defined(STRNSTR_IMPL) && STRNSTR_IMPL

extern LIBACQUIRE_EXPORT char *strnstr(const char *, const char *, size_t);

#endif /* !defined(HAVE_STRNSTR) || defined(STRNSTR_IMPL) && STRNSTR_IMPL */

#if !defined(HAVE_STRCASESTR) || defined(STRCASESTR_IMPL) && STRCASESTR_IMPL

extern LIBACQUIRE_EXPORT char *strcasestr(const char *, const char *);

#endif /* !defined(HAVE_STRCASESTR) || defined(STRCASESTR_IMPL) &&             \
          STRCASESTR_IMPL */

#if !defined(HAVE_STRERRORLEN_S) || HAVE_STRERRORLEN_S == 0 ||                 \
    defined(STRERRORLEN_IMPL) && STRERRORLEN_IMPL

extern LIBACQUIRE_EXPORT size_t strerrorlen_s(errno_t);

#endif /* !defined(HAVE_STRERRORLEN_S) || defined(STRERRORLEN_IMPL) &&         \
          STRERRORLEN_IMPL */

#if !defined(HAVE_SNPRINTF) || defined(SNPRINTF_IMPL) && SNPRINTF_IMPL
extern LIBACQUIRE_EXPORT int snprintf(char *buffer, size_t count,
                                      const char *format, ...);
#endif /* !defined(HAVE_SNPRINTF) || defined(SNPRINTF_IMPL) && SNPRINTF_IMPL   \
        */

#ifdef LIBACQUIRE_IMPLEMENTATION

#if !defined(HAVE_SNPRINTF) && defined(SNPRINTF_IMPL) && SNPRINTF_IMPL
#define HAVE_SNPRINTF 1

/*
 * `snprintf`, `vsnprintf`, `strnstr` taken from:
 * https://chromium.googlesource.com/chromium/blink/+/5cedd2fd208daf119b9ea47c7c1e22d760a586eb/Source/wtf/StringExtras.h
 * …then modified to remove C++ specifics and WebKit specific macros
 *
 * Copyright (C) 2006, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2020 Offscale.io. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

int snprintf(char *buffer, size_t count, const char *format, ...) {
  int result;
  va_list args;
  va_start(args, format);
  result = _vsnprintf(buffer, count, format, args);
  va_end(args);
  /* In the case where the string entirely filled the buffer, _vsnprintf will
     not null-terminate it, but snprintf must. */
  if (count > 0)
    buffer[count - 1] = '\0';
  return result;
}

inline double wtf_vsnprintf(char *buffer, size_t count, const char *format,
                            va_list args) {
  int result = _vsnprintf(buffer, count, format, args);
  /* In the case where the string entirely filled the buffer, _vsnprintf will
     not null-terminate it, but vsnprintf must. */
  if (count > 0)
    buffer[count - 1] = '\0';
  return result;
}

/* Work around a difference in Microsoft's implementation of vsnprintf, where
   vsnprintf does not null terminate the buffer. WebKit can rely on the null
   termination. Microsoft's implementation is fixed in VS 2015. */
#define vsnprintf(buffer, count, format, args)                                 \
  wtf_vsnprintf(buffer, count, format, args)

#endif /* !defined(HAVE_SNPRINTF) && defined(SNPRINTF_IMPL) && SNPRINTF_IMPL   \
        */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) &&                         \
    !defined(STRNCASECMP_IMPL)
/* TODO: remove this hack */
#define STRNCASECMP_IMPL 1
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) &&                   \
          !defined(STRNCASECMP_IMPL) */

#if !defined(HAVE_STRNCASECMP) && defined(STRNCASECMP_IMPL) && STRNCASECMP_IMPL
#define HAVE_STRNCASECMP 1

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
/* this didn't work `#define strncasecmp _strnicmp` */
LIBACQUIRE_EXPORT int strncasecmp(const char *_l, const char *_r, size_t n) {
  return _strnicmp(_l, _r, n);
}

/* this didn't work `#define strcasecmp _stricmp` */
LIBACQUIRE_EXPORT int strcasecmp(const char *l, const char *r) {
  return _stricmp(l, r);
}

#else
/* from MIT licensed musl @ e0ef93c20de1a9e0a6b8f4a4a951a8e61a1a2973 */
int strncasecmp(const char *_l, const char *_r, size_t n) {
  const unsigned char *l = (void *)_l, *r = (void *)_r;
  if (!n--)
    return 0;
  for (; *l && *r && n && (*l == *r || tolower(*l) == tolower(*r));
       l++, r++, n--)
    ;
  return tolower(*l) - tolower(*r);
}
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#endif /* !defined(HAVE_STRNCASECMP) && defined(STRNCASECMP_IMPL) &&           \
          STRNCASECMP_IMPL */

#if !defined(HAVE_STRNSTR) && defined(STRNSTR_IMPL) && STRNSTR_IMPL
#define HAVE_STRNSTR 1

char *strnstr(const char *buffer, const char *target, size_t bufferLength) {
  /*
     Find the first occurrence of find in s, where the search is limited to the
     first slen characters of s.

  DESCRIPTION
       The strnstr() function locates the	first occurrence of the
  null-termi-
       nated string little in the	string big, where not more than	len
  characters are searched.  Characters that appear after a `\0'	character are
  not searched.

  RETURN VALUES
       If	little is an empty string, big is returned; if little occurs
  nowhere in	big, NULL is returned; otherwise a pointer to the first
  character of the first occurrence of little is returned.

   [this doc (c) FreeBSD <3 clause BSD license> from their manpage]  */
  const size_t targetLength = strlen(target);
  const char *start;
  if (targetLength == 0)
    return (char *)buffer;
  for (start = buffer; *start && start + targetLength <= buffer + bufferLength;
       start++) {
    if (*start == *target &&
        strncmp(start + 1, target + 1, targetLength - 1) == 0)
      return (char *)(start);
  }
  return 0;
}
#endif /* !defined(HAVE_STRNSTR) && defined(STRNSTR_IMPL) && STRNSTR_IMPL */

#if !defined(HAVE_STRCASESTR) && defined(STRCASESTR_IMPL) && STRCASESTR_IMPL
#define HAVE_STRCASESTR 1
/* `strcasestr` from MUSL */

char *strcasestr(const char *h, const char *n) {
  const size_t l = strlen(n);
  for (; *h; h++)
    if (!strncasecmp(h, n, l))
      return (char *)h;
  return 0;
}

#endif /* !defined(HAVE_STRCASESTR) && defined(STRCASESTR_IMPL) &&             \
          STRCASESTR_IMPL */

#if !defined(HAVE_STRERRORLEN_S) && defined(STRERRORLEN_IMPL) &&               \
    STRERRORLEN_IMPL
#define HAVE_STRERRORLEN_S 1
/* MIT licensed function from Safe C Library */

size_t strerrorlen_s(errno_t errnum) {
#ifndef ESNULLP
#define ESNULLP (400) /* null ptr                    */
#endif

#ifndef ESLEWRNG
#define ESLEWRNG (410) /* wrong size                */
#endif

#ifndef ESLAST
#define ESLAST ESLEWRNG
#endif

  static const int len_errmsgs_s[] = {
      sizeof "null ptr",                 /* ESNULLP */
      sizeof "length is zero",           /* ESZEROL */
      sizeof "length is below min",      /* ESLEMIN */
      sizeof "length exceeds RSIZE_MAX", /* ESLEMAX */
      sizeof "overlap undefined",        /* ESOVRLP */
      sizeof "empty string",             /* ESEMPTY */
      sizeof "not enough space",         /* ESNOSPC */
      sizeof "unterminated string",      /* ESUNTERM */
      sizeof "no difference",            /* ESNODIFF */
      sizeof "not found",                /* ESNOTFND */
      sizeof "wrong size",               /* ESLEWRNG */
  };

  if (errnum >= ESNULLP && errnum <= ESLAST) {
    return len_errmsgs_s[errnum - ESNULLP] - 1;
  } else {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif /* _MSC_VER */
    const char *buf = strerror(errnum);
#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */
    return buf ? strlen(buf) : 0;
  }
}

#endif /* !defined(HAVE_STRERRORLEN_S) && defined(STRERRORLEN_IMPL) &&         \
          STRERRORLEN_IMPL */

#endif /* LIBACQUIRE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_STRING_EXTRAS_H */

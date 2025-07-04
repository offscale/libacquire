/*
 * Basic URL functions (where URL is `const char*`)
 * */

#ifndef LIBACQUIRE_ACQUIRE_URL_UTILS_H
#define LIBACQUIRE_ACQUIRE_URL_UTILS_H

#include "acquire_common_defs.h"
#include "libacquire_export.h"
#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <string.h>
#ifndef strdup
#define strdup _strdup
#endif /* !strdup */
#else
#include "acquire_string_extras.h"
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */
#include <acquire_common_defs.h>
#include <string.h>
#ifndef NAME_MAX
#define NAME_MAX 4096
#endif /* ! NAME_MAX */

/**
 * @brief Extract the path component (filename) from a URL string.
 *
 * Given a URL string, returns a newly allocated string containing
 * everything after the last '/' character in the URL, stopping before any
 * query ('?') or fragment ('#') delimiter.
 *
 * If the URL is NULL or empty, returns NULL.
 *
 * @param url Input URL string.
 * @return Pointer to a newly allocated string, or `NULL` on failure.
 *         The caller is responsible for freeing this memory.
 */
extern LIBACQUIRE_EXPORT char *get_path_from_url(const char *url);

/**
 * @brief Check whether a given string appears to be a URL.
 *
 * Performs a basic heuristic check that string starts with "http://",
 * "https://", "ftp://", or "ftps://".
 *
 * @param maybe_url String to be tested.
 * @return true if string looks like a URL, false otherwise.
 */
extern LIBACQUIRE_EXPORT bool is_url(const char *maybe_url);

#if defined(LIBACQUIRE_IMPLEMENTATION) &&                                      \
    defined(LIBACQUIRE_ACQUIRE_URL_UTILS_IMPL)

char *get_path_from_url(const char *url) {
  char buf[NAME_MAX + 1];
  const char *last_slash, *end;
  size_t len;

  if (!url || url[0] == '\0')
    return NULL;

  last_slash = strrchr(url, '/');
  if (last_slash) {
    if ((last_slash - url) < 9)
      return strdup("");
    last_slash++;
  } else
    last_slash = url;

  end = last_slash;
  while (*end != '\0' && *end != '?' && *end != '#') {
    end++;
  }

  len = end - last_slash;
  if (len >= sizeof(buf)) {
    len = sizeof(buf) - 1;
  }

  strncpy(buf, last_slash, len);
  buf[len] = '\0';

  return strdup(buf);
}

bool is_url(const char *maybe_url) {
  return maybe_url != NULL && strlen(maybe_url) > 5 &&
         (strncasecmp(maybe_url, "http://", 7) == 0 ||
          strncasecmp(maybe_url, "https://", 8) == 0 ||
          strncasecmp(maybe_url, "ftp://", 6) == 0 ||
          strncasecmp(maybe_url, "ftps://", 7) == 0 ||
          strncasecmp(maybe_url, "file://", 7) == 0);
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_ACQUIRE_URL_UTILS_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_URL_UTILS_H */

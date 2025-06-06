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
#define strdup _strdup
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
 * Given a URL string, returns a pointer to a static buffer holding
 * everything after the last '/' character in the URL, stopping before any
 * query ('?') or fragment ('#') delimiter.
 *
 * If the URL is NULL or empty, returns NULL.
 *
 * @param url Input URL string.
 * @return Pointer to static buffer with filepath component xor `NULL`.
 *
 * @note Returned pointer points to a static buffer that may be overwritten
 * on later calls.
 */
extern LIBACQUIRE_EXPORT const char *get_path_from_url(const char *url);

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

const char *get_path_from_url(const char *url) {
  static char buf[NAME_MAX + 1];
  const char *last_slash;
  size_t i;

  if (!url || url[0] == '\0')
    return NULL;

  last_slash = strrchr(url, '/');
  if (last_slash)
    last_slash++;
  else
    last_slash = url;

  for (i = 0; i < NAME_MAX && last_slash[i] != '\0'; i++) {
    if (last_slash[i] == '?' || last_slash[i] == '#')
      break;
    buf[i] = last_slash[i];
  }
  buf[i] = '\0';

  return buf;
}

bool is_url(const char *maybe_url) {
  if (strlen(maybe_url) < 8)
    return false;
  else if (maybe_url[0] == 'h' && maybe_url[1] == 't' && maybe_url[2] == 't' &&
           maybe_url[3] == 'p')
    return (maybe_url[4] == ':' && maybe_url[5] == '/' &&
            maybe_url[6] == '/') ||
           (maybe_url[4] == 's' && maybe_url[5] == ':' && maybe_url[6] == '/' &&
            maybe_url[7] == '/');
  else if (maybe_url[0] == 'f' && maybe_url[1] == 't' && maybe_url[2] == 'p')
    return (maybe_url[3] == ':' && maybe_url[4] == '/' &&
            maybe_url[5] == '/') ||
           (maybe_url[3] == 's' && maybe_url[4] == ':' && maybe_url[5] == '/' &&
            maybe_url[6] == '/');
  return false /* strchr(maybe_url, '/') != NULL */;
}

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_ACQUIRE_URL_UTILS_IMPL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! LIBACQUIRE_ACQUIRE_URL_UTILS_H */

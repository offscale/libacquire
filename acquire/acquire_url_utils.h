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
#include <stdlib.h>
#include <string.h>
#ifndef NAME_MAX
#define NAME_MAX 4096
#endif /* ! NAME_MAX */

/**
 * @brief Extract the path component (filename) from a URL string.
 *
 * Given a URL string, returns a newly allocated string containing
 * everything after the last '/' character in the URL's path, stopping before
 * any query ('?') or fragment ('#') delimiter.
 *
 * @param url Input URL string.
 * @return Pointer to a newly allocated string, or `NULL` on failure or if the
 *         input is NULL/empty. The caller is responsible for freeing this
 * memory.
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

#include <acquire_string_extras.h>

char *get_path_from_url(const char *url) {
  const char *path_start, *filename_start, *query_or_frag;
  size_t len;
  char *result;

  if (!url || !*url) {
    return NULL;
  }

  path_start = strstr(url, "://");
  if (path_start) {
    /* It's a URL. Find the path part, which starts after the authority. */
    path_start = strchr(path_start + 3, '/');
    if (!path_start) {
      /* URL has host, but no path. E.g. http://example.com */
      return strdup("");
    }
  } else {
    /* Not a full URL with scheme. Treat as local path. */
    path_start = url;
  }

  /* Now path_start points to the beginning of the path, e.g.,
   * "/path/to/file.txt" */
  filename_start = strrchr(path_start, '/');
  if (filename_start) {
    /* We found a slash, the filename is after it */
    filename_start++;
  } else {
    /* No slashes in path, the whole path is the filename */
    filename_start = path_start;
  }

  /* Find end of filename (start of query or fragment) */
  query_or_frag = filename_start;
  while (*query_or_frag && *query_or_frag != '?' && *query_or_frag != '#') {
    query_or_frag++;
  }

  len = query_or_frag - filename_start;
  result = (char *)malloc(len + 1);
  if (!result)
    return NULL; /* LCOV_EXCL_LINE */

  memcpy(result, filename_start, len);
  result[len] = '\0';
  return result;
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

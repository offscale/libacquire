#ifndef LIBACQUIRE_STRINGUTILS_H
#define LIBACQUIRE_STRINGUTILS_H

#include <string.h>
#include "stdbool.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define strdup _strdup
#endif

const char *get_path_from_url(const char *url) {
    size_t i;
    char *end_possible_query = strdup(url);
    end_possible_query = strrchr(end_possible_query, '/') + 1;
    for (i = 0; i < strlen(end_possible_query); i++)
        if (end_possible_query[i] == '?' || end_possible_query[i] == '#') {
            end_possible_query[i] = '\0';
            break;
        }
    return end_possible_query;
}

bool is_url(const char *maybe_url) {
    if (strlen(maybe_url) < 8) return false;
    else if (maybe_url[0] == 'h' && maybe_url[1] == 't' && maybe_url[2] == 't' && maybe_url[3] == 'p')
        return (maybe_url[4] == ':' && maybe_url[5] == '/' && maybe_url[6] == '/')
               || (maybe_url[4] == 's' && maybe_url[5] == ':' && maybe_url[6] == '/' && maybe_url[7] == '/');
    else if (maybe_url[0] == 'f' && maybe_url[1] == 't' && maybe_url[2] == 'p')
        return (maybe_url[3] == ':' && maybe_url[4] == '/' && maybe_url[5] == '/')
               || (maybe_url[3] == 's' && maybe_url[4] == ':' && maybe_url[5] == '/' && maybe_url[6] == '/');
    return false /* strchr(maybe_url, '/') != NULL */;
}

#endif /* LIBACQUIRE_STRINGUTILS_H */

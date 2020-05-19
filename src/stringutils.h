#ifndef LIBACQUIRE_STRINGUTILS_H
#define LIBACQUIRE_STRINGUTILS_H

#include <string.h>

const char *get_path_from_url(const char *url) {
    size_t i, end_possible_query_n;
    const char *end_possible_query = strrchr(url, '/') + 1;
    char *path = strdup(end_possible_query);
    end_possible_query_n = strlen(end_possible_query);
    for (i = 0; i < end_possible_query_n; i++)
        if (path[i] == '?' || path[i] == '#')
            while (i < end_possible_query_n)
                path[i++] = '\0';
    return path;
}

#endif //LIBACQUIRE_STRINGUTILS_H

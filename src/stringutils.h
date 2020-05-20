#ifndef LIBACQUIRE_STRINGUTILS_H
#define LIBACQUIRE_STRINGUTILS_H

#include <string.h>

const char *get_path_from_url(const char *url) {
    size_t i;
    char *end_possible_query = strrchr(url, '/') + 1;
    end_possible_query = strdup(end_possible_query);
    for (i = 0; i < strlen(end_possible_query); i++)
        if (end_possible_query[i] == '?' || end_possible_query[i] == '#') {
            end_possible_query[i] = '\0';
            break;
        }
    return end_possible_query;
}

#endif //LIBACQUIRE_STRINGUTILS_H

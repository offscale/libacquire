/*
 * Prototype for extract API
 *
 * Always `#include` this when adding new extraction implementations,
 * to ensure the implementation matches the prototype.
 *
 * The extract API is for expanding the contents of archives
 * */

#ifndef LIBACQUIRE_ACQUIRE_EXTRACT_H
#define LIBACQUIRE_ACQUIRE_EXTRACT_H

enum Archive {
    LIBACQUIRE_ZIP,
    LIBACQUIRE_INFER,
    LIBACQUIRE_UNSUPPORTED_ARCHIVE
};

extern int extract_archive(enum Archive, const char *, const char *);

#endif /* LIBACQUIRE_ACQUIRE_EXTRACT_H */

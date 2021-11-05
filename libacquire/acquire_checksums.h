/*
 * Prototype for checksum API
 *
 * Always `#include` this when adding new checksum implementations,
 * to ensure the implementation matches the prototype.
 * */

#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif
#include "acquire_config.h"
#include "acquire_string_extras.h"

extern bool sha256(const char *, const char *);

extern bool sha512(const char *, const char *);

enum Checksum {
    LIBACQUIRE_SHA256,
    LIBACQUIRE_SHA512,
    LIBACQUIRE_UNSUPPORTED_CHECKSUM
};

extern enum Checksum string2checksum(const char *);

extern bool (*get_checksum_function(enum Checksum))(const char *, const char *);

#ifdef LIBACQUIRE_IMPLEMENTATION

extern enum Checksum string2checksum(const char *s) {
    if (strncasecmp(s, "SHA256", 6) == 0)
        return LIBACQUIRE_SHA256;
    else if (strncasecmp(s, "SHA512", 6) == 0)
        return LIBACQUIRE_SHA512;
    else
        return LIBACQUIRE_UNSUPPORTED_CHECKSUM;
}

bool (*get_checksum_function(enum Checksum checksum))(const char *, const char *) {
    switch (checksum) {
        case LIBACQUIRE_SHA256:
            return sha256;
        case LIBACQUIRE_SHA512:
            return sha512;
        case LIBACQUIRE_UNSUPPORTED_CHECKSUM:
        default:
            return NULL;
    }
}

#endif /* LIBACQUIRE_IMPLEMENTATION */

#endif /* LIBACQUIRE_ACQUIRE_CHECKSUMS_H */


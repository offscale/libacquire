/*
That way you don't need to trust me, just the `diff` on your end,
*/
#ifndef LIBACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_CHECKSUMS_H

#include "stdbool.h"
#include "config.h"

extern bool sha256(const char *, const char *);
extern bool sha512(const char *, const char *);

enum Checksum {
    SHA256,
    SHA512,
    UNSUPPORTED
};

#endif /* LIBACQUIRE_CHECKSUMS_H */


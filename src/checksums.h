#ifndef LIBACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_CHECKSUMS_H

#include "stdbool.h"

extern bool sha256(const char *, const char *);
extern bool sha512(const char *, const char *);

enum Checksums {
    SHA256,
    SHA512
};

#endif /* LIBACQUIRE_CHECKSUMS_H */


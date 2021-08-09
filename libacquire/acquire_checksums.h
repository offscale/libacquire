/*
That way you don't need to trust me, just the `diff` on your end,
*/
#ifndef LIBACQUIRE_ACQUIRE_CHECKSUMS_H
#define LIBACQUIRE_ACQUIRE_CHECKSUMS_H

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif
#include "acquire_config.h"

extern bool sha256(const char *, const char *);

extern bool sha512(const char *, const char *);

enum Checksum {
    LIBACQUIRE_SHA256,
    LIBACQUIRE_SHA512,
    UNSUPPORTED
};

#endif /* LIBACQUIRE_ACQUIRE_CHECKSUMS_H */


#ifndef LIBACQUIRE_MACOS_CRYPTO_H
#define LIBACQUIRE_MACOS_CRYPTO_H

#include "config.h"

#ifdef USE_COMMON_CRYPTO
#include "checksums.h"
#include <CommonCrypto/CommonDigest.h>

bool sha256(const char *a, const char *b) {
    CC_SHA256_Init();
    CC_SHA256_Update();
    CC_SHA256_Final();
    /*extern int CC_SHA256_Init(CC_SHA256_CTX *c);
    extern int CC_SHA256_Update(CC_SHA256_CTX *c, const void *data, CC_LONG len);
    extern int CC_SHA256_Final(unsigned char *md, CC_SHA256_CTX *c);*/
    return false;
}

/* bool sha512(const char *, const char *); */
#endif

#endif /* LIBACQUIRE_MACOS_CRYPTO_H */

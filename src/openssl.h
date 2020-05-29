#ifndef LIBACQUIRE_OPENSSL_H
#define LIBACQUIRE_OPENSSL_H

/* started from https://wiki.openssl.org/index.php?title=Libcrypto_API&oldid=2082 */

#include <stdlib.h>
#include "stdbool.h"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

const char *openssl_runner() {
    /* Load the human readable error strings for libcrypto */
    ERR_load_crypto_strings();

    /* Load all digest and cipher algorithms */
    OpenSSL_add_all_algorithms();

    /* Load config file, and other important initialisation */
    OPENSSL_config(NULL);

    /* ... Do some crypto stuff here ... */

    /* Clean up */

    /* Removes all digests and ciphers */
    EVP_cleanup();

    /* if you omit the next, a small leak may be left when you make use of the BIO (low level API) for e.g. base64 transformations */
    CRYPTO_cleanup_all_ex_data();

    /* Remove error strings */
    ERR_free_strings();

    return 0;
}

bool sha256(const char *payload, const char *checksum) {
    puts("GOT HERE");
    return false;
}

bool sha512(const char *payload, const char *checksum) {
    return false;
}

#endif /* LIBACQUIRE_OPENSSL_H */

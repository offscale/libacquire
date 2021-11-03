/*
 * openssl implementation of libacquire's checksum API
 *
 * This should also work with OpenSSL derived distributions also, like:
 *   - LibreSSL
 *   - CommonCrypto (macOS)
 *
 * YMMV. Any issues, post to source GitHub.
 * */

#if !defined(LIBACQUIRE_OPENSSL_H) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_OPENSSL_H

#include "acquire_config.h"

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)

#include "acquire_checksums.h"

#ifdef USE_COMMON_CRYPTO
#include <CommonCrypto/CommonDigest.h>
#define SHA256_DIGEST_LENGTH CC_SHA256_DIGEST_LENGTH
#define SHA256_CTX CC_SHA256_CTX
#define SHA256_Init CC_SHA256_Init
#define SHA256_Update CC_SHA256_Update
#define SHA256_Final CC_SHA256_Final
#define SHA256_BLOCK_BYTES CC_SHA256_BLOCK_BYTES

#define SHA512_DIGEST_LENGTH CC_SHA512_DIGEST_LENGTH
#define SHA512_CTX CC_SHA512_CTX
#define SHA512_Init CC_SHA512_Init
#define SHA512_Update CC_SHA512_Update
#define SHA512_Final CC_SHA512_Final
#define SHA512_BLOCK_BYTES CC_SHA512_BLOCK_BYTES

#elif defined(USE_OPENSSL)

#include <openssl/sha.h>

#define SHA256_BLOCK_BYTES       64          /* block size in bytes */
#define SHA512_BLOCK_BYTES       (SHA256_BLOCK_BYTES*2)
#endif

#include <string.h>
#include <stdio.h>
#include <errno.h>

int sha256_file(const char *filename,
                unsigned char sha_output[SHA256_DIGEST_LENGTH * 2 + 1]) {
    /* mostly from BSD licensed balrog/zanka-full */
    FILE					*fp;
    SHA256_CTX              sha256_context;
    size_t					bytes;
    char					buffer[BUFSIZ];
    static unsigned char	hex[] = "0123456789abcdef";
    unsigned char			sha[SHA256_DIGEST_LENGTH];
    int						exit_code=EXIT_SUCCESS;

    /* zero out checksum */
    memset(sha_output, 0, SHA256_DIGEST_LENGTH * 2 + 1);

#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    fopen_s(&fp, filename, "r");
#else
    fp = fopen(filename, "r");
#endif

    if(!fp) {
        fprintf(stderr, "Could not open %s: %s", filename, strerror(errno));
        exit_code = ENOENT;
        goto cleanup;
    }

    /* calculate a SHA-1 checksum for files */
    SHA256_Init(&sha256_context);

    /* the file is small enough, checksum it all */
    while((bytes = fread(buffer, 1, sizeof(buffer), fp)))
        SHA256_Update(&sha256_context, buffer, bytes);

    SHA256_Final(sha, &sha256_context);

    /* map into hex characters */
    {
        int i;
        for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sha_output[i + i] = hex[sha[i] >> 4];
            sha_output[i + i + 1] = hex[sha[i] & 0x0F];
        }
        sha_output[i+i] = '\0';
    }

cleanup:
    fclose(fp);

    return exit_code;
}

int sha512_file(const char *filename,
                unsigned char sha_output[SHA512_DIGEST_LENGTH * 2 + 1]) {
    /* mostly from BSD licensed balrog/zanka-full… should deduplicate with above function */
    FILE					*fp;
    SHA512_CTX              sha512_context;
    size_t					bytes;
    char					buffer[BUFSIZ];
    static unsigned char	hex[] = "0123456789abcdef";
    unsigned char			sha[SHA512_DIGEST_LENGTH];
    int						exit_code=EXIT_SUCCESS;

    /* zero out checksum */
    memset(sha_output, 0, SHA512_DIGEST_LENGTH * 2 + 1);

#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    fopen_s(&fp, filename, "r");
#else
    fp = fopen(filename, "r");
#endif

    if(!fp) {
        fprintf(stderr, "Could not open %s: %s", filename, strerror(errno));
        exit_code = ENOENT;
        goto cleanup;
    }

    /* calculate a SHA-1 checksum for files */
    SHA512_Init(&sha512_context);

    /* the file is small enough, checksum it all */
    while((bytes = fread(buffer, 1, sizeof(buffer), fp)))
        SHA512_Update(&sha512_context, buffer, bytes);

    SHA512_Final(sha, &sha512_context);

    /* map into hex characters */
    {
        int i;
        for (i = 0; i < SHA512_DIGEST_LENGTH; i++) {
            sha_output[i + i] = hex[sha[i] >> 4];
            sha_output[i + i + 1] = hex[sha[i] & 0x0F];
        }
        sha_output[i+i] = '\0';
    }

    cleanup:
    fclose(fp);

    return exit_code;
}

bool sha256(const char *filename, const char *hash) {
    unsigned char sha_output[SHA256_DIGEST_LENGTH * 2 + 1];
    sha256_file(filename, sha_output);

    return strcmp((const char*)sha_output, hash) == 0;
}

bool sha512(const char *filename, const char *hash) {
    unsigned char sha_output[SHA512_DIGEST_LENGTH * 2 + 1];
    sha512_file(filename, sha_output);

    return strcmp((const char*)sha_output, hash) == 0;
}

#endif /* defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL) */

#endif /* !defined(LIBACQUIRE_OPENSSL_H) && defined(LIBACQUIRE_IMPLEMENTATION) */

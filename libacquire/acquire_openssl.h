#ifndef LIBACQUIRE_OPENSSL_H
#define LIBACQUIRE_OPENSSL_H

#include "acquire_config.h"

#if defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL)

#include "checksums.h"

#ifdef USE_COMMON_CRYPTO
#include <CommonCrypto/CommonDigest.h>
#define SHA256_DIGEST_LENGTH CC_SHA256_DIGEST_LENGTH
#define SHA256_CTX CC_SHA256_CTX
#define SHA256_Init CC_SHA256_Init
#define SHA256_Update CC_SHA256_Update
#define SHA256_Final CC_SHA256_Final
#define SHA256_BLOCK_BYTES CC_SHA256_BLOCK_BYTES
#elif defined(USE_OPENSSL)

#include <openssl/sha.h>

#define SHA256_BLOCK_BYTES       64          /* block size in bytes */
#endif

#include <string.h>
#include <stdio.h>
#include <errno.h>


void sha256_hash_string(unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65]) {
    size_t i;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);

    outputBuffer[SHA256_BLOCK_BYTES] = 0;
}

int sha256_file(const char *path, char outputBuffer[SHA256_BLOCK_BYTES]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    FILE *file = fopen(path, "rb");
    const int bufSize = 32768;
    unsigned char *buffer = malloc(bufSize);
    unsigned short exit_code = EXIT_SUCCESS;
    size_t bytesRead;

    if (!file) {
        exit_code = ENOENT;
        goto cleanup;
    }

    SHA256_Init(&sha256);

    if (!buffer) {
        exit_code = ENOMEM;
        goto cleanup;
    }
    for (; (bytesRead = fread(buffer, 1, bufSize, file)); SHA256_Update(&sha256, buffer, bytesRead)) {}


    SHA256_Final(hash, &sha256);
    sha256_hash_string(hash, outputBuffer);
    cleanup:
    if (file != NULL) fclose(file);
    free(buffer);
    return exit_code;
}

bool sha256(const char *filename, const char *hash) {
    static char outputBuffer[SHA256_BLOCK_BYTES];

    sha256_file(filename, outputBuffer);

    return strcmp(outputBuffer, hash) == 0;
}

/* bool sha512(const char *, const char *); */

#endif

#endif /* LIBACQUIRE_OPENSSL_H */

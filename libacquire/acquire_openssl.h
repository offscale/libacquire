#ifndef LIBACQUIRE_OPENSSL_H
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
#define BUF_SIZE 32768
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    FILE *file;
    unsigned short exit_code = EXIT_SUCCESS;
    unsigned char *sh256_buffer = malloc(BUF_SIZE);
    size_t bytesRead;

#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    exit_code = fopen_s(&file, path, "rb");
    if (exit_code) goto cleanup;
#else
    file = fopen(path, "rb");
    if (file == NULL) {
        exit_code = ENOENT;
        goto cleanup;
    }
#endif

    SHA256_Init(&sha256);

    if (!sh256_buffer) {
        exit_code = ENOMEM;
        goto cleanup;
    }
    for (; (bytesRead = fread(sh256_buffer, 1, BUF_SIZE, file)); SHA256_Update(&sha256, sh256_buffer, bytesRead)) {}


    SHA256_Final(hash, &sha256);
    sha256_hash_string(hash, outputBuffer);
cleanup:
    if (file != NULL) fclose(file);
    free(sh256_buffer);
    return exit_code;
#undef BUF_SIZE
}

bool sha256(const char *filename, const char *hash) {
    static char outputBuffer[SHA256_BLOCK_BYTES];

    sha256_file(filename, outputBuffer);

    return strcmp(outputBuffer, hash) == 0;
}

bool sha512(const char *filename, const char *hash) {
    fputs("TODO: Implement sha512", stderr);
    return false;
}

#endif /* defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL) */

#endif /* LIBACQUIRE_OPENSSL_H */

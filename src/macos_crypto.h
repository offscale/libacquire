#ifndef LIBACQUIRE_MACOS_CRYPTO_H
#define LIBACQUIRE_MACOS_CRYPTO_H

#include "config.h"

#ifdef USE_COMMON_CRYPTO

#include "checksums.h"

#include <string.h>
#include <stdio.h>
#include <CommonCrypto/CommonDigest.h>
#include <errno.h>

#define SHA256_DIGEST_LENGTH CC_SHA256_DIGEST_LENGTH
#define SHA256_CTX CC_SHA256_CTX
#define SHA256_Init CC_SHA256_Init
#define SHA256_Update CC_SHA256_Update
#define SHA256_Final CC_SHA256_Final

/*
void sha256_hash(const char *string, char outputBuffer[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    size_t i;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);

    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    outputBuffer[64] = 0;
}
*/

void sha256_hash_string(unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65]) {
    size_t i;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);

    outputBuffer[64] = 0;
}

int sha256_file(const char *path, char outputBuffer[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    FILE *file = fopen(path, "rb");
    const int bufSize = 32768;
    unsigned char *buffer = malloc(bufSize);
    unsigned short exit_code = 0;
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
    for (bytesRead = 0; (bytesRead = fread(buffer, 1, bufSize, file)); SHA256_Update(&sha256, buffer, bytesRead)) {}


    SHA256_Final(hash, &sha256);
    sha256_hash_string(hash, outputBuffer);
    printf("hash: \"%s\"\n", hash);
cleanup:
    file != NULL && fclose(file);
    free(buffer);
    return exit_code;
}

bool sha256(const char *filename, const char *hash) {
    static char outputBuffer[65];
    sha256_file(filename, outputBuffer);
    return strcmp(outputBuffer, hash) == 0;
}

/* bool sha512(const char *, const char *); */
#endif

#endif /* LIBACQUIRE_MACOS_CRYPTO_H */

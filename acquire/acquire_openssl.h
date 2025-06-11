/*
 * openssl implementation of libacquire's checksum API
 *
 * This should also work with OpenSSL derived distributions also, like:
 *   - LibreSSL
 *   - CommonCrypto (macOS)
 *
 * YMMV. Any issues, post to source GitHub.
 * */

#ifndef LIBACQUIRE_OPENSSL_H
#define LIBACQUIRE_OPENSSL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "acquire_config.h"

#if (defined(USE_COMMON_CRYPTO) && USE_COMMON_CRYPTO) ||                       \
    (defined(USE_OPENSSL) && USE_OPENSSL)

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

#elif defined(USE_OPENSSL) && USE_OPENSSL

#include <openssl/macros.h>
#include <openssl/sha.h>
#if OPENSSL_API_LEVEL >= 30000
#include <openssl/evp.h>
#include <openssl/types.h>
#else
#endif

#define SHA256_BLOCK_BYTES 64 /* block size in bytes */
#define SHA512_BLOCK_BYTES (SHA256_BLOCK_BYTES * 2)

#endif /* (defined(USE_COMMON_CRYPTO) && USE_COMMON_CRYPTO) ||                 \
          (defined(USE_OPENSSL) && USE_OPENSSL) */

#include <errno.h>
#include <stdio.h>

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_CRYPTO_IMPL)

int sha256_file(const char *filename,
                unsigned char sha_output[SHA256_DIGEST_LENGTH * 2 + 1]) {
  /* mostly from BSD licensed balrog/zanka-full */
  FILE *fp;
  SHA256_CTX sha256_context;
  size_t bytes;
  char buffer[BUFSIZ];
  int exit_code = EXIT_SUCCESS;

  /* zero out checksum */
  memset(sha_output, 0, SHA256_DIGEST_LENGTH * 2 + 1);

#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  fopen_s(&fp, filename, "r");
#else
  fp = fopen(filename, "r");
#endif /* defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) &&                   \
          __STDC_WANT_LIB_EXT1__ */

  if (!fp) {
    fprintf(stderr, "Could not open %s: %s", filename, strerror(errno));
    exit_code = ENOENT;
    goto cleanup;
  }

  /* calculate a SHA-1 checksum for files */
#if OPENSSL_API_LEVEL >= 30000
  {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned int md_len = 0;

    md = EVP_get_digestbyname("SHA2-256");

    mdctx = EVP_MD_CTX_new();
    if (!EVP_DigestInit_ex2(mdctx, md, NULL)) {
      fputs("Message digest initialization failed.", stderr);
      EVP_MD_CTX_free(mdctx);
      return 1;
    }
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)))
      if (!EVP_DigestUpdate(mdctx, &sha256_context, bytes)) {
        fputs("Message digest update failed.", stderr);
        EVP_MD_CTX_free(mdctx);
        return 1;
      } else
        md_len += bytes;
    if (!EVP_DigestFinal_ex(mdctx, sha_output, &md_len)) {
      fputs("Message digest finalization failed.", stderr);
      EVP_MD_CTX_free(mdctx);
      return 1;
    }
    EVP_MD_CTX_free(mdctx);
  }
#else
  {
    static unsigned char hex[] = "0123456789abcdef";
    unsigned char sha[SHA256_DIGEST_LENGTH];

    SHA256_Init(&sha256_context);

    /* the file is small enough, checksum it all */
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)))
      SHA256_Update(&sha256_context, buffer, bytes);

    SHA256_Final(sha, &sha256_context);

    /* map into hex characters */
    {
      int i;
      for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sha_output[i + i] = hex[sha[i] >> 4];
        sha_output[i + i + 1] = hex[sha[i] & 0x0F];
      }
      sha_output[i + i] = '\0';
    }
  }
#endif

cleanup:
  fclose(fp);

  return exit_code;
}

int sha512_file(const char *filename,
                unsigned char sha_output[SHA512_DIGEST_LENGTH * 2 + 1]) {
  /* mostly from BSD licensed balrog/zanka-full… should deduplicate with above
   * function */
  FILE *fp;
  SHA512_CTX sha512_context;
  size_t bytes;
  char buffer[BUFSIZ];
  int exit_code = EXIT_SUCCESS;

  /* zero out checksum */
  memset(sha_output, 0, SHA512_DIGEST_LENGTH * 2 + 1);

#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  fopen_s(&fp, filename, "r");
#else
  fp = fopen(filename, "r");
#endif /* defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) &&                   \
          __STDC_WANT_LIB_EXT1__ */

  if (!fp) {
    fprintf(stderr, "Could not open %s: %s", filename, strerror(errno));
    exit_code = ENOENT;
    goto cleanup;
  }

#if OPENSSL_API_LEVEL >= 30000
  {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned int md_len = 0;

    md = EVP_get_digestbyname("SHA2-512");

    mdctx = EVP_MD_CTX_new();
    if (!EVP_DigestInit_ex2(mdctx, md, NULL)) {
      fputs("Message digest initialization failed.", stderr);
      EVP_MD_CTX_free(mdctx);
      return 1;
    }
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)))
      if (!EVP_DigestUpdate(mdctx, &sha512_context, bytes)) {
        fputs("Message digest update failed.", stderr);
        EVP_MD_CTX_free(mdctx);
        return 1;
      } else
        md_len += bytes;
    if (!EVP_DigestFinal_ex(mdctx, sha_output, &md_len)) {
      fputs("Message digest finalization failed.", stderr);
      EVP_MD_CTX_free(mdctx);
      return 1;
    }
    EVP_MD_CTX_free(mdctx);
  }
#else
  {
    unsigned char sha[SHA512_DIGEST_LENGTH];
    static unsigned char hex[] = "0123456789abcdef";

    /* calculate a SHA-1 checksum for files */
    SHA512_Init(&sha512_context);

    /* the file is small enough, checksum it all */
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)))
      SHA512_Update(&sha512_context, buffer, bytes);

    SHA512_Final(sha, &sha512_context);

    /* map into hex characters */
    {
      int i;
      for (i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        sha_output[i + i] = hex[sha[i] >> 4];
        sha_output[i + i + 1] = hex[sha[i] & 0x0F];
      }
      sha_output[i + i] = '\0';
    }
  }
#endif

cleanup:
  fclose(fp);

  return exit_code;
}

#ifndef LIBACQUIRE_SHA256_IMPL
#define LIBACQUIRE_SHA256_IMPL
bool sha256(const char *filename, const char *hash) {
  unsigned char sha_output[SHA256_DIGEST_LENGTH * 2 + 1];
  sha256_file(filename, sha_output);

  return strcmp((const char *)sha_output, hash) == 0;
}
#endif /* !LIBACQUIRE_SHA256_IMPL */

#ifndef LIBACQUIRE_SHA512_IMPL
#define LIBACQUIRE_SHA512_IMPL
bool sha512(const char *filename, const char *hash) {
  unsigned char sha_output[SHA512_DIGEST_LENGTH * 2 + 1];
  sha512_file(filename, sha_output);

  return strcmp((const char *)sha_output, hash) == 0;
}
#endif /* !LIBACQUIRE_SHA512_IMPL */

#endif /* defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_CRYPTO_IMPL) */

#endif /* defined(USE_COMMON_CRYPTO) || defined(USE_OPENSSL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_OPENSSL_H */

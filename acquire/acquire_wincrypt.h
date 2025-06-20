/*
 * wincrypt implementation of libacquire's checksum API
 *
 * This should also work on Windows, ReactOS, and derivatives.
 * */

#ifndef LIBACQUIRE_WINCRYPT_H
#define LIBACQUIRE_WINCRYPT_H

#if defined(LIBACQUIRE_USE_WINCRYPT) && LIBACQUIRE_USE_WINCRYPT &&             \
    defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_CRYPTO_IMPL)

#include "acquire_checksums.h"
#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#define SHA256_BLOCK_BYTES 64 /* block size in bytes */
#define SHA512_BLOCK_BYTES (SHA256_BLOCK_BYTES * 2)

#include <stdio.h>
#include <string.h>

#include "acquire_config.h"

#include <intsafe.h>
#include <minwindef.h>
#include <winbase.h>
#include <wincrypt.h>
#include <windef.h>

/* SHA256 hash output length in bytes */
#define SHA256_HASH_BYTES 32
/* size for hex string + null */
#define SHA256_HASH_HEX_STR_LEN (SHA256_HASH_BYTES * 2 + 1)

LPTSTR get_error_message(DWORD dw) {
  LPVOID lpMsgBuf;
  if (dw == 0)
    dw = GetLastError();
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf, 0, NULL);
  return (LPTSTR)lpMsgBuf;
}

BOOL sha256_file(LPCSTR filename, CHAR hash[SHA256_HASH_HEX_STR_LEN]) {
  enum { BUFSIZE = 1024 };
  BOOL bResult = FALSE;
  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  HANDLE hFile = NULL;
  BYTE rgbFile[BUFSIZE];
  DWORD cbRead = 0;
  BYTE rgbHash[SHA256_HASH_BYTES];

  DWORD cbHash = SHA256_HASH_BYTES;
  static const CHAR rgbDigits[] = "0123456789abcdef";

  hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (INVALID_HANDLE_VALUE == hFile) {
    LPTSTR errStr = get_error_message(0);
    fprintf(stderr, "Error opening file %s\nError: %hs\n", filename, errStr);
    LocalFree(errStr);
    return FALSE;
  }

  if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT)) {
    LPTSTR errStr = get_error_message(0);
    fprintf(stderr, "CryptAcquireContext failed: %hs\n", errStr);
    LocalFree(errStr);
    CloseHandle(hFile);
    return FALSE;
  }

  if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
    LPTSTR errStr = get_error_message(0);
    fprintf(stderr, "CryptCreateHash failed: %hs\n", errStr);
    LocalFree(errStr);
    CloseHandle(hFile);
    CryptReleaseContext(hProv, 0);
    return FALSE;
  }

  while ((bResult = ReadFile(hFile, rgbFile, BUFSIZE, &cbRead, NULL)) &&
         cbRead > 0) {
    if (!CryptHashData(hHash, rgbFile, cbRead, 0)) {
      LPTSTR errStr = get_error_message(0);
      fprintf(stderr, "CryptHashData failed: %hs\n", errStr);
      LocalFree(errStr);

      CryptDestroyHash(hHash);
      CryptReleaseContext(hProv, 0);
      CloseHandle(hFile);
      return FALSE;
    }
  }

  if (!bResult) {
    LPTSTR errStr = get_error_message(0);
    fprintf(stderr, "ReadFile failed: %hs\n", errStr);
    LocalFree(errStr);

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
    return FALSE;
  }

  if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
    DWORD k = 0;
    for (DWORD i = 0; i < cbHash; i++) {
      hash[k++] = rgbDigits[rgbHash[i] >> 4];
      hash[k++] = rgbDigits[rgbHash[i] & 0xf];
    }
    hash[k] = '\0';
  } else {
    LPTSTR errStr = get_error_message(0);
    fprintf(stderr, "CryptGetHashParam failed: %hs\n", errStr);
    LocalFree(errStr);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
    return FALSE;
  }

  CryptDestroyHash(hHash);
  CryptReleaseContext(hProv, 0);
  CloseHandle(hFile);

  return TRUE;
}

/* returns true if the SHA256 hex string matches */
#ifndef LIBACQUIRE_SHA256_IMPL
#define LIBACQUIRE_SHA256_IMPL
bool sha256(const char *filename, const char *hash) {
  CHAR result[SHA256_HASH_HEX_STR_LEN];
  if (!sha256_file(filename, result))
    return false;

  if (!hash)
    return false;

  /* compare full length */
  if (strlen(hash) != (SHA256_HASH_HEX_STR_LEN - 1))
    return false;

  return (strncmp(hash, result, SHA256_HASH_HEX_STR_LEN - 1) == 0);
}
#endif /* !LIBACQUIRE_SHA256_IMPL */

#ifndef LIBACQUIRE_SHA512_IMPL
#define LIBACQUIRE_SHA512_IMPL
bool sha512(const char *filename, const char *hash) {
  fputs("SHA512 for wincrypt not implemented: always returns `false`\n",
        stderr);
  (void)filename;
  (void)hash;
  return false;
}
#endif /* !LIBACQUIRE_SHA512_IMPL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* defined(LIBACQUIRE_USE_WINCRYPT) && LIBACQUIRE_USE_WINCRYPT &&       \
          defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_CRYPTO_IMPL) */

#endif /* !LIBACQUIRE_WINCRYPT_H */

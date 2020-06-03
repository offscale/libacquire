#ifndef LIBACQUIRE_WINCRYPT_H
#define LIBACQUIRE_WINCRYPT_H

#pragma comment(lib, "crypt32.lib")

#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>
#include "checksums.h"
#include "stdbool.h"

#define BUFSIZE 1024
#define MD5LEN  16
#define SHA256LEN MD5LEN*2

bool sha256(const char *filename, const char *hash) {
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = NULL;
    BYTE rgbFile[BUFSIZE];
    DWORD cbRead = 0;
    BYTE rgbHash[SHA256LEN];
    DWORD cbHash = 0;
    CHAR rgbDigits[] = "0123456789abcdef";
    /* Logic to check usage goes here. */

    hFile = CreateFile(filename,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        dwStatus = GetLastError();
        fprintf(stderr, "Error opening file %s\nError: %lu\n", (LPCSTR) filename,
                dwStatus);
        return dwStatus;
    }

    /* Get handle to the crypto provider */
    if (!CryptAcquireContext(&hProv,
                             NULL,
                             NULL,
                             PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT)) {
        dwStatus = GetLastError();
        fprintf(stderr, "CryptAcquireContext failed: %lu\n", dwStatus);
        CloseHandle(hFile);
        return dwStatus;
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        dwStatus = GetLastError();
        fprintf(stderr, "CryptAcquireContext failed: %lu\n", dwStatus);
        CloseHandle(hFile);
        CryptReleaseContext(hProv, 0);
        return dwStatus;
    }

    while ((bResult = ReadFile(hFile, rgbFile, BUFSIZE,
                               &cbRead, NULL))) {
        if (0 == cbRead) {
            break;
        }

        if (!CryptHashData(hHash, rgbFile, cbRead, 0)) {
            dwStatus = GetLastError();
            fprintf(stderr, "CryptHashData failed: %lu\n", dwStatus);
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return dwStatus;
        }
    }

    if (!bResult) {
        dwStatus = GetLastError();
        fprintf(stderr, "ReadFile failed: %lu\n", dwStatus);
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
        return dwStatus;
    }

    cbHash = MD5LEN;
    if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
        printf("hash of \"%s\": ", filename);
        for (DWORD i = 0; i < cbHash; i++) {
            printf("%c%c", rgbDigits[rgbHash[i] >> 4],
                   rgbDigits[rgbHash[i] & 0xf]);
        }
        printf("\n");
    } else {
        dwStatus = GetLastError();
        printf("CryptGetHashParam failed: %lu\n", dwStatus);
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

    return dwStatus;
}

DWORD hash_fname_with_checksum(LPCWSTR filename, enum Checksum checksum, const char *hash) {
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = NULL;
    BYTE rgbFile[BUFSIZE];
    DWORD cbRead = 0;
    BYTE rgbHash[MD5LEN];
    DWORD cbHash = 0;
    CHAR rgbDigits[] = "0123456789abcdef";
    /* Logic to check usage goes here. */

    hFile = CreateFile((LPCSTR) filename,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        dwStatus = GetLastError();
        fprintf(stderr, "Error opening file %s\nError: %lu\n", (LPCSTR) filename,
                dwStatus);
        return dwStatus;
    }

    /* Get handle to the crypto provider */
    if (!CryptAcquireContext(&hProv,
                             NULL,
                             NULL,
                             PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT)) {
        dwStatus = GetLastError();
        fprintf(stderr, "CryptAcquireContext failed: %lu\n", dwStatus);
        CloseHandle(hFile);
        return dwStatus;
    }

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        dwStatus = GetLastError();
        fprintf(stderr, "CryptAcquireContext failed: %lu\n", dwStatus);
        CloseHandle(hFile);
        CryptReleaseContext(hProv, 0);
        return dwStatus;
    }

    while ((bResult = ReadFile(hFile, rgbFile, BUFSIZE,
                               &cbRead, NULL))) {
        if (0 == cbRead) {
            break;
        }

        if (!CryptHashData(hHash, rgbFile, cbRead, 0)) {
            dwStatus = GetLastError();
            fprintf(stderr, "CryptHashData failed: %lu\n", dwStatus);
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return dwStatus;
        }
    }

    if (!bResult) {
        dwStatus = GetLastError();
        fprintf(stderr, "ReadFile failed: %lu\n", dwStatus);
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
        return dwStatus;
    }

    cbHash = MD5LEN;
    if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
        printf("hash of \"%s\": ", filename);
        for (DWORD i = 0; i < cbHash; i++) {
            printf("%c%c", rgbDigits[rgbHash[i] >> 4],
                   rgbDigits[rgbHash[i] & 0xf]);
        }
        printf("\n");
    } else {
        dwStatus = GetLastError();
        printf("CryptGetHashParam failed: %lu\n", dwStatus);
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

    return dwStatus;
}

#endif /* LIBACQUIRE_WINCRYPT_H */

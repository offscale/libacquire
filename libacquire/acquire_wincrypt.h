/*
 * wincrypt implementation of libacquire's checksum API
 *
 * This should also work on Windows, ReactOS, and derivatives.
 * */

#if !defined(LIBACQUIRE_WINCRYPT_H) && defined(USE_WINCRYPT) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_WINCRYPT_H

#include <stdio.h>

#include "acquire_config.h"

#include <windef.h>
#include <WinBase.h>

#include <wincrypt.h>

#include "acquire_checksums.h"
#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* defined(HAS_STDBOOL) && !defined(bool) */

#define SHA256_BLOCK_BYTES       64          /* block size in bytes */
#define SHA512_BLOCK_BYTES       (SHA256_BLOCK_BYTES*2)

LPTSTR get_error_message(DWORD dw) {
    LPVOID lpMsgBuf;
    if (dw == 0)
        dw = GetLastError();
    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
    return (LPTSTR) lpMsgBuf;
}


BOOL sha256_file(LPCSTR filename,
                 CHAR hash[SHA256_BLOCK_BYTES + 1] /* plus nul char */)
{
#define BUFSIZE 1024
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = NULL;
    BYTE rgbFile[BUFSIZE];
    DWORD cbRead = 0;
    BYTE rgbHash[SHA256_BLOCK_BYTES];

    DWORD cbHash = 0;
    static const CHAR rgbDigits[] = "0123456789abcdef";

    hFile = CreateFile(filename,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        fprintf(stderr, "Error opening file %s\n"
                        "Error: %s\n", filename,
               get_error_message(0));
        return dwStatus;
    }

    // Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
                             NULL,
                             NULL,
                             PROV_RSA_AES,
                             CRYPT_VERIFYCONTEXT))
    {
        fprintf(stderr, "CryptAcquireContext failed: %s\n", get_error_message(0));
        CloseHandle(hFile);
        return dwStatus;
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        fprintf(stderr, "CryptCreateHash failed: %s\n",  get_error_message(0));
        CloseHandle(hFile);
        CryptReleaseContext(hProv, 0);
        return dwStatus;
    }

    while ((bResult = ReadFile(hFile, rgbFile, BUFSIZE,
                               &cbRead, NULL)))
    {
        if (0 == cbRead)
        {
            break;
        }

        if (!CryptHashData(hHash, rgbFile, cbRead, 0))
        {
            fprintf(stderr, "CryptHashData failed: %s\n", get_error_message(0));
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return dwStatus;
        }
    }

    if (!bResult)
    {
        fprintf(stderr, "ReadFile failed: %s\n",  get_error_message(0));
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
        return dwStatus;
    }

    cbHash = SHA256_BLOCK_BYTES;
    if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
    {
        DWORD k = 0;
        for (DWORD i = 0; i < cbHash; i++)
        {
            hash[k++] = rgbDigits[rgbHash[i] >> 4];
            hash[k++] = rgbDigits[rgbHash[i] & 0xf];
        }
        hash[k] = '\0';
    }
    else
        fprintf(stderr, "CryptGetHashParam failed: %s\n", get_error_message(0));

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

    return bResult;
#undef BUFSIZE
#undef MD5LEN
}


bool sha256(const char *filename, const char *hash) {
    CHAR result[SHA256_BLOCK_BYTES + 1];
    sha256_file(filename, result);
    return strcmp(hash, result) == 0;
}

bool sha512(const char *filename, const char *hash) {
    return false;
}

#endif /* !defined(LIBACQUIRE_WINCRYPT_H) && defined(USE_WINCRYPT) && defined(LIBACQUIRE_IMPLEMENTATION) */

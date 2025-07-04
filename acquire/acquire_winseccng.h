/*
 * winseccng implementation of libacquire's checksum API
 *
 * This should also work on Windows, ReactOS, and derivatives.
 *
 * Very much a WiP
 * */

#if !defined(LIBACQUIRE_WINSECCNG_H) && defined(LIBACQUIRE_USE_WINSECCNG) &&   \
    LIBACQUIRE_USE_WINSECCNG && defined(LIBACQUIRE_IMPLEMENTATION) &&          \
    defined(LIBACQUIRE_CRYPTO_IMPL)
#define LIBACQUIRE_WINSECCNG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <minwindef.h>
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#include <bcrypt.h>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)

static const BYTE rgbMsg[] = {0x61, 0x62, 0x63};

void __cdecl wmain(int argc, __in_ecount(argc) LPWSTR *wargv) {

  BCRYPT_ALG_HANDLE hAlg = NULL;
  BCRYPT_HASH_HANDLE hHash = NULL;
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  DWORD cbData = 0, cbHash = 0, cbHashObject = 0;
  PBYTE pbHashObject = NULL;
  PBYTE pbHash = NULL;

  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(wargv);

  /* open an algorithm handle */
  if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
                      &hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0))) {
    wprintf(L"**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n",
            status);
    goto Cleanup;
  }

  /* calculate the size of the buffer to hold the hash object */
  if (!NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH,
                                             (PBYTE)&cbHashObject,
                                             sizeof(DWORD), &cbData, 0))) {
    wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
    goto Cleanup;
  }

  /* allocate the hash object on the heap */
  pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
  if (NULL == pbHashObject) {
    wprintf(L"**** memory allocation failed\n");
    goto Cleanup;
  }

  /* calculate the length of the hash */
  if (!NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH,
                                             (PBYTE)&cbHash, sizeof(DWORD),
                                             &cbData, 0))) {
    wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
    goto Cleanup;
  }

  /* allocate the hash buffer on the heap */
  pbHash = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHash);
  if (NULL == pbHash) {
    wprintf(L"**** memory allocation failed\n");
    goto Cleanup;
  }

  /* create a hash */
  if (!NT_SUCCESS(status = BCryptCreateHash(hAlg, &hHash, pbHashObject,
                                            cbHashObject, NULL, 0, 0))) {
    wprintf(L"**** Error 0x%x returned by BCryptCreateHash\n", status);
    goto Cleanup;
  }

  /* hash some data */
  if (!NT_SUCCESS(
          status = BCryptHashData(hHash, (PBYTE)rgbMsg, sizeof(rgbMsg), 0))) {
    wprintf(L"**** Error 0x%x returned by BCryptHashData\n", status);
    goto Cleanup;
  }

  /* close the hash */
  if (!NT_SUCCESS(status = BCryptFinishHash(hHash, pbHash, cbHash, 0))) {
    wprintf(L"**** Error 0x%x returned by BCryptFinishHash\n", status);
    goto Cleanup;
  }

  wprintf(L"Success!\n");

Cleanup:

  if (hAlg) {
    BCryptCloseAlgorithmProvider(hAlg, 0);
  }

  if (hHash) {
    BCryptDestroyHash(hHash);
  }

  if (pbHashObject) {
    HeapFree(GetProcessHeap(), 0, pbHashObject);
  }

  if (pbHash) {
    HeapFree(GetProcessHeap(), 0, pbHash);
  }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(LIBACQUIRE_WINSECCNG_H) &&                                  \
          defined(LIBACQUIRE_USE_WINSECCNG) && LIBACQUIRE_USE_WINSECCNG        \
          defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          defined(LIBACQUIRE_CRYPTO_IMPL) */

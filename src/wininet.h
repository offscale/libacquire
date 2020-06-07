#ifndef LIBACQUIRE_WININET_H
#define LIBACQUIRE_WININET_H

#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include "stdbool.h"


bool GetFile(HINTERNET, const TCHAR *, const TCHAR *, TCHAR *);

const char *get_download_dir() {
    return ".downloads";
}

bool GetFile(HINTERNET hOpen,      /* Handle from InternetOpen() */
             const TCHAR *szURL,   /* Full URL */
             const TCHAR *szpath,
             TCHAR *szname) {
    DWORD dwSize;
    TCHAR szHead[15];
    BYTE *szTemp[1024];
    HINTERNET hConnect;
    FILE *pFile;
    TCHAR szpathfilename[100];

    szHead[0] = '\0';

    if (!(hConnect = InternetOpenUrl(hOpen, szURL, szHead, 15, INTERNET_FLAG_DONT_CACHE, 0))) {
        fprintf(stderr, "Error: InternetOpenUrl\n");
        return 0;
    }

    if (szname == NULL || strlen(szname) == 0) szname = (char*)get_path_from_url(szURL);

    fopen_s(&pFile, szpathfilename, "wb");
    if (pFile == NULL) {
        fprintf(stderr, "Error _tfopen\n");
        return false;
    }
    do {
        /* Keep copying in 1024 bytes chunks, while file has any data left.
           Note: bigger buffer will greatly improve performance. */
        if (!InternetReadFile(hConnect, szTemp, 1024, &dwSize)) {
            fclose(pFile);
            fprintf(stderr, "Error InternetReadFile\n");
            return FALSE;
        }
        if (dwSize == 0)
            break;  /* Condition of dwSize=0 indicate EOF. Stop. */
        else
            fwrite(szTemp, sizeof(BYTE), dwSize, pFile);
    }   /* do */
    while (TRUE);
    fflush(pFile);
    fclose(pFile);

    return TRUE;
}

int download(const char* url, enum Checksum checksum, const char* hash, const char target_directory[248],
    bool follow, size_t retry, size_t verbosity) {

    HINTERNET hInternet;

    hInternet = InternetOpen("libacquire",               /* __in LPCSTR lpszAgent       */
                             INTERNET_OPEN_TYPE_DIRECT,  /* __in DWORD  dwAccessType    */
                             NULL,                       /* __in LPCSTR lpszProxy       */
                             NULL,                       /* __in LPCSTR lpszProxyBypass */
                             0                           /* __in DWORD  dwFlags         */
    );
    GetFile(hInternet, url, get_download_dir(), NULL);
    InternetCloseHandle(hInternet);

    return EXIT_SUCCESS;
}

#endif /* LIBACQUIRE_WININET_H */

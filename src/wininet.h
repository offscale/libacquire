#ifndef LIBACQUIRE_WININET_H
#define LIBACQUIRE_WININET_H

#include <windows.h>
#include <wininet.h>
#include <corecrt_wstdio.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include "stdbool.h"

int DownloadURLImage(const TCHAR *, const TCHAR *, TCHAR *);

bool GetFile(HINTERNET, const TCHAR *, const TCHAR *, TCHAR *);

int convertURLtofname(const TCHAR *, TCHAR *);

int determinepathfilename(const TCHAR *, const TCHAR *, TCHAR *, TCHAR *);

int convertURLtofname(const TCHAR *szURL, TCHAR *szname)
/* extract the filename from the URL */
{
    char aszfilename[100];
    HRESULT result;
    char achar[3], aszURL[100];
    size_t nchars, i, j;
    int fresult;

    fresult = 0;

    nchars = strlen(szURL);
    i = nchars - 1;
    while ((i > 0) && (szURL[i] != '/') && (szURL[i] != '\\')) { i--; }
    j = 0;
    i++;
    while (i < nchars) { szname[j++] = szURL[i++]; }
    szname[j] = '\0';

/*  wcstombs ( aszfilename, szname, 100 );
       printf("%s\n", aszfilename);
  ---------------------------------------------- */
    return fresult;
}

int determinepathfilename(const TCHAR *szURL, const TCHAR *szpath, TCHAR *szname, TCHAR *szpathfilename) {
/* use path and filename when supplied.  If filename (e.g. funkypic.jpg) is not supplied, then the
   filename will be extracted from the last part of the URL */
    int result;
    result = 0;
    TCHAR szname_copy[100];

    if ((szname == NULL) || (szname[0] == '\0'))
        convertURLtofname(szURL, szname_copy);
    else
        _tcscpy(szname_copy, szname);

    if ((szpath == NULL) || (szpath[0] == '\0'))
        _tcscpy(szpathfilename, szname_copy);
    else {
        _tcscpy(szpathfilename, szpath);
        _tcscat(szpathfilename, szname_copy);
    }
    return result;
}

bool GetFile(HINTERNET hOpen, /* Handle from InternetOpen() */
             const TCHAR *szURL,        /* Full URL */
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

    determinepathfilename(szURL, szpath, szname, szpathfilename);

    if (!(pFile = _tfopen(szpathfilename, "wb"))) {
        fprintf(stderr, "Error _tfopen\n", stderr);
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

int DownloadURLImage(const TCHAR *szURL, const TCHAR *szpath, TCHAR *szname) {
    int result;

    HINTERNET hInternet;

    result = 0;

    hInternet = InternetOpen("libacquire",
                             INTERNET_OPEN_TYPE_DIRECT,  /* __in  DWORD dwAccessType */
                             NULL,                       /* __in  LPCTSTR lpszProxyName,  */
                             NULL,                       /* __in  LPCTSTR lpszProxyBypass,  */
                             (DWORD) NULL                /* __in   DWORD dwFlags  */
    );

    GetFile(hInternet, szURL, szpath, szname);
    InternetCloseHandle(hInternet);
    return result;
}

int download(const char *url, enum Checksum checksum, const char *hash, const char target_directory[248],
             bool follow, size_t retry, size_t verbosity) {
    return DownloadURLImage(url, target_directory, NULL);
}

#endif /* LIBACQUIRE_WININET_H */

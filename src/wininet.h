#ifndef LIBACQUIRE_WININET_H
#define LIBACQUIRE_WININET_H

#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include "stdbool.h"

#define BUFFER_SIZE 4096

const char *get_download_dir() {
    return ".downloads";
}

int download(const char* url, enum Checksum checksum, const char* hash, const char target_directory[248],
    bool follow, size_t retry, size_t verbosity)
{
    HINTERNET hInternet, hURL;
    DWORD file_size, nbread;
    FILE* fp = NULL;
    char* buff;
    size_t count;
    const char* file_name = get_path_from_url(url);

    hInternet = InternetOpen("QuickGet", 0, 0, 0, 0);
    if (hInternet == NULL)
        return -1;
    hURL = InternetOpenUrl(hInternet, url, 0, 0, 0, 0);
    if (hURL == NULL)
        return -1;

    file_size = InternetSetFilePointer(hURL, 0, 0, FILE_END, 0);
    if (file_size == -1)
        return -1;

    InternetSetFilePointer(hURL, 0, 0, FILE_BEGIN, 0);

    
    fopen_s(&fp, file_name, "wb");
    if (fp == NULL)
        return -1;


    puts("Starting download...");

    nbread = 0;
    buff = malloc(BUFFER_SIZE);
    count = 0;
    while (count < file_size)
    {
        InternetReadFile(hURL, buff, BUFFER_SIZE, &nbread);
        fwrite(buff, nbread, 1, fp);
        count += nbread;
        printf("\r%zu of %lu bytes downloded from %s.", count, file_size, url);
    }
    free(buff);
    fclose(fp);
    InternetCloseHandle(hURL); InternetCloseHandle(hInternet);
    return 0;
}

#endif /* LIBACQUIRE_WININET_H */

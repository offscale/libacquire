/*
 * wininet implementation of libacquire's download API
 *
 * This should also work on Windows, ReactOS, and derivatives.
 * */

#if !defined(LIBACQUIRE_WININET_H) && defined(USE_WININET) &&                  \
    defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_WININET_H

#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#include "acquire_checksums.h"

#ifndef NAME_MAX
#ifdef PATH_MAX
#define NAME_MAX PATH_MAX
#else
#define NAME_MAX 4096
#endif
#endif

#include "acquire_url_utils.h"
#include <stdio.h>
#include <tchar.h>
#include <wininet.h>

#define BUFFER_SIZE 4096

const char *get_download_dir() { return TMPDIR "//.downloads"; }

int download(const char *url, enum Checksum checksum, const char *hash,
             const char target_location[NAME_MAX], bool follow, size_t retry,
             size_t verbosity) {
  HINTERNET hInternet, hURL;
  DWORD file_size, nbread;
  FILE *fp = NULL;
  char *buff;
  size_t count;
  const char *file_name = get_path_from_url(url);

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
  while (count < file_size) {
    InternetReadFile(hURL, buff, BUFFER_SIZE, &nbread);
    fwrite(buff, nbread, 1, fp);
    count += nbread;
    printf("\r%zu of %lu bytes downloaded from %s.", count, file_size, url);
  }
  free(buff);
  fclose(fp);
  InternetCloseHandle(hURL);
  InternetCloseHandle(hInternet);
  return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(LIBACQUIRE_WININET_H) && defined(USE_WININET) &&            \
          defined(LIBACQUIRE_IMPLEMENTATION) */

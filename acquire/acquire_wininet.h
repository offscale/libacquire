/*
 * wininet implementation of libacquire's download API
 *
 * This should also work on Windows, ReactOS, and derivatives.
 * */

#if !defined(LIBACQUIRE_WININET_H) && defined(LIBACQUIRE_USE_WININET) &&       \
    LIBACQUIRE_USE_WININET && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_WININET_H

#include <minwindef.h>
#include <windef.h>

#ifdef __cplusplus
extern "C" {
#elif defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif /* __cplusplus */

#include "acquire_config.h"
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

#ifndef LIBACQUIRE_DOWNLOAD_DIR_IMPL
#define LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return TMPDIR "//.downloads"; }
#endif /* !LIBACQUIRE_DOWNLOAD_DIR_IMPL */

#ifdef LIBACQUIRE_DOWNLOAD_IMPL
int download(const char *url, enum Checksum checksum, const char *hash,
             const char *target_location /*[NAME_MAX]*/, bool follow,
             size_t retry, size_t verbosity) {
  HINTERNET hInternet = NULL, hURL = NULL;
  DWORD bytesAvailable = 0, nbread = 0;
  FILE *fp = NULL;
  char *buff = NULL;
  size_t count = 0;
  const char *file_name = get_path_from_url(url);

  hInternet =
      InternetOpen("QuickGet", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  if (hInternet == NULL)
    return -1;

  hURL = InternetOpenUrl(hInternet, url, NULL, 0,
                         INTERNET_FLAG_SECURE |
                             INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
                             INTERNET_FLAG_IGNORE_CERT_CN_INVALID,
                         0);
  if (hURL == NULL) {
    InternetCloseHandle(hInternet);
    return -1;
  }

  /* Compose full path for local file */
  char full_local_fname[NAME_MAX + 1];
  if (strlen(target_location) + 1 + strlen(file_name) >=
      sizeof(full_local_fname)) {
    InternetCloseHandle(hURL);
    InternetCloseHandle(hInternet);
    return -1;
  }
  snprintf(full_local_fname, sizeof(full_local_fname), "%s%c%s",
           target_location, '\\', file_name);

#if defined(_MSC_VER) || (defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__)
  if (fopen_s(&fp, full_local_fname, "wb") != 0 || fp == NULL) {
#else
  fp = fopen(full_local_fname, "wb");
  if (!fp) {
#endif
    InternetCloseHandle(hURL);
    InternetCloseHandle(hInternet);
    return -1;
  }

  buff = (char *)malloc(BUFFER_SIZE);
  if (!buff) {
    fclose(fp);
    InternetCloseHandle(hURL);
    InternetCloseHandle(hInternet);
    return -1;
  }

  puts("Starting download...");

  do {
    if (!InternetQueryDataAvailable(hURL, &bytesAvailable, 0, 0) ||
        bytesAvailable == 0) {
      /* No more data or error */
      break;
    }

    if (bytesAvailable > BUFFER_SIZE)
      bytesAvailable = BUFFER_SIZE;

    if (!InternetReadFile(hURL, buff, bytesAvailable, &nbread) || nbread == 0) {
      /* read error */
      break;
    }

    if (fwrite(buff, 1, nbread, fp) != nbread) {
      /* write error */
      break;
    }

    count += nbread;
    printf("\r%zu bytes downloaded from %s.", count, url);

  } while (nbread > 0);

  free(buff);
  fclose(fp);
  InternetCloseHandle(hURL);
  InternetCloseHandle(hInternet);

  return 0;
}
#endif /* LIBACQUIRE_DOWNLOAD_IMPL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(LIBACQUIRE_WININET_H) && defined(LIBACQUIRE_USE_WININET) && \
          LIBACQUIRE_USE_WININET && defined(LIBACQUIRE_IMPLEMENTATION) */

#ifndef LIBACQUIRE_ACQUIRE_WINCOMPRESSAPI_H
#define LIBACQUIRE_ACQUIRE_WINCOMPRESSAPI_H

#if defined(LIBACQUIRE_USE_WINCOMPRESSAPI) && LIBACQUIRE_USE_WINCOMPRESSAPI && \
    defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_EXTRACT_IMPL) &&  \
    (defined(_WIN32) || defined(__WIN32__) || defined(_MSC_VER) ||             \
     defined(__MINGW32__))

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>

#include <compressapi.h> /* Windows Compression API */
#include <errhandlingapi.h>
#include <fileapi.h>
#include <minwindef.h>
#include <stringapiset.h>
#include <winerror.h>

#include "acquire_extract.h"

/* ZIP local file header signature */
#define ZIP_LOCAL_FILE_HEADER_SIGNATURE 0x04034b50

/* Max path length */
#define MAX_PATH_LEN 260

/* Structure for ZIP local file header, packed */
#pragma pack(push, 1)
typedef struct _ZIP_LOCAL_FILE_HEADER {
  DWORD signature;
  WORD version_needed;
  WORD flags;
  WORD compression_method;
  WORD last_mod_time;
  WORD last_mod_date;
  DWORD crc32;
  DWORD compressed_size;
  DWORD uncompressed_size;
  WORD filename_length;
  WORD extra_field_length;
  /* Followed by filename and extra field */
} ZIP_LOCAL_FILE_HEADER;
#pragma pack(pop)

/* Helper: create directories recursively */
static int create_directory_recursive(const wchar_t *path) {
  wchar_t partial_path[MAX_PATH_LEN];
  size_t i, len;
  DWORD dwErr;

  if (!path)
    return 0;

  len = wcslen(path);
  for (i = 0; i < len; ++i) {
    partial_path[i] = path[i];
    if (path[i] == L'\\' || path[i] == L'/') {
      partial_path[i + 1] = L'\0';
      if (!CreateDirectoryW(partial_path, NULL)) {
        dwErr = GetLastError();
        if (dwErr != ERROR_ALREADY_EXISTS)
          return 0;
      }
    }
  }
  /* Create full dir */
  partial_path[len] = L'\0';
  if (!CreateDirectoryW(partial_path, NULL)) {
    dwErr = GetLastError();
    if (dwErr != ERROR_ALREADY_EXISTS)
      return 0;
  }
  return 1;
}

/* Convert UTF-8 to UTF-16 inline */
static wchar_t *utf8_to_utf16_alloc(const char *utf8) {
  int size;
  wchar_t *wstr;
  if (!utf8)
    return NULL;

  size = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
  if (size == 0)
    return NULL;
  wstr = (wchar_t *)malloc(sizeof(wchar_t) * size);
  if (!wstr)
    return NULL;
  if (MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, size) == 0) {
    free(wstr);
    return NULL;
  }
  return wstr;
}

/* Extract archive, only supports STORE compression method (0) */
int extract_archive(enum Archive archive, const char *archive_filepath,
                    const char *output_folder) {
  FILE *fzip = NULL;
  ZIP_LOCAL_FILE_HEADER header;
  wchar_t *outdir_w = NULL;
  wchar_t *filew = NULL;
  wchar_t fullpath[MAX_PATH_LEN];
  char *filename = NULL;
  int ret = EXIT_FAILURE;

  size_t readbytes, i;
  unsigned char *buffer = NULL;

  if (archive != LIBACQUIRE_ZIP)
    return EXIT_FAILURE;

  {
    const errno_t err = fopen_s(&fzip, archive_filepath, "rb");
    ;
    if (err != 0 || fzip == NULL) {
      fprintf(stderr, "Failed to open %s for reading", archive_filepath);
      free(fzip);
      ret = EXIT_FAILURE;
      goto cleanup;
    }
  }

  outdir_w = utf8_to_utf16_alloc(output_folder);
  if (!outdir_w) {
    fprintf(stderr, "Failed to convert output folder to wide string\n");
    goto cleanup;
  }

  buffer = (unsigned char *)malloc(65536);
  if (!buffer) {
    fprintf(stderr, "Failed to allocate buffer\n");
    goto cleanup;
  }

  while (1) {
    /* Read and validate local file header */
    readbytes = fread(&header, 1, sizeof(header), fzip);
    if (readbytes < sizeof(header))
      break; /* EOF */

    if (header.signature != ZIP_LOCAL_FILE_HEADER_SIGNATURE) {
      fprintf(stderr, "Invalid ZIP local file header signature\n");
      goto cleanup;
    }

    /* Read filename */
    filename = (char *)malloc(header.filename_length + 1);
    if (!filename) {
      fprintf(stderr, "Failed to allocate filename buffer\n");
      goto cleanup;
    }
    if (fread(filename, 1, header.filename_length, fzip) !=
        header.filename_length) {
      fprintf(stderr, "Failed to read filename\n");
      free(filename);
      goto cleanup;
    }
    filename[header.filename_length] = '\0';

    /* Skip extra field */
    if (header.extra_field_length > 0) {
      if (fseek(fzip, header.extra_field_length, SEEK_CUR) != 0) {
        fprintf(stderr, "Failed to skip extra field\n");
        free(filename);
        goto cleanup;
      }
    }

    /* Create output path */
    filew = utf8_to_utf16_alloc(filename);
    free(filename);
    filename = NULL;
    if (!filew) {
      fprintf(stderr, "Failed to convert filename to wide string\n");
      goto cleanup;
    }

    /* Compose full output path: outdir\filename */
    if (swprintf(fullpath, MAX_PATH_LEN, L"%s\\%s", outdir_w, filew) < 0) {
      fprintf(stderr, "Path too long\n");
      free(filew);
      goto cleanup;
    }

    /* Check if directory entry: filename ends with '\' */
    i = wcslen(filew);
    if (i > 0 && (filew[i - 1] == L'\\' || filew[i - 1] == L'/')) {
      /* Directory entry - create directory */
      if (!create_directory_recursive(fullpath)) {
        fprintf(stderr, "Failed to create directory %ls\n", fullpath);
        free(filew);
        goto cleanup;
      }
      free(filew);
      continue;
    }

    free(filew);

    /* Make sure directory for file exists */
    {
      wchar_t *last_sep = wcsrchr(fullpath, L'\\');
      if (last_sep) {
        wchar_t saved_char = *last_sep;
        *last_sep = L'\0';
        if (!create_directory_recursive(fullpath)) {
          fprintf(stderr, "Failed to create directory for file %ls\n",
                  fullpath);
          *last_sep = saved_char;
          goto cleanup;
        }
        *last_sep = saved_char;
      }
    }

    /* Check compression method */
    if (header.compression_method != 0) {
      fprintf(stderr, "Unsupported compression method %d for file %ls\n",
              header.compression_method, fullpath);
      /* Skip compressed data */
      if (fseek(fzip, header.compressed_size, SEEK_CUR) != 0) {
        fprintf(stderr, "Failed to skip compressed data\n");
        goto cleanup;
      }
      continue;
    }

    /* Extract file (compression method 0 == store) */
    {
      FILE *fout = _wfopen(fullpath, L"wb");
      if (!fout) {
        fprintf(stderr, "Failed to open output file %ls\n", fullpath);
        goto cleanup;
      }

      /* Read compressed_size bytes and write directly */
      DWORD remaining = header.compressed_size;
      size_t to_read;
      while (remaining > 0) {
        to_read = (remaining > 65536) ? 65536 : remaining;
        readbytes = fread(buffer, 1, to_read, fzip);
        if (readbytes != to_read) {
          fprintf(stderr, "Failed to read compressed data\n");
          fclose(fout);
          goto cleanup;
        }
        if (fwrite(buffer, 1, readbytes, fout) != readbytes) {
          fprintf(stderr, "Failed to write output data\n");
          fclose(fout);
          goto cleanup;
        }
        remaining -= (DWORD)readbytes;
      }
      fclose(fout);
    }
  }

  ret = EXIT_SUCCESS;

cleanup:

  if (fzip)
    fclose(fzip);
  if (outdir_w)
    free(outdir_w);
  if (filename)
    free(filename);
  if (filew)
    free(filew);
  if (buffer)
    free(buffer);

  return ret;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* defined(LIBACQUIRE_USE_WINCOMPRESSAPI) &&                            \
          LIBACQUIRE_USE_WINCOMPRESSAPI && defined(LIBACQUIRE_IMPLEMENTATION)                                            \
          && defined(LIBACQUIRE_EXTRACT_IMPL) && (defined(_WIN32) ||                                                           \
          defined(__WIN32__) || defined(_MSC_VER) || defined(__MINGW32__)) */

#endif /* !LIBACQUIRE_ACQUIRE_WINCOMPRESSAPI_H */

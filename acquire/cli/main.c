#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIBACQUIRE_IMPLEMENTATION

#include <acquire_common_defs.h>
#include <acquire_config.h>
#include <acquire_download.h>
#include <acquire_errors.h>
#include <acquire_net_common.h>

#include "cli.h"

#ifdef USE_CRC32C
#include <acquire_crc32c.h>
#elif defined(USE_LIBRHASH)
#include <acquire_librhash.h>
#endif

#ifdef USE_LIBCURL

#include <acquire_libcurl.h>

#elif defined(USE_WININET)

#include <acquire_wininet.h>

#elif defined(USE_LIBFETCH)

#include <acquire_libfetch.h>

#elif defined(USE_OPENBSD_FTP)

#include <acquire_openbsd_ftp.h>

#endif

#if defined(USE_OPENSSL) || defined(USE_LIBRESSL) || defined(USE_COMMON_CRYPTO)
#include <acquire_openssl.h>
#elif defined(USE_WINCRYPT)
#include <acquire_wincrypt.h>
#endif

int main(int argc, char *argv[]) {
  int rc = EXIT_SUCCESS;
  struct DocoptArgs *args = malloc(sizeof *args);
  enum Checksum checksum = LIBACQUIRE_SHA256;
  char output_full_path[NAME_MAX + 1];
  const char *check_env = NULL;
  int i;
  if (args == NULL) {
    fputs("Out of memory\n", stderr);
    return ENOMEM;
  } else {
    rc = docopt(args, argc, argv, /* help */ true,
                /* version */ VERSION);
    if (rc != EXIT_SUCCESS) {
      free(args);
      return rc;
    }
  }

  /* Ensure environment variable CHECK is read safely as a bool */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    char *check_buf = NULL;
    size_t check_len = 0;
    errno_t err = _dupenv_s(&check_buf, &check_len, "CHECK");
    if (err == 0 && check_buf != NULL && check_len > 0) {
      check_env = check_buf;
    }
  }
#else
  check_env = getenv("CHECK");
#endif

  if (check_env != NULL && args->check == 0) {
    if (check_env[0] == '1' || check_env[0] == 't' || check_env[0] == 'T' ||
        check_env[0] == 'y' || check_env[0] == 'Y') {
      args->check = true;
    }
  }

  if (args->output != NULL && args->directory != NULL) {
    size_t dir_len = strlen(args->directory);
    size_t out_len = strlen(args->output);

    if (dir_len + 1 + out_len >= sizeof(output_full_path)) {
      fprintf(stderr, "Output path too long.\n");
      return 1;
    }

    snprintf(output_full_path, sizeof(output_full_path), "%s%s%s",
             args->directory,
             (args->directory[dir_len - 1] == PATH_SEP[0]) ? "" : PATH_SEP,
             args->output);
    args->output = output_full_path;
  } else if (args->output == NULL && args->directory == NULL) {
    args->directory = (char *)TMPDIR;
    args->output = (char *)TMPDIR;
  } else if (args->output == NULL) {
    args->output = args->directory;
  } else if (args->directory == NULL) {
    args->directory = args->output;
  }

  if (args->url == NULL) {
    for (i = argc - 1; i > 0; i--) {
      if (is_url(argv[i])) {
        args->url = argv[i];
        break;
      }
    }
    if (args->url == NULL) {
      fprintf(stderr, "No valid URL specified.\n");
      return UNIMPLEMENTED;
    }
  }

  if (args->checksum != NULL) {
    checksum = string2checksum((const char *)args->checksum);
    if (checksum == LIBACQUIRE_UNSUPPORTED_CHECKSUM) {
      fprintf(stderr, "Unsupported checksum algorithm: %s\n", args->checksum);
      return UNIMPLEMENTED;
    }
  }

  if (args->check) {
    return is_downloaded(args->url, checksum, args->hash, args->output)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
  }

  return download(args->url, checksum, args->hash, args->output, false, 0, 0);
}

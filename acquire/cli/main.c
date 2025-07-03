/* acquire/cli/main.c */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "acquire_checksums.h"
#include "acquire_common_defs.h"
#include "acquire_config.h"
#include "acquire_download.h"
#include "acquire_handle.h"
#include "acquire_net_common.h"
#include "acquire_url_utils.h"

#include "cli.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <minwindef.h>
#endif

int main(int argc, char *argv[]) {
  struct DocoptArgs args;
  struct acquire_handle *handle;
  int rc = EXIT_SUCCESS;
  char *url_to_use;
  char *output_path;
  char *output_path_alloc = NULL;
  char final_output_path_buf[NAME_MAX + 1];

  memset(&args, 0, sizeof(args));
  rc = docopt(&args, argc, argv, 1, LIBACQUIRE_VERSION);
  if (rc != EXIT_SUCCESS) {
    return rc;
  }

  handle = acquire_handle_init();
  if (handle == NULL) {
    fputs("Out of memory: could not create acquire handle.\n", stderr);
    return ENOMEM;
  }

  url_to_use = NULL;
  if (args.url != NULL) {
    url_to_use = args.url;
  } else {
    int i;
    for (i = argc - 1; i > 0; i--) {
      if (is_url(argv[i])) {
        url_to_use = argv[i];
        break;
      }
    }
  }

  if (url_to_use == NULL) {
    fprintf(stderr, "Error: No valid URL specified.\n");
    rc = EXIT_FAILURE;
    goto cleanup;
  }

  if (args.output != NULL) {
    output_path = args.output;
  } else {
    output_path_alloc = get_path_from_url(url_to_use);
    if (output_path_alloc == NULL || *output_path_alloc == '\0') {
      fprintf(stderr,
              "Error: Could not determine filename from URL and no "
              "output directory was specified.\n");
      rc = EXIT_FAILURE;
      goto cleanup;
    }
    if (args.directory != NULL) {
      snprintf(final_output_path_buf, sizeof(final_output_path_buf), "%s%s%s",
               args.directory, PATH_SEP, output_path_alloc);
      output_path = final_output_path_buf;
    } else {
      output_path = output_path_alloc;
    }
  }

  if (args.check) {
    if (is_downloaded(url_to_use, string2checksum(args.checksum), args.hash,
                      args.directory)) {
      printf("File is already downloaded and verified.\n");
      rc = EXIT_SUCCESS;
    } else {
      printf("File is not downloaded or checksum mismatch.\n");
      rc = EXIT_FAILURE;
    }
  } else {
    printf("Downloading '%s' to '%s'...\n", url_to_use, output_path);
    if (acquire_download_sync(handle, url_to_use, output_path) != 0) {
      fprintf(stderr, "Download failed: %s\n",
              acquire_handle_get_error_string(handle));
      rc = EXIT_FAILURE;
    } else {
      printf("Download complete.\n");
      rc = EXIT_SUCCESS;
    }
  }

cleanup:
  acquire_handle_free(handle);
  free(output_path_alloc);
  return rc;
}

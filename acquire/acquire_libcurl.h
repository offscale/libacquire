/*
 * libcurl implementation of libacquire's download API
 *
 * Thanks to cmake/FindCurlCustom this should work with:
 *   - Apple macOS (via `brew`, `port`, system, &etc.)
 *   - Linux, BSDs, SunOS (via system package manager, &etc.)
 *   - Windows (via conan, vcpkg, &etc.)
 *   - Elsewhere that curl supports (not tested, but shouldn't be an issue)
 * */

#if !defined(LIBACQUIRE_LIBCURL_H) && defined(USE_LIBCURL) &&                  \
    defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_LIBCURL_H

#include <stdint.h>
#include <stdlib.h>

#include "acquire_common_defs.h"
#include "acquire_string_extras.h"

#include <curl/curl.h>

#include <memory.h>

const char *get_download_dir() { return ".downloads"; }

int download_to_stdout(const char *url, const char *checksum,
                       const char *target_location, bool follow, size_t retry) {
  CURL *curl = curl_easy_init();
  if (curl) {
    CURLcode res = curl_easy_setopt(curl, CURLOPT_URL, url);
    if (res != CURLE_OK)
      return EXIT_FAILURE;

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
      return EXIT_FAILURE;
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

struct dnld_params_t {
  char dnld_remote_fname[MAX_FILENAME];
  char dnld_full_local_fname[NAME_MAX + 1];
  char dnld_url[2083];
  FILE *dnld_stream;
  /* FILE *dbg_stream; */
  uint64_t dnld_file_sz;
};

static int get_oname_from_cd(char const *const cd, char *oname) {
  char const *const cdtag = "Content-disposition:";
  char const *const key = "filename=";
  char *val = NULL;

  /* Example Content-Disposition: filename=name1367; charset=funny;
   * option=strange */

  /* If filename is present */
  val = strcasestr((const char *)cd, (const char *)key);
  if (!val) {
    fprintf(stderr, "No key-value for \"%s\" in \"%s\"", key, cdtag);
    return EXIT_FAILURE;
  }

  /* Move to value */
  val += strlen(key);

  /* Copy value as oname */
  while (*val != '\0' && *val != ';') {
    /* fprintf (stderr, ".... %c\n", *val); */
    *oname++ = *val++;
  }
  *oname = '\0';

  return EXIT_SUCCESS;
}

static int get_oname_from_url(char const *url, char *oname) {
  char const *u = url;

  /* Remove "http(s)://" */
  u = strstr(u, "://");
  if (u)
    u += strlen("://");

  u = strrchr(u, '/');

  /* Remove last '/' */
  u++;

  /* Copy value as oname */
  while (*u != '\0') {
    /* fprintf (stderr, ".... %c\n", *u); */
    *oname++ = *u++;
  }
  *oname = '\0';

  return EXIT_SUCCESS;
}

size_t dnld_header_parse(void *hdr, size_t size, size_t nmemb, void *userdata) {
  const size_t cb = size * nmemb;
  const char *hdr_str = hdr;
  struct dnld_params_t *dnld_params = (struct dnld_params_t *)userdata;
  char const *const cdtag = "Content-disposition:";

  /* Example:
   * ...
   * Content-Type: text/html
   * Content-Disposition: filename=name1367; charset=funny; option=strange
   */
  /* if (strstr(hdr_str, "Content-disposition:"))
      fprintf(stderr, "has c-d: %s\n", hdr_str);*/

  if (!strncasecmp(hdr_str, cdtag, strlen(cdtag))) {
    /* fprintf(stderr, "Found c-d: %s\n", hdr_str); */
    int ret = get_oname_from_cd(hdr_str + strlen(cdtag),
                                dnld_params->dnld_remote_fname);
    if (ret)
      fprintf(stderr, "ERR: bad remote name");
  }

  return cb;
}

FILE *get_dnld_stream(char const *const fname) {
  FILE *fp;
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  fopen_s(&fp, fname, "wb");
#else
  fp = fopen(fname, "wb");
#endif
  if (!fp) {
    fprintf(stderr, "Could not create file \"%s\"\n", fname);
    return NULL;
  }

  return fp;
}

size_t write_cb(const void *buffer, size_t sz, size_t nmemb, void *userdata) {
  size_t ret = 0;
  struct dnld_params_t *dnld_params = (struct dnld_params_t *)userdata;

  if (!dnld_params->dnld_remote_fname[0]) {
    ret = get_oname_from_url(dnld_params->dnld_url,
                             dnld_params->dnld_remote_fname);
  }

  if (ret != 0)
    return ret;

  if (!dnld_params->dnld_stream) {
    dnld_params->dnld_stream =
        get_dnld_stream(dnld_params->dnld_full_local_fname);
  }

  ret = fwrite(buffer, sz, nmemb, dnld_params->dnld_stream);

  if (ret == (sz * nmemb)) {
    dnld_params->dnld_file_sz += ret;
  }
  return ret;
}

int download(const char *url, enum Checksum checksum, const char *hash,
             const char target_location[NAME_MAX], bool follow, size_t retry,
             size_t verbosity) {
  CURL *curl;
  CURLcode cerr = CURLE_OK;
  struct dnld_params_t dnld_params;
  size_t i;
  const char *path;

  memset(&dnld_params, 0, sizeof(dnld_params));

  if (is_file(target_location)) {
    /* This next branch subsumes the `is_downloaded` function? */
    if (filesize(target_location) > 0 /* && check checksum */) {
      return EEXIST /*CURLE_ALREADY_COMPLETE*/;
    } else
    set_remote_fname_to_target_location : {
      const size_t target_location_n = strlen(target_location);
      strncpy(dnld_params.dnld_remote_fname, target_location,
              target_location_n);
      strncpy(dnld_params.dnld_full_local_fname, target_location,
              target_location_n);
    }
  } else if (is_relative(target_location)) {
    goto set_remote_fname_to_target_location;
  } else if (!is_directory(target_location)) {
    fprintf(stderr, "Create \"%s\" and ensure its accessible, then try again\n",
            target_location);
    return CURLINFO_OS_ERRNO + 2;
  }

  curl_global_init((size_t)CURL_GLOBAL_ALL);

  strncpy(dnld_params.dnld_url, url, strlen(url));

  curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    fprintf(stderr, "`curl_easy_init()` failed\n");
    return EXIT_FAILURE;
  }

#define handle_curl_error(cerr, name)                                          \
  if ((cerr) != CURLE_OK) {                                                    \
    fprintf(stderr, "%s: failed with err %d\n", name, cerr);                   \
    goto bail;                                                                 \
  } else
  cerr = curl_easy_setopt(curl, CURLOPT_URL, url);
  handle_curl_error(cerr, "CURLOPT_URL");

  if (url[0] == 'f' && url[1] == 't' && url[2] == 'p' && url[3] == 's')
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
  else
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

  /* ask libcurl to use TLS version 1.2 or later */
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

  curl_easy_setopt(curl, CURLOPT_VERBOSE, verbosity);

  /* disable progress meter, set to 0L to enable it */
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

  cerr = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, dnld_header_parse);
  handle_curl_error(cerr, "CURLOPT_HEADERFUNCTION")

      cerr = curl_easy_setopt(curl, CURLOPT_HEADERDATA, &dnld_params);
  handle_curl_error(cerr, "CURLOPT_HEADERDATA");

  if (strlen(dnld_params.dnld_remote_fname) == 0 ||
      strcmp(dnld_params.dnld_remote_fname, "/") == 0) {
    path = get_path_from_url(url);
    for (i = 0; i < strlen(path); i++)
      dnld_params.dnld_remote_fname[i] = path[i];
    dnld_params.dnld_remote_fname[i + 1] = '\0';
  }

  if (dnld_params.dnld_full_local_fname[0] != 0)
    /*pass*/;
  else if (strlen(dnld_params.dnld_remote_fname) == 0 ||
           strcmp(dnld_params.dnld_remote_fname, "/") == 0) {
    fprintf(stderr, "unable to derive a filename to save to, from: %s\n", url);
    goto bail;
  } else {
    snprintf(dnld_params.dnld_full_local_fname, NAME_MAX + 1, "%s/%s",
             target_location, dnld_params.dnld_remote_fname);
  }

  /* fold this condition into one `stat` call? */
  if (is_file(dnld_params.dnld_full_local_fname) &&
      filesize(dnld_params.dnld_full_local_fname) > 0) {
    cerr = CURLE_ALREADY_COMPLETE;
    goto bail;
  }

  cerr = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  handle_curl_error(cerr, "CURLOPT_WRITEFUNCTION");

  cerr = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dnld_params);
  handle_curl_error(cerr, "CURLOPT_WRITEDATA");

  cerr = curl_easy_perform(curl);
  if (cerr != CURLE_OK)
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(cerr));

  fclose(dnld_params.dnld_stream);

  if (dnld_params.dnld_file_sz < 1) {
    fprintf(stderr, "Downloaded an empty file\n");
    cerr = CURLE_GOT_NOTHING;
  }

bail:

  /* always cleanup */
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  if (cerr != CURLE_OK && cerr != CURLE_ALREADY_COMPLETE) {
    fprintf(stderr, "curl failed with: %s\n", curl_easy_strerror(cerr));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

#undef handle_curl_error
}

int download_many(const char *url[], const char *hashes[],
                  enum Checksum checksums[], const char *target_location,
                  bool follow, size_t retry, size_t verbosity) {
  return UNIMPLEMENTED;
}

#endif /* !defined(LIBACQUIRE_LIBCURL_H) && defined(USE_LIBCURL) &&            \
          defined(LIBACQUIRE_IMPLEMENTATION) */

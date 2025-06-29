#ifndef LIBACQUIRE_LIBCURL_H
#define LIBACQUIRE_LIBCURL_H

#if defined(LIBACQUIRE_USE_LIBCURL) && LIBACQUIRE_USE_LIBCURL &&               \
    defined(LIBACQUIRE_IMPLEMENTATION)

/*
 * libcurl implementation of libacquire's download API
 *
 * Thanks to cmake/FindCurlCustom this should work with:
 *   - Apple macOS (via `brew`, `port`, system, &etc.)
 *   - Linux, BSDs, SunOS (via system package manager, &etc.)
 *   - Windows (via conan, vcpkg, &etc.)
 *   - Elsewhere that curl supports (not tested, but shouldn't be an issue)
 * */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ACQUIRE_TESTING
#define ACQUIRE_STATIC
#else
#define ACQUIRE_STATIC static
#endif /* ACQUIRE_TESTING */

#include <errno.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <acquire_common_defs.h>
#include <acquire_fileutils.h>
#include <acquire_string_extras.h>

#include <curl/curl.h>

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

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

ACQUIRE_STATIC int get_oname_from_cd(char const *const cd, char *oname) {
  char const *const cdtag = "Content-disposition:";
  char const *const key = "filename=";
  char *val = NULL;

  /* Example Content-Disposition: filename=name1367; charset=funny;
   * option=strange */

  /* If filename is present */
  val = strcasestr((const char *)cd, (const char *)key);
  if (!val) {
    fprintf(stderr, "No key-value for \"%s\" in \"%s\"\n", key, cdtag);
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

ACQUIRE_STATIC int get_oname_from_url(char const *url, char *oname) {
  char const *u = url;

  /* Remove "http(s)://" */
  u = strstr(u, "://");
  if (u)
    u += strlen("://");

  u = strrchr(u, '/');

  if (!u)
    u = url; /* fallback if no '/' */

  /* Remove last '/' */
  else
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
  const char *hdr_str = (const char *)hdr;
  struct dnld_params_t *dnld_params = (struct dnld_params_t *)userdata;
  const char *const cdtag = "Content-disposition:";

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
      fprintf(stderr, "ERR: bad remote name\n");
  }

  return cb;
}

static FILE *get_dnld_stream(const char *const fname) {
  FILE *fp = NULL;
#if defined(_MSC_VER) || (defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__)
  /* Use fopen_s if available */
  if (fopen_s(&fp, fname, "wb") != 0)
    fp = NULL;
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
  struct dnld_params_t *dnld_params = (struct dnld_params_t *)userdata;

  if (!dnld_params->dnld_remote_fname[0]) {
    get_oname_from_url(dnld_params->dnld_url, dnld_params->dnld_remote_fname);
  }

  if (!dnld_params->dnld_stream) {
    dnld_params->dnld_stream =
        get_dnld_stream(dnld_params->dnld_full_local_fname);
    if (!dnld_params->dnld_stream)
      return 0;
  }

  {
    size_t elements_written =
        fwrite(buffer, sz, nmemb, dnld_params->dnld_stream);
    if (elements_written == nmemb) {
      dnld_params->dnld_file_sz += sz * elements_written;
      return sz * elements_written;
    }
  }

  return 0;
}

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_DOWNLOAD_IMPL)
int download(const char *url, enum Checksum checksum, const char *hash,
             const char *target_location /*[NAME_MAX]*/, bool follow,
             size_t retry, size_t verbosity) {
  CURL *curl;
  CURLcode cerr = CURLE_OK;
  struct dnld_params_t dnld_params;
  const char *path;

  memset(&dnld_params, 0, sizeof(dnld_params));

  if (is_file(target_location)) {
    if (filesize(target_location) > 0) {
      return EEXIST;
    } else {
      /* treat target_location as a filename */
      {
        size_t len = strlen(target_location);
        if (len >= sizeof(dnld_params.dnld_remote_fname))
          len = sizeof(dnld_params.dnld_remote_fname) - 1;
        memcpy(dnld_params.dnld_remote_fname, target_location, len);
        dnld_params.dnld_remote_fname[len] = '\0';
      }
      {
        size_t len = strlen(target_location);
        if (len >= sizeof(dnld_params.dnld_full_local_fname))
          len = sizeof(dnld_params.dnld_full_local_fname) - 1;
        memcpy(dnld_params.dnld_full_local_fname, target_location, len);
        dnld_params.dnld_full_local_fname[len] = '\0';
      }
    }
  } else if (is_relative(target_location)) {
    /* same as above */
    {
      size_t len = strlen(target_location);
      if (len >= sizeof(dnld_params.dnld_remote_fname))
        len = sizeof(dnld_params.dnld_remote_fname) - 1;
      memcpy(dnld_params.dnld_remote_fname, target_location, len);
      dnld_params.dnld_remote_fname[len] = '\0';
    }
    {
      size_t len = strlen(target_location);
      if (len >= sizeof(dnld_params.dnld_full_local_fname))
        len = sizeof(dnld_params.dnld_full_local_fname) - 1;
      memcpy(dnld_params.dnld_full_local_fname, target_location, len);
      dnld_params.dnld_full_local_fname[len] = '\0';
    }
  } else if (!is_directory(target_location)) {
    fprintf(stderr,
            "Create \"%s\" and ensure it's accessible, then try again\n",
            target_location);
    return CURLINFO_OS_ERRNO + 2;
  }

  {
    size_t len = strlen(url);
    if (len >= sizeof(dnld_params.dnld_url))
      len = sizeof(dnld_params.dnld_url) - 1;
    memcpy(dnld_params.dnld_url, url, len);
    dnld_params.dnld_url[len] = '\0';
  }

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    fprintf(stderr, "`curl_easy_init()` failed\n");
    return EXIT_FAILURE;
  }

#define handle_curl_error(cerr_, name)                                         \
  if ((cerr_) != CURLE_OK) {                                                   \
    fprintf(stderr, "%s: failed with err %d: %s\n", name, cerr_,               \
            curl_easy_strerror(cerr_));                                        \
    goto bail;                                                                 \
  } else

  cerr = curl_easy_setopt(curl, CURLOPT_URL, url);
  handle_curl_error(cerr, "CURLOPT_URL");

  if (strncmp(url, "ftps", 4) == 0) {
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  } else {
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  }

  /* ask libcurl to use TLS version 1.2 or later */
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

  curl_easy_setopt(curl, CURLOPT_VERBOSE, (long)verbosity);

  /* disable progress meter by default */
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

  cerr = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, dnld_header_parse);
  handle_curl_error(cerr, "CURLOPT_HEADERFUNCTION");

  cerr = curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&dnld_params);
  handle_curl_error(cerr, "CURLOPT_HEADERDATA");

  if (strlen(dnld_params.dnld_remote_fname) == 0 ||
      strcmp(dnld_params.dnld_remote_fname, "/") == 0) {
    path = get_path_from_url(url);
    if (strlen(path) >= sizeof(dnld_params.dnld_remote_fname))
      path = ""; /* fail silently, will error below */
    {
      size_t len = strlen(path);
      if (len >= sizeof(dnld_params.dnld_remote_fname))
        len = sizeof(dnld_params.dnld_remote_fname) - 1;
      memcpy(dnld_params.dnld_remote_fname, path, len);
      dnld_params.dnld_remote_fname[len] = '\0';
    }
  }

  if (dnld_params.dnld_full_local_fname[0] == '\0') {
    if (strlen(dnld_params.dnld_remote_fname) == 0 ||
        strcmp(dnld_params.dnld_remote_fname, "/") == 0) {
      fprintf(stderr, "unable to derive a filename to save to, from: %s\n",
              url);
      goto bail;
    } else {
      snprintf(dnld_params.dnld_full_local_fname,
               sizeof(dnld_params.dnld_full_local_fname), "%s%c%s",
               target_location, '/', dnld_params.dnld_remote_fname);
    }
  }

  if (is_file(dnld_params.dnld_full_local_fname) &&
      filesize(dnld_params.dnld_full_local_fname) > 0) {
    cerr = CURLE_OK /* simulate CURLE_ALREADY_COMPLETE */;
    goto bail;
  }

  cerr = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  handle_curl_error(cerr, "CURLOPT_WRITEFUNCTION");

  cerr = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&dnld_params);
  handle_curl_error(cerr, "CURLOPT_WRITEDATA");

  cerr = curl_easy_perform(curl);
  if (cerr != CURLE_OK)
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(cerr));

  if (dnld_params.dnld_stream) {
    fclose(dnld_params.dnld_stream);
    dnld_params.dnld_stream = NULL;
  }

  if (dnld_params.dnld_file_sz < 1) {
    fprintf(stderr, "Downloaded an empty file\n");
    cerr = CURLE_GOT_NOTHING;
  }

bail:
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  if (cerr != CURLE_OK) {
    fprintf(stderr, "curl failed with: %s\n", curl_easy_strerror(cerr));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

#undef handle_curl_error
}
#endif /* LIBACQUIRE_IMPLEMENTATION && LIBACQUIRE_DOWNLOAD_IMPL */

int download_many(const char *url[], const char *hashes[],
                  enum Checksum checksums[], const char *target_location,
                  bool follow, size_t retry, size_t verbosity) {
  return UNIMPLEMENTED;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* defined(LIBACQUIRE_USE_LIBCURL) &&                                   \
          defined(LIBACQUIRE_IMPLEMENTATION) */

#endif /* LIBACQUIRE_LIBCURL_H */

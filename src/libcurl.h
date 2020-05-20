#ifndef LIBACQUIRE_LIBCURL_H
#define LIBACQUIRE_LIBCURL_H

#include <stdlib.h>
#include <curl/curl.h>

#include "stdbool.h"
#include "errors.h"
#include "stringutils.h"
#include "fileutils.h"

#if defined(_AIX)
#include <sys/limits.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
|| defined(__OpenBSD__) || defined(__bsdi__)
|| defined(__DragonFly__) || defined(macintosh)
|| defined(__APPLE__) || defined(__APPLE_CC__)
#include <sys/syslimits.h>
#elif defined(__HAIKU__)
#include <system/user_runtime.h>
#elif defined(__linux__) || defined(linux) || defined(__linux)
#include <linux/version.h>
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)
#include <linux/limits.h>
#endif
#elif defined(sun) || defined(__sun) || defined(__SVR4) || defined(__svr4__)
#include <sys/param.h>
#else

#include <limits.h>

#endif

#ifndef NAME_MAX
#ifdef PATH_MAX
#define NAME_MAX PATH_MAX
#else
#define NAME_MAX 4096
#endif
#endif

#include <memory.h>

const char *get_download_dir() {
    return ".downloads";
}

bool is_downloaded(const char *url, const char *checksum);

int download_to_stdout(const char *url, const char *checksum, const char *target_directory,
                       bool follow, size_t retry) {
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
    char dnld_remote_fname[NAME_MAX];
    char dnld_full_local_fname[NAME_MAX];
    char dnld_url[NAME_MAX];
    FILE *dnld_stream;
    /* FILE *dbg_stream; */
    uint64_t dnld_file_sz;
};

static int get_oname_from_cd(char const *const cd, char *oname) {
    char const *const cdtag = "Content-disposition:";
    char const *const key = "filename=";
    char *val = NULL;

    /* Example Content-Disposition: filename=name1367; charset=funny; option=strange */

    /* If filename is present */
    val = strcasestr(cd, key);
    if (!val) {
        fprintf(stderr, "No key-value for \"%s\" in \"%s\"", key, cdtag);
        return EXIT_FAILURE;
    }

    /* Move to value */
    val += strlen(key);

    /* Copy value as oname */
    while (*val != '\0' && *val != ';') {
        //fprintf (stderr, ".... %c\n", *val);
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
    struct dnld_params_t *dnld_params = (struct dnld_params_t *) userdata;
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
        int ret = get_oname_from_cd(hdr_str + strlen(cdtag), dnld_params->dnld_remote_fname);
        if (ret)
            fprintf(stderr, "ERR: bad remote name");
    }

    return cb;
}

FILE *get_dnld_stream(char const *const fname) {
    FILE *fp = fopen(fname, "wb");
    if (!fp) {
        fprintf(stderr, "Could not create file %s\n", fname);
        return NULL;
    }

    return fp;
}

size_t write_cb(const void *buffer, size_t sz, size_t nmemb, void *userdata) {
    size_t ret = 0;
    struct dnld_params_t *dnld_params = (struct dnld_params_t *) userdata;

    if (!dnld_params->dnld_remote_fname[0]) {
        ret = get_oname_from_url(dnld_params->dnld_url, dnld_params->dnld_remote_fname);
    }

    if (ret != 0)
        return ret;

    if (!dnld_params->dnld_stream) {
        dnld_params->dnld_stream = get_dnld_stream(dnld_params->dnld_full_local_fname);
    }

    ret = fwrite(buffer, sz, nmemb, dnld_params->dnld_stream);

    if (ret == (sz * nmemb)) {
        dnld_params->dnld_file_sz += ret;
    }
    return ret;
}


int download(const char *url, const char *checksum, const char *target_directory,
             bool follow, size_t retry, size_t verbosity) {
    CURL *curl;
    CURLcode cerr = CURLE_OK;
    struct dnld_params_t dnld_params;
    size_t i;
    const char *path;

    if (!is_directory(target_directory)) {
        fprintf(stderr, "Create \"%s\" and ensure its accessible, then try again`\n", target_directory);
        return CURLINFO_OS_ERRNO + 2;
    }

    curl_global_init(CURL_GLOBAL_ALL);

    memset(&dnld_params, 0, sizeof(dnld_params));
    strncpy(dnld_params.dnld_url, url, strlen(url));

    curl = curl_easy_init();
    if (!curl) {
        curl_global_cleanup();
        fprintf(stderr, "`curl_easy_init()` failed\n");
        return EXIT_FAILURE;
    }

    cerr = curl_easy_setopt(curl, CURLOPT_URL, url);
    if (cerr) {
        fprintf(stderr, "%s: failed with err %d\n", "URL", cerr);
        goto bail;
    }

    if (url[0] == 'f' && url[1] == 't' && url[2] == 'p' && url[3] == 's')
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    else
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    /* >=TLS1.2  */
    curl_easy_setopt(curl, CURLOPT_SSL_CIPHER_LIST, "NULL-SHA256 AES128-SHA256 AES256-SHA256 AES128-GCM-SHA256 "
                                                    "AES256-GCM-SHA384 DH-RSA-AES128-SHA256 DH-RSA-AES256-SHA256 "
                                                    "DH-RSA-AES128-GCM-SHA256 DH-RSA-AES256-GCM-SHA384 "
                                                    "DH-DSS-AES128-SHA256 DH-DSS-AES256-SHA256 DH-DSS-AES128-GCM-SHA256 "
                                                    "DH-DSS-AES256-GCM-SHA384 DHE-RSA-AES128-SHA256 DHE-RSA-AES256-SHA256 "
                                                    "DHE-RSA-AES128-GCM-SHA256 DHE-RSA-AES256-GCM-SHA384 "
                                                    "DHE-DSS-AES128-SHA256 DHE-DSS-AES256-SHA256 DHE-DSS-AES128-GCM-SHA256 "
                                                    "DHE-DSS-AES256-GCM-SHA384 ECDHE-RSA-AES128-SHA256 "
                                                    "ECDHE-RSA-AES256-SHA384 ECDHE-RSA-AES128-GCM-SHA256 "
                                                    "ECDHE-RSA-AES256-GCM-SHA384 ECDHE-ECDSA-AES128-SHA256 "
                                                    "ECDHE-ECDSA-AES256-SHA384 ECDHE-ECDSA-AES128-GCM-SHA256 "
                                                    "ECDHE-ECDSA-AES256-GCM-SHA384 ADH-AES128-SHA256 ADH-AES256-SHA256 "
                                                    "ADH-AES128-GCM-SHA256 ADH-AES256-GCM-SHA384 AES128-CCM AES256-CCM "
                                                    "DHE-RSA-AES128-CCM DHE-RSA-AES256-CCM AES128-CCM8 AES256-CCM8 "
                                                    "DHE-RSA-AES128-CCM8 DHE-RSA-AES256-CCM8 ECDHE-ECDSA-AES128-CCM "
                                                    "ECDHE-ECDSA-AES256-CCM ECDHE-ECDSA-AES128-CCM8 ECDHE-ECDSA-AES256-CCM8 "
                                                    "ECDHE-ECDSA-CAMELLIA128-SHA256 ECDHE-ECDSA-CAMELLIA256-SHA384 "
                                                    "ECDHE-RSA-CAMELLIA128-SHA256 ECDHE-RSA-CAMELLIA256-SHA384 "
                                                    "TLS_AES_256_GCM_SHA384 TLS_CHACHA20_POLY1305_SHA256 "
                                                    "TLS_AES_128_GCM_SHA256 TLS_AES_128_CCM_8_SHA256 "
                                                    "TLS_AES_128_CCM_SHA256");

    curl_easy_setopt(curl, CURLOPT_VERBOSE, verbosity);

    /* disable progress meter, set to 0L to enable it */
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    cerr = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, dnld_header_parse);
    if (cerr) {
        fprintf(stderr, "%s: failed with err %d\n", "HEADER", cerr);
        goto bail;
    }

    cerr = curl_easy_setopt(curl, CURLOPT_HEADERDATA, &dnld_params);
    if (cerr) {
        fprintf(stderr, "%s: failed with err %d\n", "HEADER DATA", cerr);
        goto bail;
    }

    if (strlen(dnld_params.dnld_remote_fname) == 0 || strcmp(dnld_params.dnld_remote_fname, "/") == 0) {
        path = get_path_from_url(url);
        for (i = 0; i < strlen(path); i++)
            dnld_params.dnld_remote_fname[i] = path[i];
        dnld_params.dnld_remote_fname[i + 1] = '\0';
    }

    if (strlen(dnld_params.dnld_remote_fname) == 0 || strcmp(dnld_params.dnld_remote_fname, "/") == 0) {
        fprintf(stderr, "unable to derive a filename to save to, from: %s\n", url);
        goto bail;
    }

    snprintf(dnld_params.dnld_full_local_fname, sizeof(dnld_params.dnld_full_local_fname),
             "%s/%s", target_directory, dnld_params.dnld_remote_fname);

    if (is_file(dnld_params.dnld_full_local_fname)) {
        cerr = CURLE_ALREADY_COMPLETE;
        goto bail;
    }

    cerr = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    if (cerr) {
        fprintf(stderr, "%s: failed with err %d\n", "WR CB", cerr);
        goto bail;
    }

    cerr = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dnld_params);
    if (cerr) {
        fprintf(stderr, "%s: failed with err %d\n", "WR Data", cerr);
        goto bail;
    }

    cerr = curl_easy_perform(curl);
    if (cerr != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(cerr));

    fclose(dnld_params.dnld_stream);

    if (dnld_params.dnld_file_sz < 1) {
        fprintf(stderr, "Downloaded an empty file\n");
        cerr = CURLE_GOT_NOTHING;
    }

    bail:

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return cerr;
}

int download_many(const char *url[], const char *checksum, const char *target_directory,
                  bool follow, size_t retry, size_t verbosity) {
    return UNIMPLEMENTED;
}

#endif //LIBACQUIRE_LIBCURL_H

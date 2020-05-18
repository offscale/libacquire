#ifndef LIBACQUIRE_LIBCURL_H
#define LIBACQUIRE_LIBCURL_H

#include <stdlib.h>
#include <curl/curl.h>

#include "stdbool.h"
#include "errors.h"

const char* get_download_dir() {
    return ".downloads";
}

bool is_downloaded(const char *url, const char *checksum);

int download_to_stdout(const char *url, const char *checksum, const char *target_directory, bool follow, size_t retry) {
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/*
 * This callback sets the filename where output shall be written when
 * curl options --remote-name (-O) and --remote-header-name (-J) have
 * been simultaneously given and additionally server returns an HTTP
 * Content-Disposition header specifying a filename property.
 */
static void set_filename() {
    /*curl_easy_getinfo(per->curl, CURLINFO_PROTOCOL, CURLPROTO_HTTPS);
    if(checkprefix("Content-disposition:", str)) {
        const char *p = str + 20;*/

        /* look for the 'filename=' parameter
           (encoded filenames (*=) are not supported) */
        /*for(;;) {
            char *filename;
            size_t len;

            while(*p && (p < end) && !ISALPHA(*p))
                p++;
            if(p > end - 9)
                break;

            if(memcmp(p, "filename=", 9)) {*/
                /* no match, find next parameter */
                /*while((p < end) && (*p != ';'))
                    p++;
                continue;
            }
            p += 9;*/

            /* this expression below typecasts 'cb' only to avoid
               warning: signed and unsigned type in conditional expression
            */
            /*len = (ssize_t)cb - (p - str);
            filename = parse_filename(p, len);
            if(filename) {
                if(outs->stream) {
                    int rc;*/
                    /* already opened and possibly written to */
                    /*if(outs->fopened)
                        fclose(outs->stream);
                    outs->stream = NULL;*/

                    /* rename the initial file name to the new file name */
                    /*rc = rename(outs->filename, filename);
                    if(rc != 0) {
                        warnf(per->config->global, "Failed to rename %s -> %s: %s\n",
                              outs->filename, filename, strerror(errno));
                    }
                    if(outs->alloc_filename)
                        Curl_safefree(outs->filename);
                    if(rc != 0) {
                        free(filename);
                        return failure;
                    }
                }
                outs->is_cd_filename = TRUE;
                outs->s_isreg = TRUE;
                outs->fopened = FALSE;
                outs->filename = filename;
                outs->alloc_filename = TRUE;
                hdrcbdata->honor_cd_filename = FALSE; */ /* done now! */
                /*if(!tool_create_output_file(outs, per->config))
                    return failure;
            }
            break;
        }
        if(!outs->stream && !tool_create_output_file(outs, per->config))
            return failure;
    }*/
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int download(const char *url, const char *checksum, const char *target_directory, bool follow, size_t retry, size_t verbosity)
{
    /* Function body mostly from https://github.com/curl/curl/blob/4c2f5d5/docs/examples/url2file.c  */
    CURL *curl;
    CURLcode res;
    static const char *pagefilename = "page.out";
    FILE *pagefile;

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl = curl_easy_init();
    if (!curl) return EXIT_FAILURE;

    /* set URL to get here */
    curl_easy_setopt(curl, CURLOPT_URL, url);

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

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    /* open the file */
    pagefile = fopen(pagefilename, "wb");
    if (pagefile) {
        /* write the page body to this file handle */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

        /* get it! */
        res = curl_easy_perform(curl);

        /* close the header file */
        fclose(pagefile);
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl);

    curl_global_cleanup();

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int download_many(const char *url[], const char *checksum, const char *target_directory, bool follow, size_t retry, size_t verbosity) {
    return UNIMPLEMENTED;
}


#endif //LIBACQUIRE_LIBCURL_H

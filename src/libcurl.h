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

int download(const char *url, const char *checksum, const char *target_directory, bool follow, size_t retry) {
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        if (url[0] == 'f' && url[1] == 't' && url[2] == 'p' && url[3] == 's')
            curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
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
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int download_many(const char *url[], const char *checksum, const char *target_directory, bool follow, size_t retry) {
    return UNIMPLEMENTED;
}


#endif //LIBACQUIRE_LIBCURL_H

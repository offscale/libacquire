#[=======================================================================[

libacquire_helpers
------------------

Functions to reuse crypto, network, and https libraries

]=======================================================================]

####################
# crypto libraries #
####################

macro(set_crypto_lib)
    # "Crypto library to use, defaults to Linux,BSD,SunOS: OpenSSL; Windows: STunnel; macOS: LibreSSL"
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT DEFINED CRYPTO_LIB)
        check_include_files("CommonCrypto/CommonCrypto.h;CommonCrypto/CommonDigest.h" HAVE_COMMON_CRYPTO_H)
        if (NOT HAVE_COMMON_CRYPTO_H)
            message(FATAL_ERROR "CommonCrypto.h not found")
        endif ()
        set(CRYPTO_LIB "CommonCrypto")
        set(USE_COMMON_CRYPTO 1)
        add_compile_definitions(USE_COMMON_CRYPTO=1)
    elseif ((CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" AND NOT DEFINED CRYPTO_LIB) OR CRYPTO_LIB STREQUAL "LibreSSL")
        set(CRYPTO_LIB "LibreSSL")
        include("../cmake/FindLibreSSL.cmake")
        set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
        set(USE_LIBRESSL 1)
        add_compile_definitions(USE_LIBRESSL=1)
    elseif ((CMAKE_SYSTEM_NAME STREQUAL "Linux"
            OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD"
            OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
            AND NOT DEFINED CRYPTO_LIB
            OR CRYPTO_LIB STREQUAL "OpenSSL")
        set(CRYPTO_LIB "OpenSSL")
        set(USE_OPENSSL 1)
        add_compile_definitions(USE_OPENSSL=1)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT DEFINED CRYPTO_LIB)
        set(CRYPTO_LIB "WinCrypt")
        set(USE_WINCRYPT 1)
        add_compile_definitions(USE_WINCRYPT=1)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "SunOS" AND NOT DEFINED CRYPTO_LIB)
        message(FATAL_ERROR "TODO")
    endif ()
endmacro(set_crypto_lib)

#################
# SSL libraries #
#################

macro(set_ssl_lib)
    if (CRYPTO_LIB STREQUAL "OpenSSL")
        if ((NOT DEFINED OPENSSL_ROOT_DIR OR NOT IS_DIRECTORY OPENSSL_ROOT_DIR)
                AND CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND IS_DIRECTORY "/usr/local/opt/openssl")
            set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
        elseif ((NOT DEFINED OPENSSL_ROOT_DIR OR NOT IS_DIRECTORY OPENSSL_ROOT_DIR)
                AND IS_DIRECTORY "/usr/include/openssl")
            set(OPENSSL_ROOT_DIR "/usr/include/openssl")
        endif ()
        find_package(OpenSSL REQUIRED)
        check_include_file("openssl/sha.h" HAVE_OPENSSL_SHA_H LANGUAGE C)
        if (NOT HAVE_OPENSSL_SHA_H)
            message(FATAL_ERROR "OpenSSL installation doesn't have the required header files, install them or use alt SSL")
        endif ()
    elseif (CRYPTO_LIB STREQUAL "LibreSSL")
        find_package(LibreSSL REQUIRED)
    elseif (CRYPTO_LIB STREQUAL "CommonCrypto" OR CRYPTO_LIB STREQUAL "OpenSSL")
        list(APPEND _Header_Files "acquire_openssl.h")
    elseif (CRYPTO_LIB STREQUAL "WinCrypt")
        list(APPEND _Header_Files "acquire_wincrypt.h")
    elseif (NOT DEFINED CRYPTO_LIB OR CRYPTO_LIB STREQUAL "")
        message(FATAL_ERROR "CRYPTO_LIB undefined")
    else ()
        message(FATAL_ERROR "Not implemented CRYPTO_LIB of '${CRYPTO_LIB}'")
    endif ()
endmacro(set_ssl_lib)

########################
# HTTP/HTTPS libraries #
########################

macro(set_http_https_lib)
    if (DEFINED USE_LIBFETCH
            OR (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD"
            OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
            AND USE_LIBCURL STREQUAL "OFF")
        set(USE_LIBFETCH 1)
        add_compile_definitions(USE_LIBFETCH=1)
        list(APPEND _Header_Files "acquire_libfetch.h")
        set(HTTPS_LIB "OPENSSL")
    elseif (DEFINED USE_MY_LIBFETCH)
        set(USE_LIBFETCH 1)
        set(USE_MY_LIBFETCH 1)
        add_compile_definitions(USE_MY_LIBFETCH=1)
        list(APPEND _Header_Files "acquire_libfetch.h")
        set(HTTPS_LIB "LIBFETCH")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" AND USE_LIBCURL STREQUAL "OFF")
        set(USE_OPENBSD_FTP 1)
        add_compile_definitions(USE_OPENBSD_FTP=1)
        set(HTTPS_LIB "OPENBSD_FTP")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(USE_WININET 1)
        add_compile_definitions(USE_WININET=1)
        list(APPEND _Header_Files "acquire_wininet.h")
        set(HTTPS_LIB "WININET")
    else ()
        set(USE_LIBCURL 1)
        add_compile_definitions(USE_LIBCURL=1)
        get_curl(CURL_LINK_LIBRARIES)
        list(APPEND _Header_Files "acquire_libcurl.h")
        set(HTTPS_LIB "LIBCURL")
    endif ()
endmacro(set_http_https_lib)

########################
# Extraction libraries #
########################

function(set_extract_lib extract_lib extraction_api)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT DEFINED USE_ZLIB AND NOT DEFINED USE_LIBARCHIVE)
        set(USE_WINCOMPRESSAPI 1)
        set("${extract_lib}" "WINCOMPRESSAPI" PARENT_SCOPE)
        list(APPEND _Header_Files "acquire_compressapi.h")
        set(extraction_api "WINCOMPRESSAPI")
    elseif (DEFINED USE_LIBARCHIVE)
        list(APPEND _Header_Files "acquire_libarchive.h")
        set("${extract_lib}" "LIBARCHIVE" PARENT_SCOPE)
        set(extraction_api "LIBARCHIVE")
    else ()
        set(USE_ZLIB 1)
        list(APPEND _Header_Files "acquire_zlib.h")
        set("${extract_lib}" "ZLIB" PARENT_SCOPE)
        set(extraction_api "ZLIB")
    endif ()
    set(extraction_api "${extraction_api}" PARENT_SCOPE)
endfunction(set_extract_lib extract_lib)

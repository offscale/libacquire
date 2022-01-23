#[=======================================================================[

FindLibAcquire
--------------

Finds the dependencies that libacquire relies on (libacquire itself is header-only)

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

LIBACQUIRE_FOUND
    System has any of the libraries that libacquire depends on.
LIBACQUIRE_INCLUDE_DIR
    Include directories of any of the libraries that libacquire depends on. Use with `target_include_directories`.
LIBACQUIRE_LIBRARIES
    Libraries that libacquire depends on. Use `target_link_libraries` to link.

]=======================================================================]

########################
# Networking libraries #
########################

function (set_networking_lib HTTPS_LIBRARY)
    if (DEFINED HTTPS_LIB)
        set("USE_${HTTPS_LIB}" 1 PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "At least one HTTPS library must be specified for linkage")
    endif ()

    if (DEFINED USE_LIBCURL AND DEFINED CURL_LINK_LIBRARIES)
        set(HTTPS_LIBRARY "${CURL_LINK_LIBRARIES}" PARENT_SCOPE)
    elseif (DEFINED USE_WININET)
        set(HTTPS_LIBRARY "wininet" PARENT_SCOPE)
        if (NOT DEFINED USE_OPENSSL)
            set(USE_WINCRYPT 1 PARENT_SCOPE)
        endif (NOT DEFINED USE_OPENSSL)
    elseif (DEFINED USE_MY_LIBFETCH)
        set(HTTPS_LIBRARY "freebsd_libfetch" PARENT_SCOPE)
    elseif (DEFINED USE_LIBFETCH)
        set(HTTPS_LIBRARY "fetch" PARENT_SCOPE)
    endif ()
endfunction (set_networking_lib HTTPS_LIBRARY)

set_networking_lib(HTTPS_LIBRARY)

if (DEFINED HTTPS_LIBRARY AND NOT HTTPS_LIBRARY STREQUAL "")
    list(APPEND LIBACQUIRE_LIBRARIES "${HTTPS_LIBRARY}")
else ()
    message(FATAL_ERROR "HTTPS_LIBRARY not set for linkage")
endif ()

message(STATUS "net LIBACQUIRE_LIBRARIES = ${LIBACQUIRE_LIBRARIES}")

###########################
# Cryptographic libraries #
###########################

function (set_cryptography_lib CRYPTO_LIBRARIES)
    if (NOT DEFINED CRYPTO_LIB)
        message(FATAL_ERROR "Crypto library could not be inferred so must be specified for linkage")
    endif (NOT DEFINED CRYPTO_LIB)

    set(CRYPTO_LIBRARIES "")

    if (DEFINED OPENSSL_LIBRARIES)
        list(APPEND CRYPTO_LIBRARIES "OpenSSL::SSL")
        list(APPEND CRYPTO_LIBRARIES "${OPENSSL_LIBRARIES}")
        set(LIBACQUIRE_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}")
    endif (DEFINED OPENSSL_LIBRARIES)

    if (DEFINED USE_WINCRYPT)
        list(APPEND CRYPTO_LIBRARIES "advapi32")
        list(APPEND CRYPTO_LIBRARIES "crypt32")
    endif (DEFINED USE_WINCRYPT)
    set(CRYPTO_LIBRARIES "${CRYPTO_LIBRARIES}" PARENT_SCOPE)
endfunction (set_cryptography_lib)

set_cryptography_lib(CRYPTO_LIBRARIES)

if (DEFINED CRYPTO_LIBRARIES AND NOT CRYPTO_LIBRARIES STREQUAL "")
    list(APPEND LIBACQUIRE_LIBRARIES "${CRYPTO_LIBRARIES}")
elseif (NOT DEFINED USE_COMMON_CRYPTO)  # Link not needed
    message(FATAL_ERROR "CRYPTO_LIBRARIES not set for linkage")
endif ()

message(STATUS "crypt LIBACQUIRE_LIBRARIES = ${LIBACQUIRE_LIBRARIES}")

#########################
# Compression libraries #
#########################

macro (download_extract_miniz download_dir)
    set(MINIZ_VERSION "2.2.0")
    set(MINIZ_BASENAME "miniz-${MINIZ_VERSION}.zip")
    get_filename_component(MINIZ_BASENAME_NO_EXT "${MINIZ_BASENAME}" NAME_WLE)
    set(MINIZ_ZIP_FILE "${download_dir}/${MINIZ_BASENAME}")
    if (NOT EXISTS "${MINIZ_ZIP_FILE}")
        file(DOWNLOAD
                "https://github.com/richgel999/miniz/releases/download/${MINIZ_VERSION}/miniz-${MINIZ_VERSION}.zip"
                "${MINIZ_ZIP_FILE}"
                EXPECTED_HASH "SHA256=e4aa5078999c7f7466fa6b8f9848e39ddfff9a4bafc50215764aebe1f13b3841")
        #file(ARCHIVE_EXTRACT INPUT "${MINIZ_ZIP_FILE}"
        #        DESTINATION "${download_dir}")
    endif (NOT EXISTS "${MINIZ_ZIP_FILE}")

    if (NOT EXISTS "${download_dir}/zip.h")
        file(DOWNLOAD
                "https://raw.githubusercontent.com/kuba--/zip/5b3f387/src/zip.h"
                "${download_dir}/zip.h"
                EXPECTED_HASH "SHA256=f2f9ecb2a5c9d9fa489c278c011aedb4752567b17d41912b57eb148f05774e4e")

        # My C89 compatible PR
        file(DOWNLOAD
                "https://raw.githubusercontent.com/kuba--/zip/42dc4ce/src/zip.c"
                "${download_dir}/zip.c"
                EXPECTED_HASH "SHA256=ceafe6f3a7788697eb639ccc17788f2d9618ff6a573b96a6fb9db044eb19a7b7")

        # Looks like they changed miniz, but are still on 2.2.0
        file(DOWNLOAD
                "https://raw.githubusercontent.com/kuba--/zip/5b3f387/src/miniz.h"
                "${download_dir}/miniz.h"
                EXPECTED_HASH "SHA256=ce02b94490b7a24cc24d2426869a04239ff47dd29d133f9a57625afc0f4a0e87")
    endif (NOT EXISTS "${download_dir}/zip.h")

    file(TO_NATIVE_PATH "${MINIZ_ZIP_FILE}" MINIZ_ZIP_FILE)

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
        string(REPLACE "\\" "\\\\" MINIZ_ZIP_FILE "${MINIZ_ZIP_FILE}")
    endif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
endmacro (download_extract_miniz download_dir)

function (download_unarchiver EXTRACT_LIB)
    ###############################################################
    # Download and setup miniz, a modern zero-dependency zlib alt #
    ###############################################################
    set(EXTRACT_LIB "miniz")
    set(EXTRACT_LIB "miniz" PARENT_SCOPE)

    if (NOT TARGET "${EXTRACT_LIB}")
        set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/third_party/${EXTRACT_LIB}")
        download_extract_miniz("${DOWNLOAD_DIR}")

        set(Header_Files "${DOWNLOAD_DIR}/${EXTRACT_LIB}.h" "${DOWNLOAD_DIR}/zip.h")
        source_group("Header Files" FILES "${Header_Files}")

        set(Source_Files "${DOWNLOAD_DIR}/zip.c")
        source_group("Source Files" FILES "${Source_Files}")

        add_library("${EXTRACT_LIB}" "${Header_Files}" "${Source_Files}")

        target_link_libraries("${EXTRACT_LIB}" PRIVATE "libacquire_compiler_flags")

        set_target_properties(
                "${EXTRACT_LIB}"
                PROPERTIES
                LINKER_LANGUAGE
                C
        )
    endif (NOT TARGET "${EXTRACT_LIB}")
endfunction (download_unarchiver EXTRACT_LIB)

function (set_download_unarchiver EXTRACT_LIB)
    download_unarchiver(EXTRACT_LIB)
    if (NOT DEFINED EXTRACT_LIB)
        message(FATAL_ERROR "EXTRACT_LIB is not defined")
    endif (NOT DEFINED EXTRACT_LIB)
    set(EXTRACT_LIB "${EXTRACT_LIB}" PARENT_SCOPE)
    unset(USE_ZLIB)
    unset(USE_ZLIB PARENT_SCOPE)
    remove_definitions(-DUSE_ZLIB)
    set(USE_MINIZ 1)
    set(USE_MINIZ "${USE_MINIZ}" PARENT_SCOPE)
endfunction (set_download_unarchiver EXTRACT_LIB)

if (DEFINED EXTRACT_LIB)
    set("USE_${EXTRACT_LIB}" "1" PARENT_SCOPE)
else ()
    message(FATAL_ERROR "Compression API not set, did you run `set_extract_lib()`?")
endif ()

function (set_extraction_api)
    if (DEFINED USE_ZLIB)
        include(FindZLIB)
        find_package(ZLIB QUIET)
        if (ZLIB_FOUND)
            include(FindPkgConfig)
            if (PKG_CONFIG_FOUND)
                pkg_check_modules(UNZIP minizip)
                if (UNZIP_FOUND)
                    list(APPEND extract_libraries "ZLIB::ZLIB")
                    list(APPEND extract_libraries "${UNZIP_LIBRARIES}")
                    set(MINIZIP 1)
                else ()
                    set_download_unarchiver(EXTRACT_LIB)
                endif (UNZIP_FOUND)
            else ()
                set_download_unarchiver(EXTRACT_LIB)
            endif (PKG_CONFIG_FOUND)
        else ()
            set_download_unarchiver(EXTRACT_LIB)
        endif (ZLIB_FOUND)
    else()
        set_download_unarchiver(EXTRACT_LIB)
        set(EXTRACT_LIB "${EXTRACT_LIB}" PARENT_SCOPE)
    endif (DEFINED USE_ZLIB)
    set(EXTRACT_LIBRARIES "${EXTRACT_LIB}" PARENT_SCOPE)
endfunction (set_extraction_api)

set_extraction_api(EXTRACT_LIBRARIES)

if (DEFINED EXTRACT_LIBRARIES AND NOT EXTRACT_LIBRARIES STREQUAL "")
    list(APPEND LIBACQUIRE_LIBRARIES "${EXTRACT_LIBRARIES}")
else ()
    message(FATAL_ERROR "EXTRACT_LIBRARIES not set for linkage")
endif ()

message(STATUS "compress LIBACQUIRE_LIBRARIES = ${LIBACQUIRE_LIBRARIES}")

######################
# Checksum libraries #
######################

function (set_checksum_libraries CHECKSUM_LIBRARIES)
    # Note that most checksum libraries are crypto libraries so this function doesn't HAVE to be called
    if (USE_LIBRHASH)
        #include("${CMAKE_SOURCE_DIR}/cmake/FindLibRHash.cmake")
        #find_library(LibRHash_LIBRARY NAMES RHash LibRHash librhash rhash REQUIRED)
        find_package(rhash CONFIG REQUIRED)
        set(CHECKSUM_LIB "rhash")
    endif (USE_LIBRHASH)
    # set(CHECKSUM_LIB "librhash" PARENT_SCOPE)
    if (DEFINED CHECKSUM_LIBRARIES AND NOT CHECKSUM_LIBRARIES STREQUAL "")
        set(CHECKSUM_LIBRARIES "${CHECKSUM_LIB}" PARENT_SCOPE)
    endif (DEFINED CHECKSUM_LIBRARIES AND NOT CHECKSUM_LIBRARIES STREQUAL "")
endfunction (set_checksum_libraries CHECKSUM_LIBRARIES)

set_checksum_libraries(CHECKSUM_LIBRARIES)

if (CHECKSUM_LIBRARIES)
    list(APPEND LIBACQUIRE_LIBRARIES "${CHECKSUM_LIBRARIES}")
else ()
    message(STATUS "CHECKSUM_LIBRARIES not set for linkage (crypto libraries will be used)")
endif ()

message(STATUS "checksum LIBACQUIRE_LIBRARIES = ${LIBACQUIRE_LIBRARIES}")

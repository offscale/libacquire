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

if (DEFINED HTTPS_LIB)
    set("USE_${HTTPS_LIB}" 1)
else ()
    message(FATAL_ERROR "At least one HTTPS library must be specified for linkage")
endif ()

if (DEFINED USE_LIBCURL AND DEFINED CURL_LINK_LIBRARIES)
    set(LIBACQUIRE_LIBRARIES "${CURL_LINK_LIBRARIES}")
elseif (DEFINED USE_WININET)
    set(LIBACQUIRE_LIBRARIES "wininet")
    if (NOT DEFINED USE_OPENSSL)
        set(USE_WINCRYPT 1)
    endif ()
elseif (DEFINED USE_MY_LIBFETCH)
    set(LIBACQUIRE_LIBRARIES "freebsd_libfetch")
elseif (DEFINED USE_LIBFETCH)
    set(APPEND LIBACQUIRE_LIBRARIES "fetch")
endif ()

###########################
# Cryptographic libraries #
###########################

if (DEFINED OPENSSL_LIBRARIES)
    list(APPEND LIBACQUIRE_LIBRARIES "OpenSSL::SSL")
    list(APPEND LIBACQUIRE_LIBRARIES "${OPENSSL_LIBRARIES}")
    set(LIBACQUIRE_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}")
endif (DEFINED OPENSSL_LIBRARIES)

if (NOT DEFINED LIBACQUIRE_LIBRARIES)
    message(FATAL_ERROR "At least one network library must be specified for linkage")
endif (NOT DEFINED LIBACQUIRE_LIBRARIES)

if (DEFINED USE_WINCRYPT)
    list(APPEND LIBACQUIRE_LIBRARIES "advapi32")
    list(APPEND LIBACQUIRE_LIBRARIES "crypt32")
endif (DEFINED USE_WINCRYPT)

#########################
# Compression libraries #
#########################

function (download_unarchiver library)
    ###########################################################################
    # Download and setup puff, a zero-dependency zlib alt from zlib's contrib #
    ###########################################################################
    set(LIBRARY_NAME "puff")

    if (NOT TARGET "${LIBRARY_NAME}")
        set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/third_party/${LIBRARY_NAME}")
        file(DOWNLOAD
                "https://raw.githubusercontent.com/madler/zlib/1005690/contrib/puff/puff.h"
                "${DOWNLOAD_DIR}/${LIBRARY_NAME}.h"
                EXPECTED_HASH "SHA256=969b7be2a930db0cdcb19b0e5b29ae6741f5a8f663b6dba6d647e12ec60cfa8e")

        file(DOWNLOAD "https://raw.githubusercontent.com/madler/zlib/03614c5/contrib/puff/puff.c"
                "${DOWNLOAD_DIR}/${LIBRARY_NAME}.c"
                EXPECTED_HASH "SHA256=6d0eef92e115a42e570b79d8b07a04af5ccbd6b3f3fbca9cbc61c49db9c9df43")

        set(Header_Files "${DOWNLOAD_DIR}/${LIBRARY_NAME}.h")
        source_group("Header Files" FILES "${Header_Files}")

        set(Source_Files "${DOWNLOAD_DIR}/${LIBRARY_NAME}.c")
        source_group("Source Files" FILES "${Source_Files}")

        add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")

        target_link_libraries("${LIBRARY_NAME}" INTERFACE "${PROJECT_LOWER_NAME}_compiler_flags")

        set_target_properties(
                "${LIBRARY_NAME}"
                PROPERTIES
                LINKER_LANGUAGE
                C
        )
    endif ()

    set(${library} "${LIBRARY_NAME}" PARENT_SCOPE)
endfunction (download_unarchiver library)

macro (set_download_unarchiver)
    set(archive_lib "")
    download_unarchiver(archive_lib)
    list(APPEND LIBACQUIRE_LIBRARIES "${archive_lib}")
    unset(USE_ZLIB)
    remove_definitions(-DUSE_ZLIB)
    set(USE_PUFF 1)
    set(USE_PUFF "${USE_PUFF}" PARENT_SCOPE)
endmacro (set_download_unarchiver)

if (DEFINED USE_ZLIB)
    include(FindZLIB)
    find_package(ZLIB QUIET)
    if (ZLIB_FOUND)
        INCLUDE (FindPkgConfig)
        if (PKG_CONFIG_FOUND)
            PKG_CHECK_MODULES(UNZIP minizip)
            if (UNZIP_FOUND)
                list(APPEND LIBACQUIRE_LIBRARIES "ZLIB::ZLIB")
                list(APPEND LIBACQUIRE_LIBRARIES "${UNZIP_LIBRARIES}")
                set(MINIZIP 1)
            else ()
                set_download_unarchiver()
            endif (UNZIP_FOUND)
        else ()
            set_download_unarchiver()
        endif (PKG_CONFIG_FOUND)
    else ()
        set_download_unarchiver()
    endif (ZLIB_FOUND)
endif (DEFINED USE_ZLIB)


message(STATUS "FindLibAcquire.cmake EXTRACT_LIB = ${EXTRACT_LIB}")
if (DEFINED EXTRACT_LIB)
    set("USE_${EXTRACT_LIB}" 1)
    set("USE_${EXTRACT_LIB}" "1" PARENT_SCOPE)
    message(STATUS "USE_${EXTRACT_LIB} is ${USE_${EXTRACT_LIB}}")
else ()
    message(FATAL_ERROR "Compression API not set, did you run `set_extract_lib()`?")
endif ()

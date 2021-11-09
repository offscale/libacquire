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

macro (download_extract_miniz download_dir)
    set(MINIZ_VERSION "2.2.0")
    set(MINIZ_BASENAME "miniz-${MINIZ_VERSION}.zip")
    get_filename_component(MINIZ_BASENAME_NO_EXT "${MINIZ_BASENAME}" NAME_WLE)
    set(MINIZ_ZIP_FILE "${download_dir}/${MINIZ_BASENAME}")
    if (NOT EXISTS "${ZLIB_ZIP_FILE}")
        file(DOWNLOAD
                "https://github.com/richgel999/miniz/releases/download/${MINIZ_VERSION}/miniz-${MINIZ_VERSION}.zip"
                "${MINIZ_ZIP_FILE}"
                EXPECTED_HASH "SHA256=e4aa5078999c7f7466fa6b8f9848e39ddfff9a4bafc50215764aebe1f13b3841")
        file(ARCHIVE_EXTRACT INPUT "${MINIZ_ZIP_FILE}"
                DESTINATION "${download_dir}")
    endif ()

    file(TO_NATIVE_PATH "${MINIZ_ZIP_FILE}" MINIZ_ZIP_FILE)

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
        string(REPLACE "\\" "\\\\" MINIZ_ZIP_FILE "${MINIZ_ZIP_FILE}")
    endif ()
endmacro (download_extract_miniz download_dir)

function (download_unarchiver library)
    ###############################################################
    # Download and setup miniz, a modern zero-dependency zlib alt #
    ###############################################################
    set(LIBRARY_NAME "miniz")

    if (NOT TARGET "${LIBRARY_NAME}")
        set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/third_party/${LIBRARY_NAME}")
        download_extract_miniz("${DOWNLOAD_DIR}")

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

function (set_download_unarchiver libraries)
    set(archive_lib "")
    download_unarchiver(archive_lib)
    message(STATUS "set_download_unarchiver::b4 libraries = ${libraries}")
    list(APPEND libraries "${archive_lib}")
    message(STATUS "set_download_unarchiver::l8 libraries = ${libraries}")
    unset(USE_ZLIB)
    unset(USE_ZLIB PARENT_SCOPE)
    remove_definitions(-DUSE_ZLIB)
    set(USE_MINIZ 1)
    set(USE_MINIZ "${USE_MINIZ}" PARENT_SCOPE)
    set(EXTRACT_LIB "MINIZ")
    set(EXTRACT_LIB "MINIZ" PARENT_SCOPE)
endfunction (set_download_unarchiver)

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
                set_download_unarchiver("${LIBACQUIRE_LIBRARIES}")
            endif (UNZIP_FOUND)
        else ()
            set_download_unarchiver("${LIBACQUIRE_LIBRARIES}")
        endif (PKG_CONFIG_FOUND)
    else ()
        set_download_unarchiver("${LIBACQUIRE_LIBRARIES}")
    endif (ZLIB_FOUND)

    if (EXTRACT_LIB STREQUAL "MINIZ")
        string(TOLOWER "${EXTRACT_LIB}" EXTRACT_LIB_LOWER)
        list(APPEND LIBACQUIRE_LIBRARIES "${EXTRACT_LIB_LOWER}")
    endif ()
endif (DEFINED USE_ZLIB)

if (DEFINED EXTRACT_LIB)
    set("USE_${EXTRACT_LIB}" "1" PARENT_SCOPE)
else ()
    message(FATAL_ERROR "Compression API not set, did you run `set_extract_lib()`?")
endif ()

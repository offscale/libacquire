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

function(set_networking_lib HTTPS_LIBRARY)
    if (DEFINED HTTPS_LIB)
        set("USE_${HTTPS_LIB}" 1 PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "At least one HTTPS library must be specified for linkage")
    endif ()

    if (DEFINED LIBACQUIRE_USE_LIBCURL AND DEFINED CURL_LINK_LIBRARIES)
        set(HTTPS_LIBRARY "${CURL_LINK_LIBRARIES}" PARENT_SCOPE)
    elseif (DEFINED LIBACQUIRE_USE_WININET)
        set(HTTPS_LIBRARY "wininet" PARENT_SCOPE)
        if (NOT DEFINED LIBACQUIRE_USE_OPENSSL)
            set(LIBACQUIRE_USE_WINCRYPT 1 PARENT_SCOPE)
        endif (NOT DEFINED LIBACQUIRE_USE_OPENSSL)
    elseif (DEFINED LIBACQUIRE_USE_MY_LIBFETCH)
        set(HTTPS_LIBRARY "freebsd_libfetch" PARENT_SCOPE)
    elseif (DEFINED LIBACQUIRE_USE_LIBFETCH)
        set(HTTPS_LIBRARY "fetch" PARENT_SCOPE)
    endif ()
endfunction(set_networking_lib HTTPS_LIBRARY)

set_networking_lib(HTTPS_LIBRARY)

if (DEFINED HTTPS_LIBRARY AND NOT HTTPS_LIBRARY STREQUAL "")
    list(APPEND LIBACQUIRE_LIBRARIES "${HTTPS_LIBRARY}")
else ()
    message(FATAL_ERROR "HTTPS_LIBRARY not set for linkage")
endif ()

message(STATUS "net LIBACQUIRE_LIBRARIES      = ${LIBACQUIRE_LIBRARIES}")


######################
# Checksum libraries #
######################

function(set_checksum_libraries CHECKSUM_LIBRARIES)
    # Note that most checksum libraries are crypto libraries so this function doesn't HAVE to be called
    if (LIBACQUIRE_USE_LIBRHASH)
        # list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
        if (NOT TARGET LibRHash)
            find_package(LibRHash REQUIRED)
        endif (NOT TARGET LibRHash)
        set(CHECKSUM_LIBRARIES_USE "USE_LIBRASH" PARENT_SCOPE)
        set(CHECKSUM_LIBRARIES "LibRHash::LibRHash" PARENT_SCOPE)
    endif (LIBACQUIRE_USE_LIBRHASH)
endfunction(set_checksum_libraries CHECKSUM_LIBRARIES)

set_checksum_libraries(CHECKSUM_LIBRARIES)

if (DEFINED CHECKSUM_LIBRARIES AND NOT CHECKSUM_LIBRARIES STREQUAL "")
    if (NOT CHECKSUM_LIBRARIES STREQUAL "crc32c")
        list(APPEND LIBACQUIRE_LIBRARIES "${CHECKSUM_LIBRARIES}")
    endif (NOT CHECKSUM_LIBRARIES STREQUAL "crc32c")
else ()
    message(STATUS "CHECKSUM_LIBRARIES not set for linkage (crypto libraries will be used)")
endif ()

message(STATUS "checksum LIBACQUIRE_LIBRARIES = ${LIBACQUIRE_LIBRARIES}")

###########################
# Cryptographic libraries #
###########################

function(set_cryptography_lib CRYPTO_LIBRARIES)
    if (NOT DEFINED CRYPTO_LIB)
        message(FATAL_ERROR "Crypto library could not be inferred so must be specified for linkage")
    endif (NOT DEFINED CRYPTO_LIB)

    set(CRYPTO_LIBRARIES "")

    if (DEFINED OPENSSL_LIBRARIES)
        list(APPEND CRYPTO_LIBRARIES "OpenSSL::SSL")
        list(APPEND CRYPTO_LIBRARIES "${OPENSSL_LIBRARIES}")
        set(LIBACQUIRE_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}")
    endif (DEFINED OPENSSL_LIBRARIES)

    if (DEFINED LIBACQUIRE_USE_WINCRYPT)
        list(APPEND CRYPTO_LIBRARIES "advapi32")
        list(APPEND CRYPTO_LIBRARIES "crypt32")
    endif (DEFINED LIBACQUIRE_USE_WINCRYPT)
    set(CRYPTO_LIBRARIES "${CRYPTO_LIBRARIES}" PARENT_SCOPE)
endfunction(set_cryptography_lib)

set_cryptography_lib(CRYPTO_LIBRARIES)

if (DEFINED CRYPTO_LIBRARIES AND NOT CRYPTO_LIBRARIES STREQUAL "")
    list(APPEND LIBACQUIRE_LIBRARIES "${CRYPTO_LIBRARIES}")
elseif (NOT DEFINED LIBACQUIRE_USE_COMMON_CRYPTO AND  # Link not needed
        NOT DEFINED CHECKSUM_LIBRARIES OR
        CHECKSUM_LIBRARIES STREQUAL "" OR
        CHECKSUM_LIBRARIES STREQUAL "crc32c" # Needs actual crypto not just crc32c impl
)
    message(FATAL_ERROR "CRYPTO_LIBRARIES not set for linkage")
endif ()

message(STATUS "crypt LIBACQUIRE_LIBRARIES    = ${LIBACQUIRE_LIBRARIES}")

#########################
# Compression libraries #
#########################

macro(download_extract_miniz download_dir)
    set(MINIZ_VERSION "3.0.2")
    set(MINIZ_BASENAME "miniz-${MINIZ_VERSION}.zip")
    get_filename_component(MINIZ_BASENAME_NO_EXT "${MINIZ_BASENAME}" NAME_WLE)
    set(MINIZ_BASENAME_NO_EXT "${MINIZ_BASENAME_NO_EXT}" CACHE "STRING"
            "Basename extension free location of miniz zip archive")
    set(MINIZ_ZIP_FILE "${download_dir}/${MINIZ_BASENAME}" CACHE "FILEPATH" "Location of miniz zip archive")
    if (NOT EXISTS "${MINIZ_ZIP_FILE}")
        file(DOWNLOAD
                "https://github.com/richgel999/miniz/releases/download/${MINIZ_VERSION}/miniz-${MINIZ_VERSION}.zip"
                "${MINIZ_ZIP_FILE}"
                EXPECTED_HASH "SHA256=ada38db0b703a56d3dd6d57bf84a9c5d664921d870d8fea4db153979fb5332c5")
        file(ARCHIVE_EXTRACT INPUT "${MINIZ_ZIP_FILE}"
                DESTINATION "${download_dir}")
    endif (NOT EXISTS "${MINIZ_ZIP_FILE}")

    if (NOT EXISTS "${download_dir}/zip.h")
        set(KUBA_ZIP_VER "d7a2252")
        file(DOWNLOAD
                "https://raw.githubusercontent.com/kuba--/zip/${KUBA_ZIP_VER}/src/zip.h"
                "${download_dir}/zip.h"
                EXPECTED_HASH "SHA256=3dda9a244fe05a8f003477a6e1dfb6844071d003773ab18a7708b860d8896506")

        file(DOWNLOAD
                "https://raw.githubusercontent.com/SamuelMarks/zip/refs/heads/msvc-type-coercion/src/zip.c"
                "${download_dir}/zip.c"
                EXPECTED_HASH "SHA256=729701ab2f0ae291a920cee448afa06e2932f08468f27c3cfbcc45f283013688")

        file(DOWNLOAD
                "https://raw.githubusercontent.com/kuba--/zip/${KUBA_ZIP_VER}/src/miniz.h"
                "${download_dir}/miniz.h"
                EXPECTED_HASH "SHA256=6f2b1d7d368fecadf21ddf330634dabb250e505834fe1b76394aa5834e6e0869")
    endif (NOT EXISTS "${download_dir}/zip.h")

    file(TO_NATIVE_PATH "${MINIZ_ZIP_FILE}" MINIZ_ZIP_FILE)

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
        string(REPLACE "\\" "\\\\" MINIZ_ZIP_FILE "${MINIZ_ZIP_FILE}")
    endif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
endmacro(download_extract_miniz download_dir)

function(download_unarchiver EXTRACT_LIB)
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

        target_compile_features("${EXTRACT_LIB}" PRIVATE "c_std_${CMAKE_C_STANDARD}")

        set(gcc_like "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
        set(msvc "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")
        target_compile_options(
                "${EXTRACT_LIB}"
                PRIVATE
                "$<${gcc_like}:$<BUILD_INTERFACE:-Wno-long-long>>"
                "$<${msvc}:$<BUILD_INTERFACE:-W3;-WX;-Zi>>"
        )
        target_compile_definitions(
                "${EXTRACT_LIB}"
                PRIVATE
                -D_CRT_SECURE_NO_WARNINGS
        )
        include(GNUInstallDirs)
        target_include_directories(
                "${EXTRACT_LIB}"
                PUBLIC
                "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        )

        set_target_properties(
                "${EXTRACT_LIB}"
                PROPERTIES
                LINKER_LANGUAGE
                C
        )

        include(GNUInstallDirs)
        install(TARGETS "${EXTRACT_LIB}"
                EXPORT "${EXTRACT_LIB}Targets"
                ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
        install(FILES ${Header_Files}
                TYPE "INCLUDE")
        install(EXPORT "${EXTRACT_LIB}Targets" DESTINATION "${CMAKE_INSTALL_DATADIR}/${EXTRACT_LIB}")
    endif (NOT TARGET "${EXTRACT_LIB}")
endfunction(download_unarchiver EXTRACT_LIB)

function(set_download_unarchiver EXTRACT_LIB)
    download_unarchiver(EXTRACT_LIB)
    if (NOT DEFINED EXTRACT_LIB)
        message(FATAL_ERROR "EXTRACT_LIB is not defined")
    endif (NOT DEFINED EXTRACT_LIB)
    set(EXTRACT_LIB "${EXTRACT_LIB}" PARENT_SCOPE)
    unset(LIBACQUIRE_USE_ZLIB)
    unset(LIBACQUIRE_USE_ZLIB PARENT_SCOPE)
    remove_definitions(-DLIBACQUIRE_USE_ZLIB)
    add_compile_definitions(LIBACQUIRE_USE_MINIZ=1)
    set(LIBACQUIRE_USE_MINIZ 1)
    set(LIBACQUIRE_USE_MINIZ "${LIBACQUIRE_USE_MINIZ}" PARENT_SCOPE)
endfunction(set_download_unarchiver EXTRACT_LIB)

if (DEFINED EXTRACT_LIB)
    set("USE_${EXTRACT_LIB}" "1" PARENT_SCOPE)
else ()
    message(FATAL_ERROR "Compression API not set, did you run `set_extract_lib()`?")
endif ()

function(set_extraction_api)
    if (DEFINED LIBACQUIRE_USE_ZLIB)
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
    else ()
        set_download_unarchiver(EXTRACT_LIB)
        set(EXTRACT_LIB "${EXTRACT_LIB}" PARENT_SCOPE)
    endif (DEFINED LIBACQUIRE_USE_ZLIB)
    set(EXTRACT_LIBRARIES "${EXTRACT_LIB}" PARENT_SCOPE)
endfunction(set_extraction_api)

set_extraction_api(EXTRACT_LIBRARIES)

if (DEFINED EXTRACT_LIBRARIES AND NOT EXTRACT_LIBRARIES STREQUAL "")
    list(APPEND LIBACQUIRE_LIBRARIES "${EXTRACT_LIBRARIES}")
else ()
    message(FATAL_ERROR "EXTRACT_LIBRARIES not set for linkage")
endif ()

message(STATUS "compress LIBACQUIRE_LIBRARIES = ${LIBACQUIRE_LIBRARIES}")

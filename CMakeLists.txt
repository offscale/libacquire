cmake_minimum_required(VERSION 3.0)
project(libacquire
        VERSION "0.0.1"
        LANGUAGES C)

set(HOMEPAGE_URL "https://github.com/offscale/libacquire")
set(DESCRIPTION "Downloads using libcurl—if not Windows or built with USE_LIBCURL—or Windows APIs.")
set(CMAKE_C_STANDARD 90)
set(CMAKE_VERBOSE_MAKEFILE ON)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)

if (NOT _GNU_SOURCE)
    check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

    if (NOT _GNU_SOURCE)
        unset(_GNU_SOURCE CACHE)
        check_symbol_exists(_GNU_SOURCE "features.h" _GNU_SOURCE)
    endif ()
endif ()

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif ()

############################
# Initial src dependencies #
############################

set(src src/main.c src/cli src/acquire)

##############################################################
# CMake options that can be overridden at configuration time #
##############################################################

option(USE_LIBCURL "Enable libcurl (defaults to `YES` on non-Windows)" OFF)

# "Crypto library to use, defaults to Linux,BSD,SunOS: OpenSSL; Windows: STunnel; macOS: LibreSSL"
if (DEFINED CRYPTO_LIB)
    message(FATAL_ERROR "TODO")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    check_include_files("CommonCrypto/CommonCrypto.h;CommonCrypto/CommonDigest.h" HAVE_COMMON_CRYPTO_H)
    if (NOT HAVE_COMMON_CRYPTO_H)
        message(FATAL_ERROR "CommonCrypto.h not found")
    endif ()
    set(CRYPTO_LIB CommonCrypto)
    set(USE_COMMON_CRYPTO 1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    set(CRYPTO_LIB LibreSSL)
    include(cmake/FindLibreSSL.cmake)
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
    set(USE_LIBRESSL 1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux"
        OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD"
        OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    set(CRYPTO_LIB OpenSSL)
    set(USE_OPENSSL 1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(FATAL_ERROR "TODO")
elseif (CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    message(FATAL_ERROR "TODO")
endif ()

function(gettempdir VARNAME)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(${VARNAME} "$ENV{TEMP}" PARENT_SCOPE)
    else ()
        set(${VARNAME} "$ENV{TMPDIR}" PARENT_SCOPE)
        message("-- Using ${VARNAME} $ENV{TMPDIR}")
        if (NOT ${VARNAME})
            set(${VARNAME} "/tmp" PARENT_SCOPE)
        endif ()
    endif ()
endfunction(gettempdir)

gettempdir(TMPDIR_DEFAULT)
set(TMPDIR "${TMPDIR_DEFAULT}" CACHE PATH "Select tmpdir, will use default otherwise")

if (NOT IS_DIRECTORY "${TMPDIR}")
    message(FATAL_ERROR "Cannot determine usable temporary directory, found ${TMPDIR}")
endif ()

check_include_file(limits.h HAVE_LIMITS_H)
if (HAVE_LIMITS_H)
    set(HAVE_SYS_LIMITS_H 0)  # Don't actually care if it exists or not, always choose limits.h
else ()
    check_include_file(sys/limits.h HAVE_SYS_LIMITS_H)
endif ()

#################
# SSL libraries #
#################

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
    message("GOT THERE")
    find_package(LibreSSL REQUIRED)
elseif (CRYPTO_LIB STREQUAL "CommonCrypto" OR CRYPTO_LIB STREQUAL "OpenSSL")
    list(APPEND src src/openssl.h)
else ()
    message("CRYPTO_LIB is ${CRYPTO_LIB}")
endif ()

########################
# HTTP/HTTPS libraries #
########################

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Windows") OR USE_LIBCURL)
    set(USE_LIBCURL ON)
    include(cmake/FindCurlCustom.cmake)
    get_curl(CURL_LINK_LIBRARIES)
    list(APPEND src src/libcurl)
else ()
    message(FATAL_ERROR "TODO: Windows APIs")
endif ()

if (USE_LIBCURL)
    set(LIBCURL 1)
else ()
    set(LIBCURL 0)
endif ()

################################
# Configure file (interpolate) #
################################

configure_file(src/config.h.in ${PROJECT_BINARY_DIR}/src/config.h @ONLY)

#############################################
# Compiler, library, and exec configuration #
#############################################

if (CMAKE_COMPILER_IS_MSVC)
    # TODO: Configure compiler flags for this one
    message("TODO: Configure compiler flags for this one")
else ()
    add_compile_options(
            "$<$<CONFIG:DEBUG>:-Wall>"
            "$<$<CONFIG:DEBUG>:-Werror>"
            "$<$<CONFIG:DEBUG>:-pedantic>"
            "$<$<CONFIG:DEBUG>:-Wno-missing-braces>"
            "$<$<CONFIG:RELEASE>:-O3>"
    )

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # add_definitions(-D_GNU_SOURCE)
        add_compile_options("$<$<CONFIG:DEBUG>:-pedantic>")
    else ()
        add_compile_options("$<$<CONFIG:DEBUG>:-Wpedantic>")
    endif ()
endif ()

add_executable(acquire ${src})
target_include_directories(acquire PUBLIC ${PROJECT_BINARY_DIR}/src)

if (DEFINED CURL_LINK_LIBRARIES)
    target_link_libraries(acquire ${CURL_LINK_LIBRARIES})
endif ()

if (DEFINED OPENSSL_LIBRARIES)
    target_link_libraries(acquire OpenSSL::SSL)
    target_link_libraries(acquire ${OPENSSL_LIBRARIES})
    target_include_directories(acquire PRIVATE ${OPENSSL_INCLUDE_DIR})
endif ()

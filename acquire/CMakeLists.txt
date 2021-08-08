get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" LIBRARY_NAME "${LIBRARY_NAME}")

include(GenerateExportHeader)

set(Header_Files "acquire.h")
source_group("Header Files" FILES "${Header_Files}")

##############################################################
# CMake options that can be overridden at configuration time #
##############################################################

option(USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" OFF)

# "Crypto library to use, defaults to Linux,BSD,SunOS: OpenSSL; Windows: STunnel; macOS: LibreSSL"
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT DEFINED CRYPTO_LIB)
  check_include_files("CommonCrypto/CommonCrypto.h;CommonCrypto/CommonDigest.h" HAVE_COMMON_CRYPTO_H)
  if (NOT HAVE_COMMON_CRYPTO_H)
    message(FATAL_ERROR "CommonCrypto.h not found")
  endif ()
  set(CRYPTO_LIB CommonCrypto)
  set(USE_COMMON_CRYPTO 1)
elseif ((CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" AND NOT DEFINED CRYPTO_LIB) OR CRYPTO_LIB STREQUAL "LibreSSL")
  set(CRYPTO_LIB LibreSSL)
  include(../cmake/FindLibreSSL.cmake)
  set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
  set(USE_LIBRESSL 1)
elseif ((CMAKE_SYSTEM_NAME STREQUAL "Linux"
        OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD"
        OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        AND NOT DEFINED CRYPTO_LIB
        OR CRYPTO_LIB STREQUAL "OpenSSL")
  set(CRYPTO_LIB OpenSSL)
  set(USE_OPENSSL 1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT DEFINED CRYPTO_LIB)
  set(CRYPTO_LIB "WinCrypt")
  set(USE_WINCRYPT 1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "SunOS" AND NOT DEFINED CRYPTO_LIB)
  message(FATAL_ERROR "TODO")
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
  find_package(LibreSSL REQUIRED)
elseif (CRYPTO_LIB STREQUAL "CommonCrypto" OR CRYPTO_LIB STREQUAL "OpenSSL")
  list(APPEND Header_Files "openssl.h")
elseif (CRYPTO_LIB STREQUAL "WinCrypt")
  list(APPEND Header_Files "wincrypt.h")
elseif (NOT DEFINED CRYPTO_LIB)
  message(FATAL_ERROR "CRYPTO_LIB undefined")
else ()
  message("CRYPTO_LIB is ${CRYPTO_LIB}")
endif ()

########################
# HTTP/HTTPS libraries #
########################

if ((CMAKE_SYSTEM_NAME STREQUAL "FreeBSD"
        OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        AND USE_LIBCURL STREQUAL "OFF")
  set(USE_LIBFETCH 1)
  list(APPEND Header_Files "libfetch.h")
elseif (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" AND USE_LIBCURL STREQUAL "OFF")
  set(USE_OPENBSD_FTP 1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(USE_WININET 1)
  list(APPEND Header_Files "wininet.h")
else ()
  set(USE_LIBCURL 1)
  include(../cmake/FindCurlCustom.cmake)
  get_curl(CURL_LINK_LIBRARIES)
  list(APPEND Header_Files "libcurl.h")
endif ()

################################
# Configure file (interpolate) #
################################

configure_file(config.h.in "${PROJECT_BINARY_DIR}/src/config.h" @ONLY)

#############################
# Test and target formation #
#############################

source_group("Header Files" FILES "${Header_Files}")

add_library("${LIBRARY_NAME}" INTERFACE)
target_sources("${LIBRARY_NAME}" PRIVATE "${Header_Files}")

target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
        "$<INSTALL_INTERFACE:include>"
)

set(_libs "")
if (DEFINED CURL_LINK_LIBRARIES)
  list(APPEND _libs "${CURL_LINK_LIBRARIES}")
elseif (DEFINED USE_WININET)
  list(APPEND _libs "wininet")
elseif (DEFINED USE_LIBFETCH)
  list(APPEND _libs "fetch")
endif ()

if (DEFINED OPENSSL_LIBRARIES)
  list(APPEND _libs "OpenSSL::SSL")
  list(APPEND _libs "${OPENSSL_LIBRARIES}")
  target_include_directories("${LIBRARY_NAME}" PRIVATE ${OPENSSL_INCLUDE_DIR})
endif ()

target_link_libraries("${LIBRARY_NAME}" INTERFACE "${PROJECT_LOWER_NAME}_compiler_flags" "${_libs}")

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

#########################
# Export / installation #
#########################

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

# install rules
set(installable_libs "${LIBRARY_NAME}" "${PROJECT_LOWER_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
  list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "bin"
        EXPORT "${LIBRARY_NAME}Targets")
install(FILES
        "${PROJECT_BINARY_DIR}/src/config.h"
        "${Header_Files}"
        DESTINATION
        "include")

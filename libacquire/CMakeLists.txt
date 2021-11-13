get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" LIBRARY_NAME "${LIBRARY_NAME}")

include("${CMAKE_SOURCE_DIR}/cmake/FindCurlCustom.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/libacquire_helpers.cmake")

set(Header_Files "acquire_download.h")
if (NOT HAS_STDBOOL AND NOT MSVC)
  list(APPEND Header_Files "acquire_stdbool.h")
endif ()
source_group("Header Files" FILES "${Header_Files}")

##############################################################
# CMake options that can be overridden at configuration time #
##############################################################

option(USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" OFF)

set(_Header_Files "")

# Set libacquire dependencies

include("${CMAKE_SOURCE_DIR}/cmake/libacquire_helpers.cmake")

set_crypto_lib()

set_ssl_lib()

set_http_https_lib()

set_extract_lib()

include("${CMAKE_SOURCE_DIR}/cmake/FindLibAcquire.cmake")

foreach (_library "HTTPS" "CRYPTO" "EXTRACT")
  message(STATUS "[libacquire] ${_library}_LIB:\t${${_library}_LIB}")
endforeach (_library "HTTPS" "CRYPTO" "EXTRACT")

################################
# Configure file (interpolate) #
################################

configure_file(acquire_config.h.in "${PROJECT_BINARY_DIR}/src/acquire_config.h" @ONLY)

#############################
# Test and target formation #
#############################

source_group("Header Files" FILES "${Header_Files}")

add_library("${LIBRARY_NAME}" INTERFACE)
target_sources("${LIBRARY_NAME}" PRIVATE "${Header_Files}")

set(_include_dirs
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
        "$<INSTALL_INTERFACE:include>")

if (DEFINED LIBACQUIRE_INCLUDE_DIR)
  list(APPEND _include_dirs "$<BUILD_INTERFACE:${LIBACQUIRE_INCLUDE_DIR}>")
endif ()

target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "${_include_dirs}"
)
target_link_libraries(
        "${LIBRARY_NAME}"
        INTERFACE
        "${PROJECT_NAME}_compiler_flags"
        "${LIBACQUIRE_LIBRARIES}"
)

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

option(BUILD_OPENBSD_FTP "Build `ftp` binary from OpenBSD" OFF)

if (BUILD_OPENBSD_FTP)
  add_subdirectory("openbsd_ftp")
endif (BUILD_OPENBSD_FTP)

option(BUILD_FREEBSD_LIBFETCH "Build `libfetch` library from FreeBSD" OFF)

if (BUILD_FREEBSD_LIBFETCH OR USE_MY_LIBFETCH)
  add_subdirectory("freebsd_libfetch")
endif (BUILD_FREEBSD_LIBFETCH OR USE_MY_LIBFETCH)

#######################################
# Export / installation configuration #
#######################################

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

# install rules
set(installable_libs "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
  list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "bin"
        EXPORT "${LIBRARY_NAME}Targets")
install(FILES
        "${PROJECT_BINARY_DIR}/src/acquire_config.h"
        "${Header_Files}"
        DESTINATION
        "include")

cmake_minimum_required(VERSION 3.0)
    project(libacquire
        VERSION "0.0.1"
        LANGUAGES C)
set(HOMEPAGE_URL "https://github.com/offscale/libacquire")
set(DESCRIPTION "Downloads using libcurl—if not Windows or built with USE_LIBCURL—or Windows APIs.")
set(CMAKE_C_STANDARD 90)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(USE_LIBCURL "Enable libcurl (defaults to `YES` on non-Windows)" OFF)

include(CheckIncludeFile)

function(gettempdir VARNAME)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(${VARNAME} "$ENV{TEMP}" PARENT_SCOPE)
    else()
        set(${VARNAME} "$ENV{TMPDIR}" PARENT_SCOPE)
        message("VARNAME = ${VARNAME} $ENV{TMPDIR}")
        if (NOT ${VARNAME})
            set(${VARNAME} "/tmp" PARENT_SCOPE)
        endif()
    endif()
endfunction(gettempdir)

gettempdir(TMPDIR_DEFAULT)
set(TMPDIR "${TMPDIR_DEFAULT}" CACHE PATH "Select tmpdir, will use default otherwise")

if(NOT IS_DIRECTORY "${TMPDIR}")
    message(FATAL_ERROR "Cannot determine usable temporary directory, found ${TMPDIR}")
endif()

check_include_file(limits.h HAVE_LIMITS_H)
if (HAVE_LIMITS_H)
    set(HAVE_SYS_LIMITS_H 0)  # Don't actually care if it exists or not, always choose limits.h
else()
    check_include_file(sys/limits.h HAVE_SYS_LIMITS_H)
endif()

set(src src/main.c src/cli src/acquire)

if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Windows") OR USE_LIBCURL)
    set(USE_LIBCURL ON)
    set(CURL_NO_OLDIES ON)

    # curl version >=7.57 can have config files
    find_package(CURL QUIET COMPONENTS libcurl CONFIG)

    # This conditional if/else block from https://gitlab.kitware.com/cmake/cmake/-/issues/18378#note_467156
    if(CURL_FOUND)
        message(STATUS "Using ${CURL_LIBRARIES} v${CURL_VERSION}")
        get_target_property(LIBCURL_TYPE CURL::libcurl TYPE)
        # CURL_STATICLIB is missing for curl versions <7.61.1
        if(CURL_VERSION VERSION_LESS 7.61.1 AND LIBCURL_TYPE STREQUAL "STATIC_LIBRARY")
            set_target_properties(CURL::libcurl PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB)
        endif()
    else()
        # try to use libcurl.pc if installed
        set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} lib)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} _imp.lib -d.lib -d_imp.lib)
        find_package(PkgConfig)
        if(PkgConfig_FOUND)
            set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
            pkg_search_module(CURL libcurl)
            if(CURL_FOUND)
                # CMake <3.12 workaround
                if(NOT CURL_LINK_LIBRARIES)
                    find_library(CURL_LINK_LIBRARIES NAMES ${CURL_LIBRARIES}
                            HINTS "${CURL_LIBDIR}" NO_DEFAULT_PATH)
                    if(NOT CURL_LINK_LIBRARIES)
                        message(FATAL_ERROR "curl is not installed; install libcurl then try again")
                    endif()
                endif()
                if(NOT CURL_LINK_LIBRARIES MATCHES "_imp.lib$|${CMAKE_SHARED_LIBRARY_SUFFIX}$")
                    list(REMOVE_ITEM CURL_STATIC_LIBRARIES ${CURL_LIBRARIES})
                    add_library(CURL::libcurl STATIC IMPORTED)
                    set_target_properties(CURL::libcurl PROPERTIES
                            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_STATIC_INCLUDE_DIRS}"
                            INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
                            INTERFACE_LINK_LIBRARIES "${CURL_STATIC_LIBRARIES}"
                            IMPORTED_LINK_INTERFACE_LANGUAGES C
                            IMPORTED_LOCATION "${CURL_LINK_LIBRARIES}")
                    link_directories(CURL_STATIC_LIBRARY_DIRS)
                else()
                    add_library(CURL::libcurl SHARED IMPORTED)
                    set_target_properties(CURL::libcurl PROPERTIES
                            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
                    if(WIN32)
                        set_target_properties(CURL::libcurl PROPERTIES
                                IMPORTED_IMPLIB "${CURL_LINK_LIBRARIES}")
                    else()
                        set_target_properties(CURL::libcurl PROPERTIES
                                IMPORTED_LOCATION "${CURL_LINK_LIBRARIES}")
                    endif()
                endif()
                message(STATUS "Using ${CURL_LIBRARIES} v${CURL_VERSION}")
            endif()
        endif()
        # try old module from CMake distribution
        if(NOT CURL_FOUND)
            find_package(CURL REQUIRED COMPONENTS libcurl MODULE)
            if(NOT CURL_LIBRARIES MATCHES "_imp.lib$|${CMAKE_SHARED_LIBRARY_SUFFIX}$")
                add_library(CURL::libcurl STATIC IMPORTED)
                set_target_properties(CURL::libcurl PROPERTIES
                        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
                        INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
                        IMPORTED_LINK_INTERFACE_LANGUAGES C
                        IMPORTED_LOCATION "${CURL_LIBRARIES}")
            else()
                add_library(CURL::libcurl SHARED IMPORTED)
                set_target_properties(CURL::libcurl PROPERTIES
                        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
                if(WIN32)
                    set_target_properties(CURL::libcurl PROPERTIES
                            IMPORTED_IMPLIB "${CURL_LIBRARIES}")
                else()
                    set_target_properties(CURL::libcurl PROPERTIES
                            IMPORTED_LOCATION "${CURL_LIBRARIES}")
                endif()
            endif()
            set(CURL_VERSION ${CURL_VERSION_STRING})
        endif()
    endif()

    if (NOT CURL_FOUND)
        message(FATAL_ERROR "curl is not installed; install libcurl then try again")
    endif()

    list(APPEND src src/libcurl)
else()
    message(FATAL_ERROR "TODO: Windows APIs")
endif()

if (USE_LIBCURL)
    set(LIBCURL 1)
else()
    set(LIBCURL 0)
endif()

configure_file(src/config.h.in ${PROJECT_BINARY_DIR}/src/config.h)

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options("$<$<CONFIG:DEBUG>:-Wall>" "$<$<CONFIG:RELEASE>:-O3>")
    add_compile_options("$<$<CONFIG:DEBUG>:-Werror>")

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_compile_options("$<$<CONFIG:DEBUG>:-pedantic>")
    else()
        add_compile_options("$<$<CONFIG:DEBUG>:-Wpedantic>")
    endif()
else()
    # TODO: Configure compiler flags for this one
    message("TODO: Configure compiler flags for this one")
endif()

add_executable(acquire ${src})
message(STATUS "CURL_LINK_LIBRARIES is ${CURL_LINK_LIBRARIES}")
target_include_directories(acquire PUBLIC ${PROJECT_BINARY_DIR}/src)

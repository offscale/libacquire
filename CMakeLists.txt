cmake_minimum_required(VERSION 3.19)
project(libacquire
        VERSION "0.0.1"
        LANGUAGES C)

set(HOMEPAGE_URL "https://github.com/offscale/libacquire")
set(DESCRIPTION "The core for your package manager. Features: download, verify, and extract.")

set(CMAKE_C_STANDARD 90)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

if (DEFINED USE_MY_LIBFETCH) # Experiments
    add_compile_definitions(USE_MY_LIBFETCH=1)

    if (NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" AND NOT CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        add_compile_definitions(WITH_SSL=1)
        set(WITH_SSL 1)
    else ()
        add_compile_definitions(HAVE_REALLOCARRAY=1)
    endif ()
endif (DEFINED USE_MY_LIBFETCH)

###############################
# System symbol configuration #
###############################

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CTest)

if (CMAKE_C_STANDARD GREATER 90)
    CHECK_INCLUDE_FILE("stdbool.h" HAS_STDBOOL)
    if (HAS_STDBOOL)
        add_compile_definitions(HAS_STDBOOL=1)
    endif ()
endif ()
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

function (gettempdir VARNAME)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(${VARNAME} "$ENV{TEMP}" PARENT_SCOPE)
    else ()
        set(${VARNAME} "$ENV{TMPDIR}" PARENT_SCOPE)
        message("-- Using ${VARNAME} $ENV{TMPDIR}")
        if (NOT ${VARNAME})
            set(${VARNAME} "/tmp" PARENT_SCOPE)
        endif ()
    endif ()
endfunction (gettempdir)

gettempdir(TMPDIR_DEFAULT)
set(TMPDIR "${TMPDIR_DEFAULT}" CACHE PATH "Select tmpdir, will use default otherwise")

if (NOT IS_DIRECTORY "${TMPDIR}")
    message(FATAL_ERROR "Cannot determine usable temporary directory, found ${TMPDIR}")
endif ()

check_include_file(limits.h HAVE_LIMITS_H)
if (HAVE_LIMITS_H)
    add_compile_definitions(HAVE_LIMITS_H=1)
    add_compile_definitions(HAVE_SYS_LIMITS_H=1)  # Don't actually care if it exists or not, always choose limits.h
else ()
    check_include_file(sys/limits.h HAVE_SYS_LIMITS_H)
    if (HAVE_SYS_LIMITS_H)
        add_compile_definitions(HAVE_SYS_LIMITS_H=1)
    endif ()
endif ()

#################################
# Compiler/linker configuration #
#################################

add_library("${PROJECT_NAME}_compiler_flags" INTERFACE)
target_compile_features("${PROJECT_NAME}_compiler_flags" INTERFACE "c_std_${CMAKE_C_STANDARD}")

# add compiler warning flags just when building this project via
# the BUILD_INTERFACE genex
set(gcc_like "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")
target_compile_options(
        "${PROJECT_NAME}_compiler_flags"
        INTERFACE
        "$<${gcc_like}:$<BUILD_INTERFACE:-Wshadow;-Wformat=2;-Wall;-pedantic>>"
        "$<${msvc}:$<BUILD_INTERFACE:-W3;-WX;-Zi;-permissive->>"
)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if (APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif ()

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif ()

if ((DEFINED CMAKE_C_FLAGS AND NOT CMAKE_C_FLAGS STREQUAL "")
        OR (DEFINED CMAKE_CXX_FLAGS AND NOT CMAKE_CXX_FLAGS STREQUAL ""))
    set(TARGET_ARCH_REGEX "^.*-march[= ]([^ ]+).*$")
    string(REGEX MATCH "${TARGET_ARCH_REGEX}" TARGET_ARCH_MATCH ${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS})
    string(REGEX MATCH <match-regex> <out-var> <input>...)
    if (TARGET_ARCH_MATCH)
        string(REGEX REPLACE "${TARGET_ARCH_REGEX}" "\\1" TARGET_ARCH ${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS})
    endif ()
endif ()

########################
# Subprojects to build #
########################

add_subdirectory("libacquire")
add_subdirectory("libacquire/acquire")

################################
# Generate amalgamation header #
################################

include("${CMAKE_SOURCE_DIR}/cmake/libacquire_amalgamation.cmake")

generate_amalgamation_header()

######################
# Test configuration #
######################

if (CMAKE_PROJECT_NAME STREQUAL "libacquire" AND BUILD_TESTING)
    enable_testing()
    add_subdirectory("libacquire/tests")
endif ()

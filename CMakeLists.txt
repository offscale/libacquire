cmake_minimum_required(VERSION 3.0)
project(libacquire
        VERSION "0.0.1"
        LANGUAGES C)

set(HOMEPAGE_URL "https://github.com/offscale/libacquire")
set(DESCRIPTION "The core for your package manager. Features: download, verify, and extract.")
set(CMAKE_C_STANDARD 90)
set(CMAKE_VERBOSE_MAKEFILE ON)

include(cmake/libacquire.cmake)

libacquire()

if (CMAKE_PROJECT_NAME STREQUAL "libacquire" AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()

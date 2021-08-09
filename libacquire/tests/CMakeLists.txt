cmake_minimum_required(VERSION 3.15)
project(libacquire_tests
        VERSION "0.0.1"
        LANGUAGES C)

set(HOMEPAGE_URL "https://github.com/offscale/libacquire")
set(DESCRIPTION "The core for your package manager. Features: download, verify, and extract.")
set(CMAKE_C_STANDARD 90)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory("test_libacquire")

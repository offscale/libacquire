cmake_minimum_required(VERSION 3.0)
project(libacquire_tests
        VERSION "0.0.1"
        LANGUAGES C)

set(HOMEPAGE_URL "https://github.com/offscale/libacquire")
set(DESCRIPTION "The core for your package manager. Features: download, verify, and extract.")
set(CMAKE_C_STANDARD 90)
set(CMAKE_VERBOSE_MAKEFILE ON)

file(DOWNLOAD https://raw.githubusercontent.com/silentbicycle/greatest/master/greatest.h
        ${PROJECT_BINARY_DIR}/downloads/greatest.h)

include_directories(${PROJECT_BINARY_DIR}/downloads)

set(src src/test_foo)

add_executable(test_foo_exe ${src})
#add_library(${PROJECT_NAME}::rexo ALIAS rexo)
add_test(test_foo ${PROJECT_BINARY_DIR}/test_foo_exe)

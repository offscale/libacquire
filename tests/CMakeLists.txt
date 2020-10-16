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

################################
# Configure file (interpolate) #
################################
configure_file(src/config_for_tests.h.in ${PROJECT_BINARY_DIR}/src/config_for_tests.h @ONLY)

################################
# Put directory in path for <> #
################################
include_directories(${PROJECT_BINARY_DIR}/downloads ${PROJECT_BINARY_DIR}/src ../src)

########################
# Set src dependencies #
########################
set(src src/test src/test_fileutils src/test_string_extras)

add_executable(test_libacquire ${src})
#add_library(${PROJECT_NAME}::rexo ALIAS rexo)
add_test(test_libacquire ${PROJECT_BINARY_DIR}/test_libacquire)

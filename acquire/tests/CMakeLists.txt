set(exec_name "test_${PROJECT_NAME}")

#########################
# Dependencies download #
#########################

set(GREATEST_CRC32C "c66c379f")
set(GREATEST_SHA256 "1292593d95c35eeccc89ffa1c91d6fe53b49f81cbf2c2b7758842b7f3186fcc2")
set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/test_downloads")

file(TO_NATIVE_PATH "${DOWNLOAD_DIR}" DOWNLOAD_DIR)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
  string(REPLACE "\\" "\\\\" DOWNLOAD_DIR "${DOWNLOAD_DIR}")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)

file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")
set(GREATEST_BASEFILENAME "greatest.h")
set(GREATEST_FILE "${DOWNLOAD_DIR}/${GREATEST_BASEFILENAME}")
file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
  string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
endif ()

set(GREATEST_URL "https://raw.githubusercontent.com/silentbicycle/greatest/11a6af1/greatest.h")
file(DOWNLOAD "${GREATEST_URL}" "${GREATEST_FILE}"
        EXPECTED_HASH "SHA256=${GREATEST_SHA256}")

################################
# Configure file (interpolate) #
################################

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" _CMAKE_CURRENT_SOURCE_DIR)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_LIST_FILE}" _CMAKE_CURRENT_LIST_FILE)
file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
download_extract_miniz("${DOWNLOAD_DIR}")
file(TO_NATIVE_PATH "${ZLIB_ZIP_FILE}" ZLIB_ZIP_FILE)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
  string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_SOURCE_DIR "${_CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_LIST_FILE "${_CMAKE_CURRENT_LIST_FILE}")
  string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
  string(REPLACE "\\" "\\\\" ZLIB_ZIP_FILE "${ZLIB_ZIP_FILE}")
endif ()

configure_file(config_for_tests.h.in "${PROJECT_BINARY_DIR}/src/config_for_tests.h" @ONLY)

###############################
# Set libacquire dependencies #
###############################

include("${CMAKE_SOURCE_DIR}/cmake/libacquire_helpers.cmake")

set(CMAKE_CURRENT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}")
set_checksum_lib()

set_crypto_lib()

set_ssl_lib()

set_http_https_lib()

set_extract_lib()

include("${CMAKE_SOURCE_DIR}/cmake/FindLibAcquire.cmake")

set_checksum_lib()

set(_targets "${exec_name}" "test_amalgamation")
set(_archives "libarchive" "miniz") # "zlib"
set(All_Header_Files "${GREATEST_FILE}")

foreach (_library "HTTPS" "CRYPTO" "EXTRACT" "CHECKSUM")
  message(STATUS "[test_libacquire] ${_library}_LIB: \t${${_library}_LIB}")
endforeach (_library "HTTPS" "CRYPTO" "EXTRACT" "CHECKSUM")

macro (test_wrapper)
    target_include_directories(
            "${EXEC_NAME}"
            PRIVATE
            "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/acquire>"
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/acquire>"
            "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
    )

    add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")
endmacro (test_wrapper)

foreach (EXEC_NAME ${_targets})
    math(EXPR "${exec_name}_count" "${${exec_name}_count} + 1")
    set(Header_Files
            "${CMAKE_SOURCE_DIR}/acquire/tests/test_checksum.h"
            "${CMAKE_SOURCE_DIR}/acquire/tests/test_download.h"
            "${CMAKE_SOURCE_DIR}/acquire/tests/test_fileutils.h"
            "${CMAKE_SOURCE_DIR}/acquire/tests/test_string_extras.h"
            "${CMAKE_SOURCE_DIR}/acquire/tests/test_url_utils.h"
    )
    source_group("Header Files" FILES "${Header_Files}")

    set(Source_Files "test.c")
    source_group("Source Files" FILES "${Source_Files}")

    add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

    target_link_libraries(
            "${EXEC_NAME}"
            PRIVATE
            "lib${PROJECT_NAME}"
            "${PROJECT_NAME}_compiler_flags"
            "${CHECKSUM_LIBRARIES}"
    )
    target_include_directories(
            "${EXEC_NAME}"
            PRIVATE
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>"
    )
    test_wrapper()
endforeach (EXEC_NAME ${_targets})

set(SUBPROJECT_NAME "test_extract")
foreach (ARCHIVE_LIB ${_archives})
    set(EXEC_NAME "${SUBPROJECT_NAME}_${ARCHIVE_LIB}")

    set(Header_Files "test_extract.h")
    source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")

    set(Source_Files "test_extract.c")
    source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

    add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
    string(TOUPPER "${ARCHIVE_LIB}" ARCHIVE_LIB_UPPER)
    set(_dir "${DOWNLOAD_DIR}/${EXTRACT_DIR}extract/${ARCHIVE_LIB}")
    if (NOT IS_DIRECTORY "${_dir}")
        file(MAKE_DIRECTORY "${_dir}")
    endif (NOT IS_DIRECTORY "${_dir}")
    target_compile_definitions(
            "${EXEC_NAME}"
            PRIVATE
            "ARCHIVE_LIB=\"${ARCHIVE_LIB}\""
            "USE_${ARCHIVE_LIB_UPPER}=1"
            "ARCHIVE_LIB_NAME=<acquire_${ARCHIVE_LIB}.h>"
    )
    set(_libs "${LIBACQUIRE_LIBRARIES}")
    if (ARCHIVE_LIB STREQUAL "miniz")
        set(archive_lib "${ARCHIVE_LIB}")
    elseif (ARCHIVE_LIB STREQUAL "libarchive")
        include("${CMAKE_SOURCE_DIR}/cmake/FindLibArchiveCustom.cmake")
        target_include_directories("${EXEC_NAME}" PRIVATE "${LibArchive_INCLUDE_DIRS}")
        set(archive_lib "${LibArchive_LIBRARIES}")
    endif ()
    #target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBACQUIRE_LIBRARIES}" "${archive_lib}")
    message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
    target_link_libraries(
            "${EXEC_NAME}"
            PRIVATE
            "lib${PROJECT_NAME}"
            "${archive_lib}"
    )
    target_compile_definitions("${EXEC_NAME}" PRIVATE LIBACQUIRE_IMPLEMENTATION=1)

    test_wrapper()
endforeach (ARCHIVE_LIB ${_archives})

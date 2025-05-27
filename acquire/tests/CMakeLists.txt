set(exec_name "test_${PROJECT_NAME}")

#########################
# Dependencies download #
#########################

set(GREATEST_CRC32C "c66c379f")
set(GREATEST_SHA256 "b3f89ecad2301c64c580f328cc03a74d92f4cd4bf42ab18d3d4bfb7b8be5b948")

set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/test_downloads")

file(TO_NATIVE_PATH "${DOWNLOAD_DIR}" DOWNLOAD_DIR)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
    string(REPLACE "\\" "\\\\" DOWNLOAD_DIR "${DOWNLOAD_DIR}")
endif ()

file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")

set(GREATEST_BASEFILENAME "greatest.h")
set(GREATEST_FILE "${DOWNLOAD_DIR}/${GREATEST_BASEFILENAME}")
file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
    string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
endif ()

set(GREATEST_URL "https://raw.githubusercontent.com/SamuelMarks/greatest/cmake-and-msvc/greatest.h")

file(DOWNLOAD "${GREATEST_URL}" "${GREATEST_FILE}"
        EXPECTED_HASH "SHA256=${GREATEST_SHA256}")

################################
# Configure file (interpolate) #
################################

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" _CMAKE_CURRENT_SOURCE_DIR)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_LIST_FILE}" _CMAKE_CURRENT_LIST_FILE)
file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
    string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_SOURCE_DIR "${_CMAKE_CURRENT_SOURCE_DIR}")
    string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_LIST_FILE "${_CMAKE_CURRENT_LIST_FILE}")
    string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
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

foreach (_library "HTTPS" "CRYPTO" "EXTRACT" "CHECKSUM")
    message(STATUS "[test_libacquire] ${_library}_LIB: \t${${_library}_LIB}")
endforeach ()

list(REMOVE_DUPLICATES CHECKSUM_LIBRARIES)

set(_targets "${exec_name}" "test_amalgamation")
set(_archives "libarchive" "miniz")

macro (set_backend_definitions TARGET)
    if(USE_LIBCURL)
        target_compile_definitions(${TARGET} PRIVATE USE_LIBCURL=1)
    elseif(USE_LIBFETCH)
        target_compile_definitions(${TARGET} PRIVATE USE_LIBFETCH=1)
    elseif(USE_WININET)
        target_compile_definitions(${TARGET} PRIVATE USE_WININET=1)
    elseif(USE_OPENBSD_FTP)
        target_compile_definitions(${TARGET} PRIVATE USE_OPENBSD_FTP=1)
    endif()

    if(USE_MINIZ)
        target_compile_definitions(${TARGET} PRIVATE USE_MINIZ=1)
    elseif(USE_LIBARCHIVE)
        target_compile_definitions(${TARGET} PRIVATE USE_LIBARCHIVE=1)
    endif()
endmacro()

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
endmacro()

foreach (EXEC_NAME ${_targets})
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

    set_backend_definitions(${EXEC_NAME})

    target_link_libraries(
            "${EXEC_NAME}"
            PRIVATE
            "${PROJECT_NAME}"
            "${PROJECT_NAME}_impl"
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
endforeach()

# Extraction archive tests
foreach (ARCHIVE_LIB ${_archives})
    set(EXEC_NAME "test_extract_${ARCHIVE_LIB}")

    set(Header_Files "test_extract.h")
    source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")

    set(Source_Files "test_extract.c")
    source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

    add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

    if(ARCHIVE_LIB STREQUAL "miniz")
        target_compile_definitions("${EXEC_NAME}" PRIVATE USE_MINIZ=1)
    elseif(ARCHIVE_LIB STREQUAL "libarchive")
        target_compile_definitions("${EXEC_NAME}" PRIVATE USE_LIBARCHIVE=1)
        include("${CMAKE_SOURCE_DIR}/cmake/FindLibArchiveCustom.cmake")
        target_include_directories("${EXEC_NAME}" PRIVATE "${LibArchive_INCLUDE_DIRS}")
    else()
        message(FATAL_ERROR "Unknown ARCHIVE_LIB: ${ARCHIVE_LIB}")
    endif()

    target_compile_definitions("${EXEC_NAME}" PRIVATE ARCHIVE_LIB=${ARCHIVE_LIB})

    target_compile_definitions("${EXEC_NAME}" PRIVATE ARCHIVE_HEADER_NAME=acquire_${ARCHIVE_LIB}.h)

    set_backend_definitions(${EXEC_NAME})

    target_link_libraries(
            "${EXEC_NAME}"
            PRIVATE
            "${PROJECT_NAME}"
            "${PROJECT_NAME}_impl"
            $<$<STREQUAL:${ARCHIVE_LIB},libarchive>:${LibArchive_LIBRARIES}>
            $<$<STREQUAL:${ARCHIVE_LIB},miniz>:miniz>
    )

    test_wrapper()
endforeach()

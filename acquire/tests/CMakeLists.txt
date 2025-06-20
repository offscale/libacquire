set(exec_name "test_${PROJECT_NAME}")

#########################
# Dependencies download #
#########################

set(GREATEST_CRC32C "f83d6ce5")
set(GREATEST_SHA256 "b3f89ecad2301c64c580f328cc03a74d92f4cd4bf42ab18d3d4bfb7b8be5b948")
set(GREATEST_ARCHIVE_SHA256 "67bad60cd59fffc6b5bc65934deb338b61e8399703807458539c82afbba87d31")

set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/test_downloads")

file(TO_NATIVE_PATH "${DOWNLOAD_DIR}" DOWNLOAD_DIR)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
    string(REPLACE "\\" "\\\\" DOWNLOAD_DIR "${DOWNLOAD_DIR}")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)

file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")

set(GREATEST_BASEFILENAME "greatest.h")

set(GREATEST_ARCHIVE "${DOWNLOAD_DIR}/greatest.zip")
set(GREATEST_ARCHIVE_URL "https://github.com/SamuelMarks/greatest/archive/refs/heads/cmake-and-msvc.zip")
set(GREATEST_URL "https://raw.githubusercontent.com/SamuelMarks/greatest/cmake-and-msvc/greatest.h")

if (NOT IS_DIRECTORY "${DOWNLOAD_DIR}/greatest")
    file(DOWNLOAD "${GREATEST_ARCHIVE_URL}" "${GREATEST_ARCHIVE}"
            EXPECTED_HASH "SHA256=${GREATEST_ARCHIVE_SHA256}")

    file(ARCHIVE_EXTRACT INPUT "${GREATEST_ARCHIVE}" DESTINATION "${DOWNLOAD_DIR}")
    file(RENAME "${DOWNLOAD_DIR}/greatest-cmake-and-msvc" "${DOWNLOAD_DIR}/greatest")
endif (NOT IS_DIRECTORY "${DOWNLOAD_DIR}/greatest")
set(GREATEST_FILE "${DOWNLOAD_DIR}/greatest/greatest.h")

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
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)

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
endforeach (_library "HTTPS" "CRYPTO" "EXTRACT" "CHECKSUM")

list(REMOVE_DUPLICATES CHECKSUM_LIBRARIES)

set(_targets "${exec_name}" "test_amalgamation")
if (EXTRACT_LIB STREQUAL "WINCOMPRESSAPI")
    set(_archives "wincompressapi;")
else ()
    set(_archives "libarchive" "miniz")
endif ()

macro(set_backend_definitions TARGET)
    if (LIBACQUIRE_USE_LIBCURL)
        target_compile_definitions(${TARGET} PRIVATE LIBACQUIRE_USE_LIBCURL=1)
    elseif (LIBACQUIRE_USE_LIBFETCH)
        target_compile_definitions(${TARGET} PRIVATE LIBACQUIRE_USE_LIBFETCH=1)
    elseif (LIBACQUIRE_USE_WININET)
        target_compile_definitions(${TARGET} PRIVATE LIBACQUIRE_USE_WININET=1)
    elseif (LIBACQUIRE_USE_OPENBSD_FTP)
        target_compile_definitions(${TARGET} PRIVATE LIBACQUIRE_USE_OPENBSD_FTP=1)
    endif ()

    if (LIBACQUIRE_USE_MINIZ)
        target_compile_definitions(${TARGET} PRIVATE LIBACQUIRE_USE_MINIZ=1)
    elseif (LIBACQUIRE_USE_LIBARCHIVE)
        target_compile_definitions(${TARGET} PRIVATE LIBACQUIRE_USE_LIBARCHIVE=1)
    endif ()
endmacro()

macro(test_wrapper)
    target_include_directories(
            "${EXEC_NAME}"
            PRIVATE
            "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/acquire>"
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/acquire>"
            "$<BUILD_INTERFACE:${DOWNLOAD_DIR}/greatest>"
    )

    add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")
endmacro()

configure_file(config_for_tests.h.in "${PROJECT_BINARY_DIR}/src/config_for_tests.h" @ONLY)

foreach (EXEC_NAME ${_targets})
    set(Header_Files
            "test_checksum.h"
            "test_download.h"
            "test_fileutils.h"
            "test_string_extras.h"
            "test_url_utils.h"
    )
    source_group("Header Files" FILES "${Header_Files}")

    set(Source_Files "test.c")
    source_group("Source Files" FILES "${Source_Files}")

    set_source_files_properties("test_string_extras.c"
            PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1")

    add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
    target_compile_definitions("${EXEC_NAME}" PUBLIC "_${TARGET_ARCH}_")

    set_backend_definitions(${EXEC_NAME})

    target_link_libraries(
            "${EXEC_NAME}"
            PRIVATE
            "${PROJECT_NAME}"
            "${PROJECT_NAME}_compiler_flags"
            #"${CHECKSUM_LIBRARIES}"
    )

    if (LIBACQUIRE_USE_MINIZ)
        find_package(kubazip CONFIG REQUIRED)
        target_link_libraries("${EXEC_NAME}" PRIVATE kubazip::kubazip)
    endif (LIBACQUIRE_USE_MINIZ)

    target_include_directories(
            "${EXEC_NAME}"
            PRIVATE
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>"
    )
    test_wrapper()
endforeach (EXEC_NAME ${_targets})

# Extraction archive tests
if (NOT (EXTRACT_LIB STREQUAL "WINCOMPRESSAPI"))
    foreach (ARCHIVE_LIB ${_archives})
        set(EXEC_NAME "test_extract_${ARCHIVE_LIB}")

        set(Header_Files "test_extract.h")
        source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")

        set(Source_Files "test_extract.c")
        source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

        add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
        target_compile_definitions("${EXEC_NAME}" PUBLIC "_${TARGET_ARCH}_")

        if (ARCHIVE_LIB STREQUAL "miniz")
            target_compile_definitions("${EXEC_NAME}" PRIVATE LIBACQUIRE_USE_MINIZ=1)
        elseif (ARCHIVE_LIB STREQUAL "libarchive")
            target_compile_definitions("${EXEC_NAME}" PRIVATE LIBACQUIRE_USE_LIBARCHIVE=1)
            include("${CMAKE_SOURCE_DIR}/cmake/FindLibArchiveCustom.cmake")
            target_include_directories("${EXEC_NAME}" PRIVATE "${LibArchive_INCLUDE_DIRS}")
        elseif (ARCHIVE_LIB STREQUAL "wincompressapi")
            target_compile_definitions("${EXEC_NAME}" PRIVATE LIBACQUIRE_USE_WINCOMPRESSAPI=1)
            target_link_libraries("${EXEC_NAME}" PRIVATE Cabinet)
            message(DEBUG "TODO: Finish this test")
        else ()
            message(FATAL_ERROR "Unknown ARCHIVE_LIB: ${ARCHIVE_LIB}")
        endif ()

        if (LIBACQUIRE_USE_MINIZ)
            set_source_files_properties("test_extract.c"
                    PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_USE_MINIZ=1")

            find_package(kubazip CONFIG REQUIRED)
            target_link_libraries("${EXEC_NAME}" PRIVATE kubazip::kubazip)
        elseif (LIBACQUIRE_USE_LIBARCHIVE)
            set_source_files_properties("test_extract.c"
                    PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_USE_LIBARCHIVE=1")
        else ()
            set_source_files_properties("test_extract.c"
                    PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1")
        endif ()

        target_compile_definitions(
                "${EXEC_NAME}"
                PRIVATE
                "ARCHIVE_LIB=${ARCHIVE_LIB}"
                "ARCHIVE_HEADER_NAME=acquire_${ARCHIVE_LIB}.h"
        )

        set_backend_definitions(${EXEC_NAME})

        target_link_libraries(
                "${EXEC_NAME}"
                PRIVATE
                "${PROJECT_NAME}"
                "$<$<STREQUAL:${ARCHIVE_LIB},libarchive>:${LibArchive_LIBRARIES}>"
                # "$<$<STREQUAL:${ARCHIVE_LIB},miniz>:miniz>"
        )

        test_wrapper()
    endforeach (ARCHIVE_LIB ${_archives})
endif ()

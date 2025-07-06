#get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(LIBRARY_NAME "${PROJECT_NAME}")

include("${CMAKE_SOURCE_DIR}/cmake/FindCurlCustom.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/lib${PROJECT_NAME}_helpers.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/target_link_libraries_once.cmake")

include(GNUInstallDirs)
set(installable_libs "")

##############################################################
# CMake options that can be overridden at configuration time #
##############################################################

option(LIBACQUIRE_USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" OFF)

option(LIBACQUIRE_HEADER_ONLY "Make the build header-only" OFF)

if (LIBACQUIRE_HEADER_ONLY)
    set(lib_type "INTERFACE")
else ()
    set(lib_type "PUBLIC")
endif ()

set(_Header_Files "")

# Set libacquire dependencies

include("${CMAKE_SOURCE_DIR}/cmake/lib${PROJECT_NAME}_helpers.cmake")

set_checksum_lib()

set_crypto_lib()

set_ssl_lib()

set_http_https_lib()

set_extract_lib()

include("${CMAKE_SOURCE_DIR}/cmake/FindLibAcquire.cmake")

foreach (_library "HTTPS" "CRYPTO" "EXTRACT" "CHECKSUM")
    message(STATUS "[libacquire] ${_library}_LIB:\t\t${${_library}_LIB}")
endforeach (_library "HTTPS" "CRYPTO" "EXTRACT" "CHECKSUM")

set_checksum_lib()

list(REMOVE_DUPLICATES LibRHash_LIBRARIES)
list(REMOVE_DUPLICATES CHECKSUM_LIBRARIES)
list(REMOVE_DUPLICATES LIBACQUIRE_LIBRARIES)

################################
# Configure file (interpolate) #
################################

configure_file(acquire_config.h.in "${PROJECT_BINARY_DIR}/src/acquire_config.h" @ONLY)

#############################
# Test and target formation #
#############################

set(Header_Files "acquire_download.h")
if (NOT HAS_STDBOOL AND NOT MSVC)
    list(APPEND Header_Files "acquire_stdbool.h")
endif (NOT HAS_STDBOOL AND NOT MSVC)
source_group("Header Files" FILES "${Header_Files}")

option(DEBUG_TOKENISER "OFF")

include("${CMAKE_SOURCE_DIR}/cmake/generate_header_and_source_from_header_only.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/rewrite_includes_to_gen.cmake")

if (LIBACQUIRE_HEADER_ONLY)
    message(STATUS "LIBACQUIRE_HEADER_ONLY")
    add_library("${LIBRARY_NAME}" INTERFACE "${_Header_Files}" "${Header_Files}")
    target_compile_definitions("${LIBRARY_NAME}" PUBLIC "_${TARGET_ARCH}_")
    set(lib_vis "INTERFACE")
else ()
    set(gen_header_files "")
    set(gen_source_files "")

    set(header_impls
            "acquire_checksums.h"
            "acquire_common_defs.h"
            "acquire_download.h"
            "acquire_extract.h"
            "acquire_fileutils.h"
            "acquire_handle.h"
            "acquire_net_common.h"
            "acquire_status_codes.h"
            "acquire_string_extras.h"
            "acquire_url_utils.h"
    )

    message(STATUS "CRYPTO_LIB was ${CRYPTO_LIB}")

    if (BSD)
        set(HAS_LIBBSD 1)
    else ()
        find_package(LibBSD)
        if (LibBSD_FOUND)
            set(HAS_LIBBSD 1)
        endif (LibBSD_FOUND)
    endif (BSD)

    set(CHECKSUM_SRC_CRC32c "")
    # TODO: Decide more granularly which implementation to take for SHA256, &etc.
    # E.g., on Windows you might want librhash if `LIBACQUIRE_USE_LIBRHASH` but on non-Windows OpenSSL.


    ################
    # Cryptography #
    ################
    if (CRYPTO_LIB STREQUAL "CommonCrypto" OR
            CRYPTO_LIB STREQUAL "OpenSSL" OR
            CRYPTO_LIB STREQUAL "LibreSSL")
        set(CHECKSUM_SRC "acquire_openssl.h")
        set(CHECKSUM_SRC_CRC32c "acquire_crc32c.h")
        list(APPEND header_impls "${CHECKSUM_SRC_CRC32c}")
    elseif (LIBACQUIRE_USE_LIBRHASH)
        set(CHECKSUM_SRC "acquire_librhash.h")
    elseif (CRYPTO_LIB STREQUAL "WinCrypt")
        set(LIBACQUIRE_USE_WINCRYPT 1)
        set(CHECKSUM_SRC "acquire_wincrypt.h")
        set(LIBACQUIRE_USE_CRC32C 1)
        set(CHECKSUM_SRC_EXTRA "acquire_crc32c.h")
        list(APPEND header_impls "${CHECKSUM_SRC_EXTRA}")
    elseif (LIBACQUIRE_USE_CRC32C)
        set(CHECKSUM_SRC "acquire_crc32c.h")
    else ()
        # set(CRYPTO_LIB "acquire_crc32c.h")
        message(FATAL_ERROR "TODO")
    endif ()
    list(APPEND header_impls "${CHECKSUM_SRC}")

    ##############
    # Extraction #
    ##############
    if (LIBACQUIRE_USE_LIBARCHIVE)
        set(ARCHIVE_LIB "acquire_libarchive.h")
        set(ARCHIVE_LIB_LINK "${LibArchive_LIBRARIES}")
    elseif (LIBACQUIRE_USE_MINIZ)
        set(ARCHIVE_LIB "acquire_miniz.h")
        set(ARCHIVE_LIB_LINK kubazip::kubazip)
    elseif (LIBACQUIRE_USE_WINCOMPRESSAPI)
        set(ARCHIVE_LIB "acquire_wincompressapi.h")
        # https://learn.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-createcompressor
        set(ARCHIVE_LIB_LINK Cabinet)
    elseif (NOT DEFINED ARCHIVE_LIB OR ARCHIVE_LIB STREQUAL "")
        message(FATAL_ERROR "ARCHIVE_LIB must be set")
    endif ()
    list(APPEND header_impls "${ARCHIVE_LIB}")

    ##############
    # Networking #
    ##############
    if (LIBACQUIRE_USE_LIBFETCH)
        set(NETWORK_LIB "acquire_libfetch.h")
        set(NETWORK_LIB_LINK "freebsd_libfetch")
    elseif (LIBACQUIRE_USE_WININET)
        set(NETWORK_LIB "acquire_wininet.h")
        set(NETWORK_LIB_LINK "wininet")
    elseif (LIBACQUIRE_USE_LIBCURL)
        set(NETWORK_LIB "acquire_libcurl.h")
        set(NETWORK_LIB_LINK CURL::libcurl)
    elseif (LIBACQUIRE_USE_OPENBSD_FTP)
        set(NETWORK_LIB "acquire_openbsd_ftp.h")
        set(NETWORK_LIB_LINK "openbsd_ftp")
    elseif (NOT DEFINED NETWORK_LIB OR NETWORK_LIB STREQUAL "")
        message(FATAL_ERROR "NETWORK_LIB must be set")
    endif ()
    list(APPEND header_impls "${NETWORK_LIB}")

    #############
    # Checksums #
    #############
    if (LIBACQUIRE_USE_LIBRHASH)
        set(CHECKSUM_LIB "acquire_librhash.h")
    elseif (LIBACQUIRE_USE_CRC32C)
        set(CHECKSUM_LIB "acquire_crc32c.h")
    endif ()
    list(APPEND header_impls "${CHECKSUM_LIB}")

    message(STATUS "header_impls = ${header_impls}")

    if (WIN32)
        list(APPEND header_impls "acquire_windows.h")
    endif (WIN32)

    foreach (header_file IN LISTS header_impls)
        if (header_file STREQUAL "")
            continue()
        endif (header_file STREQUAL "")
        message(STATUS "Header: ${header_file}")
        get_filename_component(name_no_ext "${header_file}" NAME_WE)
        set(gen_header_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.h")
        set(gen_source_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.c")

        set(associated_header "")
        if (header_file STREQUAL NETWORK_LIB)
            set(associated_header "gen_acquire_download.h")
        elseif (header_file STREQUAL CHECKSUM_SRC)
            # set(associated_header "gen_acquire_checksums.h")
        elseif (header_file STREQUAL CHECKSUM_SRC_CRC32c)
            #
        elseif (header_file STREQUAL CHECKSUM_LIB)
            set(associated_header "gen_acquire_checksums.h")
        elseif (header_file STREQUAL ARCHIVE_LIB)
            set(associated_header "gen_acquire_extract.h")
        endif ()

        generate_header_and_source_from_header_only(
                "LIBACQUIRE_IMPLEMENTATION"
                "${associated_header}"
                "${CMAKE_CURRENT_SOURCE_DIR}/${header_file}"
                "${gen_header_file}"
                "${gen_source_file}"
        )

        if (EXISTS "${gen_header_file}")
            list(APPEND gen_header_files "${gen_header_file}")
            string(REGEX REPLACE "^${CMAKE_BINARY_DIR}." "" gen_header_file_rel "${gen_header_file}")
            message(STATUS "Generated header: ${gen_header_file_rel}")
        else ()
            message(WARNING "Missing generated header: ${gen_header_file}")
        endif ()

        if (EXISTS "${gen_source_file}")
            list(APPEND gen_source_files "${gen_source_file}")
            string(REGEX REPLACE "^${CMAKE_BINARY_DIR}." "" gen_source_file_rel "${gen_source_file}")
            message(STATUS "Generated source: ${gen_source_file_rel}")
        else ()
            string(REGEX REPLACE "^${CMAKE_BINARY_DIR}." "" gen_source_file_rel "${gen_source_file}")
            message(WARNING "Missing generated source: ${gen_source_file_rel}")
        endif ()
    endforeach (header_file IN LISTS header_impls)

    file(GLOB gen_headers_list
            LIST_DIRECTORIES false
            RELATIVE "${CMAKE_BINARY_DIR}/gen"
            "${CMAKE_BINARY_DIR}/gen/gen_*.h"
    )
    set(files_to_process "")
    list(APPEND files_to_process "${gen_source_files}")
    list(APPEND files_to_process "${gen_header_files}")

    message(STATUS "gen_headers_list = ${gen_headers_list}")
    message(STATUS "gen_source_files = ${gen_source_files}")
    rewrite_includes_to_gen(
            SOURCE_FILES "${files_to_process}"
            GEN_HEADERS "${gen_headers_list}"
    )

    # Set compile definitions per source file to avoid multiple definitions
    set(impls "")
    foreach (src IN LISTS gen_source_files)
        #############
        # Checksums #
        #############
        if (src MATCHES "/gen_acquire_checksums.c$")
            if (NOT ("LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL=1"
                )
                list(APPEND impls "LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL")
            endif (NOT ("LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" IN_LIST impls))
        elseif (src MATCHES "/gen_acquire_crc32c.c$")
            if (NOT ("LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL=1;LIBACQUIRE_USE_CRC32C=1;LIBACQUIRE_ACQUIRE_CRC32C_IMPL=1"
                )
                list(APPEND impls "LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" "LIBACQUIRE_USE_CRC32C" "LIBACQUIRE_ACQUIRE_CRC32C_IMPL")
            endif (NOT ("LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" IN_LIST impls))
        elseif (src MATCHES "/gen_acquire_librhash.c$")
            if (NOT ("LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL=1;LIBACQUIRE_USE_LIBRHASH=1"
                )
                list(APPEND impls "LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" "LIBACQUIRE_USE_LIBRHASH")
            endif (NOT ("LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL" IN_LIST impls))
            #############
            # Fileutils #
            #############
        elseif (src MATCHES "/gen_acquire_fileutils.c$")
            if (NOT ("LIBACQUIRE_ACQUIRE_FILEUTILS_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_ACQUIRE_FILEUTILS_IMPL=1"
                )
                list(APPEND impls "LIBACQUIRE_ACQUIRE_FILEUTILS_IMPL")
            endif (NOT ("LIBACQUIRE_ACQUIRE_FILEUTILS_IMPL" IN_LIST impls))

            ##########
            # Handle #
            ##########
        elseif (src MATCHES "/gen_acquire_handle.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_HANDLE_IMPL=1"
            )

            #################
            # String extras #
            #################
        elseif (src MATCHES "/gen_acquire_string_extras.c$")
            if (HAS_LIBBSD)
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;STRCASESTR_IMPL=1;STRERRORLEN_IMPL=1"
                )
            else ()
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;STRCASESTR_IMPL=1;STRERRORLEN_IMPL=1;STRNSTR_IMPL=1"
                )
            endif ()
        elseif (src MATCHES "/gen_acquire_url_utils.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1"
            )
            ##################
            # Network common #
            ##################
        elseif (src MATCHES "/gen_acquire_net_common.c$")
            set(gen_acquire_net_common "${src}")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL=1"
            )
            ##############
            # Networking #
            ##############
        elseif (src MATCHES "/gen_acquire_libcurl.c$")
            if (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_DOWNLOAD_DIR_IMPL=1;LIBACQUIRE_DOWNLOAD_IMPL=1;LIBACQUIRE_USE_LIBCURL=1"
                )
                list(APPEND impls "LIBACQUIRE_DOWNLOAD_DIR_IMPL" "LIBACQUIRE_DOWNLOAD_IMPL" "LIBACQUIRE_USE_LIBCURL")
            endif (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
        elseif (src MATCHES "/gen_acquire_libfetch.c$")
            if (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_DOWNLOAD_DIR_IMPL=1;LIBACQUIRE_DOWNLOAD_IMPL=1;LIBACQUIRE_USE_LIBFETCH=1"
                )
                list(APPEND impls "LIBACQUIRE_DOWNLOAD_DIR_IMPL" "LIBACQUIRE_DOWNLOAD_IMPL" "LIBACQUIRE_USE_LIBFETCH")
            endif (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
        elseif (src MATCHES "/gen_acquire_wininet.c$")
            if (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_DOWNLOAD_DIR_IMPL=1;LIBACQUIRE_DOWNLOAD_IMPL=1;LIBACQUIRE_USE_WININET=1"
                )
                list(APPEND impls "LIBACQUIRE_DOWNLOAD_DIR_IMPL" "LIBACQUIRE_DOWNLOAD_IMPL" "LIBACQUIRE_USE_WININET")
            endif (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
        elseif (src MATCHES "/gen_acquire_openbsd_ftp.c$")
            if (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
                set_source_files_properties(
                        ${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_DOWNLOAD_DIR_IMPL=1;LIBACQUIRE_DOWNLOAD_IMPL=1;LIBACQUIRE_USE_OPENBSD_FTP=1"
                )
                list(APPEND impls "LIBACQUIRE_DOWNLOAD_DIR_IMPL" "LIBACQUIRE_DOWNLOAD_IMPL" "LIBACQUIRE_USE_OPENBSD_FTP")
            endif (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
            ###########
            # Extract #
            ###########
        elseif (src MATCHES "/gen_acquire_extract.c$")
            # For extract, could detect MINIZ or LIBARCHIVE.
            # NOTE: only one .c file is generated so not `IN_LIST impls` required
            if (LIBACQUIRE_USE_MINIZ)
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_EXTRACT_IMPL=1;LIBACQUIRE_USE_MINIZ=1")
            elseif (LIBACQUIRE_USE_LIBARCHIVE)
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_EXTRACT_IMPL=1;LIBACQUIRE_USE_LIBARCHIVE=1")
            elseif (LIBACQUIRE_USE_WINCOMPRESSAPI)
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_EXTRACT_IMPL=1;LIBACQUIRE_USE_WINCOMPRESSAPI=1")
            else ()
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_EXTRACT_IMPL=1")
            endif ()
        elseif (src MATCHES "/gen_acquire_crc32c.c$")
            if (LIBACQUIRE_USE_WINCOMPRESSAPI)
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_EXTRACT_IMPL=1;LIBACQUIRE_USE_CRC32C=1;LIBACQUIRE_ACQUIRE_CRC32C_IMPL=1")
            endif (LIBACQUIRE_USE_WINCOMPRESSAPI)

            ############
            # Fallback #
            ############
        else ()
            message(STATUS "Nothing special with ${src}")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "")
        endif ()
        if (WIN32)
            set_source_files_properties(
                    "${src}"
                    PROPERTIES COMPILE_DEFINITIONS "_${TARGET_ARCH}_"
            )
        endif (WIN32)
    endforeach (src IN LISTS gen_source_files)

    if (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))
        set_source_files_properties(
                "${gen_acquire_net_common}" PROPERTIES
                COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1;LIBACQUIRE_ACQUIRE_NET_COMMON_IMPL=1;LIBACQUIRE_DOWNLOAD_DIR_IMPL=1"
        )
    endif (NOT ("LIBACQUIRE_DOWNLOAD_DIR_IMPL" IN_LIST impls))

    if (WIN32)
        set(gen_source_file "${CMAKE_BINARY_DIR}/gen/gen_acquire_main.c")
        file(WRITE "${gen_source_file}" "#include <minwindef.h>\n\n"
                "#include \"libacquire_export.h\"\n"
                "LIBACQUIRE_EXPORT void libacquire_dummy_export(void) {}\n"
                "BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {\n"
                "    UNREFERENCED_PARAMETER(hinstDLL);\n"
                "    UNREFERENCED_PARAMETER(lpvReserved);\n"
                "    switch (fdwReason) {\n"
                "    case DLL_PROCESS_ATTACH:\n"
                "    case DLL_THREAD_ATTACH:\n"
                "    case DLL_THREAD_DETACH:\n"
                "    case DLL_PROCESS_DETACH:\n"
                "        break;\n"
                "    }\n"
                "    return TRUE;\n"
                "}\n")
        set_source_files_properties(${gen_source_file} PROPERTIES
                COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION=1")
        list(APPEND gen_source_files "${gen_source_file}")
    endif ()

    # Implement the internal implementation library as STATIC
    message(STATUS "gen_source_files = ${gen_source_files}")
    # add_library("${LIBRARY_NAME}" STATIC ${gen_header_files} ${gen_source_files})
    add_library(
            "${LIBRARY_NAME}"
            SHARED
            "${_Header_Files}"
            "${Header_Files}"
            "${gen_header_files}"
            "${gen_source_files}"
    )
    target_compile_definitions("${LIBRARY_NAME}" PUBLIC "LIBACQUIRE_IMPLEMENTATION=1")
    if (WIN32)
        target_compile_definitions("${LIBRARY_NAME}" PUBLIC "_${TARGET_ARCH}_")
        #set_target_properties("${LIBRARY_NAME}" PROPERTIES
        #        OUTPUT_NAME "${LIBRARY_NAME}"
        #        PREFIX ""
        #)
        #file(COPY_FILE "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${LIBRARY_NAME}_impl.lib"
        #        "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${LIBRARY_NAME}.lib"
        #)
    endif (WIN32)

    ######################
    # Checksum libraries #
    ######################
    if (DEFINED CHECKSUM_LIBRARIES)
        message(STATUS "[libacquire] Using var ${CHECKSUM_LIBRARIES} for checksums")
        target_link_libraries("${LIBRARY_NAME}" PUBLIC "${CHECKSUM_LIBRARIES}")
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE "${CHECKSUM_LIBRARIES_USE}=1")
    elseif (LIBACQUIRE_USE_LIBRHASH)
        message(STATUS "[libacquire] Using LibRHash for checksums")

        find_package(LibRHash REQUIRED)
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE LIBACQUIRE_USE_LIBRHASH=1)

        get_target_property(linked_libs "${LIBRARY_NAME}" LINK_LIBRARIES)
        list(FIND linked_libs "${LibRHash_LIBRARIES}" lib_index)
        if (lib_index EQUAL -1)
            target_include_directories("${LIBRARY_NAME}" PUBLIC "${LibRHash_INCLUDE_DIRS}")
            list(REMOVE_DUPLICATES LibRHash_LIBRARIES)
            target_link_libraries("${LIBRARY_NAME}" PUBLIC "${LibRHash_LIBRARIES}")
        else ()
            message(STATUS "${LIBRARY_NAME} already linked with '${_lib}'")
        endif (lib_index EQUAL -1)
    elseif (LIBACQUIRE_USE_CRC32C)
        message(STATUS "[libacquire] Using internal CRC32C implementation")
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE LIBACQUIRE_USE_CRC32C=1)
    else ()
        message(FATAL_ERROR "No checksum implementation selected: define `LIBACQUIRE_USE_LIBRHASH` or `LIBACQUIRE_USE_CRC32C`")
    endif ()

    ##############
    # Extraction #
    ##############
    if (LIBACQUIRE_USE_MINIZ)
        find_package(kubazip CONFIG REQUIRED)
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE LIBACQUIRE_USE_MINIZ=1)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE kubazip::kubazip)
    elseif (LIBACQUIRE_USE_LIBARCHIVE)
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE LIBACQUIRE_USE_LIBARCHIVE=1)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE "${LibArchive_LIBRARIES}")
    elseif (LIBACQUIRE_USE_WINCOMPRESSAPI)
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE LIBACQUIRE_USE_WINCOMPRESSAPI=1)
        # "Compress.lib"
        target_link_libraries("${LIBRARY_NAME}" PRIVATE "Kernel32.lib" "Cabinet")
    else ()
        message(FATAL_ERROR "No unarchive implementation selected: define `LIBACQUIRE_USE_LIBARCHIVE` or `LIBACQUIRE_USE_MINIZ`")
    endif ()

    ##############
    # Networking #
    ##############
    target_link_libraries("${LIBRARY_NAME}" PRIVATE "${NETWORK_LIB_LINK}")
    # NETWORK_LIB "acquire_libfetch.h"

    if (NOT BSD)
        if (LibBSD_FOUND)
            target_link_libraries("${LIBRARY_NAME}" PUBLIC LibBSD::LibBSD)
            target_compile_definitions(
                    "${LIBRARY_NAME}" PUBLIC HAVE_LIBBSD=1 HAVE_STRNSTR=1
            )
            message(STATUS "libbsd found")
        else ()
            message(STATUS "No libbsd found")
        endif (LibBSD_FOUND)
    endif (NOT BSD)

    target_include_directories(
            "${LIBRARY_NAME}"
            PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/acquire>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gen>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/acquire>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    set_target_properties(
            "${LIBRARY_NAME}"
            PROPERTIES
            LINKER_LANGUAGE C
    )

    install(FILES ${gen_header_files} TYPE INCLUDE)

    set(lib_vis "PUBLIC")

    list(REMOVE_ITEM LIBACQUIRE_LIBRARIES "${CHECKSUM_LIBRARIES}" "${EXTRACT_LIB}" "lib${EXTRACT_LIB}")
    foreach (_lib ${LIBACQUIRE_LIBRARIES})
        target_link_libraries_once("${LIBRARY_NAME}" "${lib_vis}" "${_lib}")
    endforeach (_lib ${LIBACQUIRE_LIBRARIES})
endif ()

include(GNUInstallDirs)
if (LIBACQUIRE_HEADER_ONLY)
    target_include_directories(
            "${LIBRARY_NAME}"
            "${lib_vis}"
            "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/acquire>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
endif (LIBACQUIRE_HEADER_ONLY)

if (DEFINED LIBACQUIRE_INCLUDE_DIR)
    target_include_directories(
            "${LIBRARY_NAME}"
            "${lib_vis}"
            "$<BUILD_INTERFACE:${LIBACQUIRE_INCLUDE_DIR}>"
    )
endif (DEFINED LIBACQUIRE_INCLUDE_DIR)

foreach (_lib ${LIBACQUIRE_LIBRARIES})
    target_link_libraries_once("${LIBRARY_NAME}" "${lib_vis}" "${_lib}")
endforeach (_lib ${LIBACQUIRE_LIBRARIES})
target_link_libraries(
        "${LIBRARY_NAME}"
        "${lib_vis}"
        "${PROJECT_NAME}_compiler_flags"
)

target_compile_definitions("${LIBRARY_NAME}" "${lib_vis}" "_${TARGET_ARCH}_")

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/lib${LIBRARY_NAME}_export.h")

if (1)
    # pass
elseif (LIBACQUIRE_HEADER_ONLY)
    file(WRITE "${_export_file}"
            "#ifndef LIBACQUIRE_EXPORT_H\n"
            "#define LIBACQUIRE_EXPORT_H\n\n"
            "#  ifndef LIBACQUIRE_EXPORT\n"
            "#    define LIBACQUIRE_EXPORT\n"
            "#  endif /* !LIBACQUIRE_EXPORT */\n\n"
            "#endif /* !LIBACQUIRE_EXPORT_H */\n")
else ()
    generate_export_header("${LIBRARY_NAME}"
            PREFIX_NAME "LIB"
            EXPORT_FILE_NAME "${_export_file}")
endif ()

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)
# Set OUTPUT_NAME only for the public library to get "libacquire.dylib"
if (NOT LIBACQUIRE_HEADER_ONLY AND NOT WIN32)
    set_target_properties(
            "${LIBRARY_NAME}"
            PROPERTIES
            OUTPUT_NAME "${PROJECT_NAME}"
    )
endif (NOT LIBACQUIRE_HEADER_ONLY AND NOT WIN32)

option(BUILD_OPENBSD_FTP "Build `ftp` binary from OpenBSD" OFF)
if (BUILD_OPENBSD_FTP)
    add_subdirectory("openbsd_ftp")
endif (BUILD_OPENBSD_FTP)

option(BUILD_FREEBSD_LIBFETCH "Build `libfetch` library from FreeBSD" OFF)
if (BUILD_FREEBSD_LIBFETCH OR LIBACQUIRE_USE_MY_LIBFETCH)
    add_subdirectory("freebsd_libfetch")
endif (BUILD_FREEBSD_LIBFETCH OR LIBACQUIRE_USE_MY_LIBFETCH)

#######################################
# Export / installation configuration #
#######################################

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "${${PROJECT_NAME}_VERSION}")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}")

# install rules
list(APPEND installable_libs "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif (TARGET "${DEPENDANT_LIBRARY}")
install(TARGETS ${installable_libs}
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES "${PROJECT_BINARY_DIR}/src/acquire_config.h"
        ${Header_Files} ${_Header_Files} # ${_export_file}
        TYPE "INCLUDE")

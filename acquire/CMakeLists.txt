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

option(USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" OFF)

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

if (LIBACQUIRE_HEADER_ONLY)
    message(STATUS "LIBACQUIRE_HEADER_ONLY")
    add_library("${LIBRARY_NAME}" INTERFACE "${_Header_Files}" "${Header_Files}")
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
            "acquire_net_common.h"
            "acquire_string_extras.h"
            "acquire_url_utils.h"
    )

    if (USE_LIBCURL)
        set(NETWORK_LIB "acquire_libcurl.h")
    elseif (USE_LIBFETCH)
        set(NETWORK_LIB "acquire_libfetch.h")
    elseif (USE_WININET)
        set(NETWORK_LIB "acquire_wininet.h")
    elseif (USE_OPENBSD_FTP)
        set(NETWORK_LIB "acquire_openbsd_ftp.h")
    endif ()
    list(APPEND header_impls "${NETWORK_LIB}")

    message(STATUS "CRYPTO_LIB was ${CRYPTO_LIB}")
    set(CHECKSUM_SRC_CRC32c "")
    # TODO: Decide more granularly which implementation to take for SHA256, &etc.
    # E.g., on Windows you might want librhash if `USE_LIBRHASH` but on non-Windows OpenSSL.
    if (CRYPTO_LIB STREQUAL "CommonCrypto" OR
            CRYPTO_LIB STREQUAL "OpenSSL" OR
            CRYPTO_LIB STREQUAL "LibreSSL"
    )
        set(CHECKSUM_SRC "acquire_openssl.h")
        set(CHECKSUM_SRC_CRC32c "acquire_crc32c.h")
        list(APPEND header_impls "${CHECKSUM_SRC_CRC32c}")
    elseif (USE_LIBRHASH)
        set(CHECKSUM_SRC "acquire_librhash.h")
    elseif (CRYPTO_LIB STREQUAL "WinCrypt")
        set(CHECKSUM_SRC "acquire_wincrypt.h")
    elseif (USE_CRC32C)
        set(CHECKSUM_SRC "acquire_crc32c.h")
    else ()
        # set(CRYPTO_LIB "acquire_crc32c.h")
        message(FATAL_ERROR "TODO")
    endif ()
    list(APPEND header_impls "${CHECKSUM_SRC}")

    foreach (header_file IN LISTS header_impls)
        get_filename_component(name_no_ext "${header_file}" NAME_WE)
        set(gen_header_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.h")
        set(gen_source_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.c")

        set(associated_header "")
        if (header_file STREQUAL NETWORK_LIB)
            set(associated_header "gen_acquire_download.h")
        elseif (header_file STREQUAL CHECKSUM_SRC)
            # set(associated_header "gen_acquire_checksums.h")
        else (header_file STREQUAL CHECKSUM_SRC_CRC32c)
            #
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

    # Set compile definitions per source file to avoid multiple definitions
    foreach (src IN LISTS gen_source_files)
        if (src MATCHES "/gen_acquire_checksums.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;LIBACQUIRE_ACQUIRE_CHECKSUMS_IMPL=1"
            )
        elseif (src MATCHES "/gen_acquire_crc32c.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_CRC32C=1"
            )
        elseif (src MATCHES "/gen_acquire_fileutils.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;LIBACQUIRE_IMPL_ACQUIRE_FILEUTILS=1"
            )
        elseif (src MATCHES "/gen_acquire_libcurl.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;DOWNLOAD_DIR_IMPL;DOWNLOAD_IMPL;USE_LIBCURL=1"
            )
        elseif (src MATCHES "/gen_acquire_libfetch.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;DOWNLOAD_DIR_IMPL;DOWNLOAD_IMPL;USE_LIBFETCH=1"
            )
        elseif (src MATCHES "/gen_acquire_librhash.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_LIBRHASH=1"
            )
        elseif (src MATCHES "/gen_acquire_libwininet.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;DOWNLOAD_DIR_IMPL;DOWNLOAD_IMPL;USE_WININET=1"
            )
        elseif (src MATCHES "/gen_acquire_net_common.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION"
            )
        elseif (src MATCHES "/gen_acquire_openbsd_ftp.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;DOWNLOAD_DIR_IMPL;DOWNLOAD_IMPL;USE_OPENBSD_FTP=1"
            )
        elseif (src MATCHES "/gen_acquire_openssl.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;STRCASESTR_IMPL;STRERRORLEN_IMPL;LIBACQUIRE_CRYPTO_IMPL"
            )
        elseif (src MATCHES "/gen_acquire_string_extras.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;STRCASESTR_IMPL;STRERRORLEN_IMPL"
            )
        elseif (src MATCHES "/gen_acquire_url_utils.c$")
            set_source_files_properties(
                    ${src} PROPERTIES
                    COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;LIBACQUIRE_IMPL_ACQUIRE_FILEUTILS=1"
            )
        elseif (src MATCHES "/gen_acquire_extract.c$")
            # For extract, could detect MINIZ or LIBARCHIVE
            if (USE_MINIZ)
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_MINIZ=1")
            elseif (USE_LIBARCHIVE)
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_LIBARCHIVE=1")
            else ()
                set_source_files_properties(${src} PROPERTIES
                        COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION")
            endif ()
        else ()
            message(STATUS "Nothing special with ${src}")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "")
        endif ()
    endforeach (src IN LISTS gen_source_files)

    # Implement the internal implementation library as STATIC
    add_library("${LIBRARY_NAME}_impl" STATIC ${gen_header_files} ${gen_source_files})

    if (DEFINED CHECKSUM_LIBRARIES)
        message(STATUS "[libacquire] Using var ${CHECKSUM_LIBRARIES} for checksums")
        target_link_libraries("${LIBRARY_NAME}_impl" PUBLIC "${CHECKSUM_LIBRARIES}")
        target_compile_definitions("${LIBRARY_NAME}_impl" PRIVATE "${CHECKSUM_LIBRARIES_USE}=1")
    elseif (USE_LIBRHASH)
        message(STATUS "[libacquire] Using LibRHash for checksums")

        find_package(LibRHash REQUIRED)
        target_compile_definitions("${LIBRARY_NAME}_impl" PRIVATE USE_LIBRHASH=1)

        get_target_property(linked_libs "${LIBRARY_NAME}_impl" LINK_LIBRARIES)
        message(STATUS "linked_libs = ${linked_libs}")
        message(STATUS "CHECKSUM_LIBRARIES at here is ${CHECKSUM_LIBRARIES}")
        list(FIND linked_libs "${LibRHash_LIBRARIES}" lib_index)
        if (lib_index EQUAL -1)
            target_include_directories("${LIBRARY_NAME}_impl" PUBLIC "${LibRHash_INCLUDE_DIRS}")
            list(REMOVE_DUPLICATES LibRHash_LIBRARIES)
            target_link_libraries("${LIBRARY_NAME}_impl" PUBLIC "${LibRHash_LIBRARIES}")
        else ()
            message(STATUS "${LIBRARY_NAME}_impl already linked with '${_lib}'")
        endif (lib_index EQUAL -1)
    elseif (USE_CRC32C)
        message(STATUS "[libacquire] Using internal CRC32C implementation")
        target_compile_definitions("${LIBRARY_NAME}_impl" PRIVATE USE_CRC32C=1)
    else ()
        message(FATAL_ERROR "No checksum implementation selected: define `USE_LIBRHASH` or `USE_CRC32C`")
    endif ()

    if (NOT BSD)
        find_package(LibBSD)
        if (LibBSD_FOUND)
            target_link_libraries("${LIBRARY_NAME}_impl" PUBLIC LibBSD::LibBSD)
            target_compile_definitions(
                    "${LIBRARY_NAME}_impl" PUBLIC HAVE_STRNSTR=1 HAVE_LIBBSD=1
            )
        endif (LibBSD_FOUND)
    endif (NOT BSD)

    target_include_directories(
            "${LIBRARY_NAME}_impl"
            PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gen>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/acquire>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/acquire>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    set_target_properties(
            "${LIBRARY_NAME}_impl"
            PROPERTIES
            LINKER_LANGUAGE C
    )

    install(FILES ${gen_header_files} TYPE INCLUDE)

    add_library("${LIBRARY_NAME}" SHARED "${_Header_Files}" "${Header_Files}")

    target_link_libraries("${LIBRARY_NAME}" PUBLIC "${LIBRARY_NAME}_impl")
    set(lib_vis "PUBLIC")

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

if (LIBACQUIRE_HEADER_ONLY)
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

# Set OUTPUT_NAME only for the public library to get "libacquire.dylib"
if (NOT LIBACQUIRE_HEADER_ONLY)
    set_target_properties(
            "${LIBRARY_NAME}"
            PROPERTIES
            LINKER_LANGUAGE
            C
            OUTPUT_NAME "${PROJECT_NAME}"
    )
endif (NOT LIBACQUIRE_HEADER_ONLY)

option(BUILD_OPENBSD_FTP "Build `ftp` binary from OpenBSD" OFF)
if (BUILD_OPENBSD_FTP)
    add_subdirectory("openbsd_ftp")
endif (BUILD_OPENBSD_FTP)

option(BUILD_FREEBSD_LIBFETCH "Build `libfetch` library from FreeBSD" OFF)
if (BUILD_FREEBSD_LIBFETCH OR USE_MY_LIBFETCH)
    add_subdirectory("freebsd_libfetch")
endif (BUILD_FREEBSD_LIBFETCH OR USE_MY_LIBFETCH)

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
        ${Header_Files} ${_Header_Files} ${_export_file}
        TYPE "INCLUDE")

#[=======================================================================[

generate_amalgamation_header
----------------------------

Function to generate the amalgamated header.

]=======================================================================]

function (generate_amalgamation_header)
    set(amalgam
            "acquire/acquire_stdbool.h"
            "acquire/acquire_errors.h"
            "acquire/acquire_url_utils.h"
            "acquire/acquire_string_extras.h"
            "acquire/acquire_fileutils.h"
            "acquire/acquire_checksums.h"
            "acquire/acquire_common_defs.h"

            # Crypto
            "acquire/acquire_openssl.h"
            "acquire/acquire_wincrypt.h"
            # "acquire/acquire_winseccng.h"
            "acquire/acquire_openssl.h"
            "acquire/acquire_crc32c.h"
            "acquire/acquire_librhash.h"

            # Networking
            "acquire/acquire_download.h"

            "acquire/acquire_libcurl.h"
            "acquire/acquire_wininet.h"
            "acquire/acquire_libfetch.h"

            "acquire/acquire_net_common.h"

            # Archiving
            "acquire/acquire_extract.h"
            "acquire/acquire_libarchive.h"
            "acquire/acquire_miniz.h"
            # "acquire/acquire_zlib.h"  # TODO
            )

    set(amalgam_files "")
    set(all_contents "")
    foreach (_amalgam ${amalgam})
        string(APPEND amalgam_files " \"${_amalgam}\"")
        string(APPEND all_contents "\n/* Generated from: ${_amalgam} */\n\n")
        file(READ "${_amalgam}" contents)
        string(APPEND all_contents "${contents}")
    endforeach ()

    set(all_contents_filtered "")
    set(line "")
    set(skip_line 0)
    string(LENGTH "${all_contents}" _len)
    math(EXPR len "${_len} - 1")
    foreach (char_index RANGE ${len})
        string(SUBSTRING "${all_contents}" "${char_index}" "1" char)
        # `#include\n\"foo"` will break… but who does that?!
        if (char STREQUAL "\n")
            string(LENGTH "${line}" len)
            if (len GREATER 10)
                string(SUBSTRING "${line}" "0" "10" _line)
                if (_line STREQUAL [[#include "]])
                    string(APPEND all_contents_filtered "/* ${line} */\n")
                else ()
                    string(APPEND all_contents_filtered "${line}\n")
                endif ()
            else ()
                string(APPEND all_contents_filtered "${line}\n")
            endif ()

            set(line "")
        else ()
            string(APPEND line "${char}")
        endif ()
    endforeach ()

    # I suppose this could be further extended, deduplicating headers and putting them all to the top…

    file(WRITE
            "${CMAKE_CURRENT_BINARY_DIR}/src/acquire.h"
            "/* SPDX-License-Identifier: (Apache-2.0 OR MIT)\n"
            " * https://github.com/offscale/libacquire */\n"
            "#ifndef LIBACQUIRE_H\n"
            "#define LIBACQUIRE_H\n"
            "${all_contents_filtered}\n"
            "#endif /* ! LIBACQUIRE_H */\n"
            )
endfunction (generate_amalgamation_header)

function (create_amalgamation_target amalgamation_header)
    set(LIBRARY_NAME "libacquire")

    if (NOT TARGET "${LIBRARY_NAME}")
        source_group("Header Files" FILES "${amalgamation_header}")
        get_filename_component(dir_of_amalgamation_header "${amalgamation_header}" DIRECTORY)

        add_library("${LIBRARY_NAME}" INTERFACE)
        include(GNUInstallDirs)
        target_include_directories(
                "${LIBRARY_NAME}"
                INTERFACE
                "$<BUILD_INTERFACE:${dir_of_amalgamation_header}>"
                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        )

        if (NOT DEFINED TARGET_ARCH)
            get_arch()
        endif (NOT DEFINED TARGET_ARCH)
        target_compile_definitions(
                "${LIBRARY_NAME}"
                INTERFACE
                "_${TARGET_ARCH}_"
        )

        get_cmake_property(_variableNames VARIABLES)
        foreach (_variableName ${_variableNames})
            string(SUBSTRING "${_variableName}" 0 4 maybe_use)
            if (maybe_use STREQUAL "USE_")
                target_compile_definitions("${LIBRARY_NAME}" INTERFACE "${_variableName}=${${_variableName}}")
            endif (maybe_use STREQUAL "USE_")
        endforeach(_variableName ${_variableNames})

        set_target_properties(
                "${LIBRARY_NAME}"
                PROPERTIES
                LINKER_LANGUAGE
                C
        )

        # setup the version numbering
        set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
        set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

        # install rules
        include(GNUInstallDirs)
        set(installable_libs "${LIBRARY_NAME}")
        if (TARGET "${DEPENDANT_LIBRARY}")
            list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
        endif (TARGET "${DEPENDANT_LIBRARY}")
        install(TARGETS ${installable_libs}
                EXPORT "${LIBRARY_NAME}Targets"
                ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
        install(FILES "${amalgamation_header}"
                TYPE "INCLUDE")
        install(EXPORT "${LIBRARY_NAME}Targets" DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}")
    endif (NOT TARGET "${LIBRARY_NAME}")
endfunction (create_amalgamation_target amalgamation_header)

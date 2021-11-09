#[=======================================================================[

generate_amalgamation_header
----------------------------

Function to generate the amalgamated header.

]=======================================================================]

function (generate_amalgamation_header)
    set(amalgam
            "libacquire/acquire_stdbool.h"
            "libacquire/acquire_errors.h"
            "libacquire/acquire_url_utils.h"
            "libacquire/acquire_string_extras.h"
            "libacquire/acquire_fileutils.h"
            "libacquire/acquire_checksums.h"
            "libacquire/acquire_common_defs.h"

            # Crypto
            "libacquire/acquire_openssl.h"
            "libacquire/acquire_wincrypt.h"
            # "libacquire/acquire_winseccng.h"
            "libacquire/acquire_openssl.h"

            # Networking
            "libacquire/acquire_download.h"

            "libacquire/acquire_libcurl.h"
            "libacquire/acquire_wininet.h"
            "libacquire/acquire_libfetch.h"

            "libacquire/acquire_net_common.h"

            # Archiving
            "libacquire/acquire_extract.h"
            "libacquire/acquire_miniz.h"
            "libacquire/acquire_zlib.h"
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
            "#endif /* LIBACQUIRE_H */\n"
            )
endfunction (generate_amalgamation_header)

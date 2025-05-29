get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

include(GenerateExportHeader)

########################
# Generate error files #
########################

function (parse_emit_errors name in_fname out_fname)
    file(READ "${in_fname}" contents)
    string(TOUPPER "${name}" upper_name)
    set(new_contents "static struct fetcherr ${name}_errlist[] = {")
    set(line "")
    set(skip_line 0)
    string(LENGTH "${contents}" _len)
    math(EXPR len "${_len} - 1")
    foreach (char_index RANGE ${len})
        string(SUBSTRING "${contents}" "${char_index}" "1" char)
        string(LENGTH "${line}" line_len)
        if (char STREQUAL "\n")
            if (NOT skip_line AND line_len)
                string(APPEND new_contents "\"${line}\" },")
            endif (NOT skip_line AND line_len)
            set(skip_line 0)
            set(line "")
        elseif (char STREQUAL "#")
            set(skip_line 1)
        elseif (char STREQUAL "\t")
            if (line_len)
                string(LENGTH "${contents}" _l)
                math(EXPR n "${_l} - 1")
                foreach (char_idx RANGE ${n})
                    string(SUBSTRING "${line}" "${char_idx}" "1" ch)
                    if (ch STREQUAL " ")
                        string(SUBSTRING "${line}" "0" "${char_idx}" fst_word)
                        math(EXPR n_minus_1 "${n} - ${char_idx}")
                        math(EXPR char_idx_plus_1 "${char_idx} + 1")
                        string(SUBSTRING "${line}" "${char_idx_plus_1}" "${n_minus_1}" snd_word)
                        set(snd_word "FETCH_${snd_word},")
                        string(LENGTH "${snd_word}" snd_word_size)
                        math(EXPR snd_word_padding "12 - ${snd_word_size}")
                        foreach (_ RANGE 0 "${snd_word_padding}")
                            string(APPEND snd_word " ")
                        endforeach (_ RANGE 0 "${snd_word_padding}")
                        string(LENGTH "${snd_word}" snd_word_size)
                        string(APPEND new_contents "\n\t{ ${fst_word},\t${snd_word}\t")
                        break ()
                    endif (ch STREQUAL " ")
                endforeach (char_idx RANGE ${n})
                set(line "")
            endif (line_len)
        else ()
            string(APPEND line "${char}")
        endif ()
    endforeach (char_index RANGE ${len})
    string(APPEND new_contents "\n\t{ -1,\tFETCH_UNKNOWN,\t\"Unknown ${upper_name} error\" }")
    string(APPEND new_contents "\n};\n")
    file(WRITE "${out_fname}" "${new_contents}")
    message(STATUS "Generated \"${out_fname}\" from \"${in_fname}\"")
endfunction (parse_emit_errors name in_fname out_fname)

foreach (proto "ftp" "http")
    parse_emit_errors("${proto}" "${proto}.errors" "${proto}err.h")
endforeach (proto "ftp" "http")

############################
# Library target formation #
############################

set(Header_Files
        "common.h"
        "fetch.h"
        "ftperr.h"
        "httperr.h"
        "shim.h"
        )
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "common.c"
        "fetch.c"
        "file.c"
        "ftp.c"
        "http.c"
        "shim.c"
        )
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" SHARED "${Header_Files}" "${Source_Files}")

set(_libs "${PROJECT_NAME}_compiler_flags")
if (DEFINED WITH_SSL)
    find_package(OpenSSL REQUIRED)
    list(APPEND _libs "OpenSSL::SSL")
endif (DEFINED WITH_SSL)

check_symbol_exists(strnstr "string.h" HAS_STRNSTR)

if (HAS_STRNSTR)
    add_compile_definitions(HAS_STRNSTR=1)
else()
    list(APPEND _libs "${PROJECT_NAME}")
endif ()

list(REMOVE_DUPLICATES _libs)
target_link_libraries("${LIBRARY_NAME}" PUBLIC "${_libs}")

include(GNUInstallDirs)
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

#######################################
# Export / installation configuration #
#######################################

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "${${PROJECT_NAME}_VERSION}")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}")

# install rules
set(installable_libs "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif (TARGET "${DEPENDANT_LIBRARY}")

install(TARGETS ${installable_libs}
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES "${_export_file}" ${Header_Files}
        TYPE "INCLUDE")

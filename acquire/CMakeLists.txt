#get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(LIBRARY_NAME "${PROJECT_NAME}")

include("${CMAKE_SOURCE_DIR}/cmake/FindCurlCustom.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/lib${PROJECT_NAME}_helpers.cmake")

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
endif (LIBACQUIRE_HEADER_ONLY)

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
    message(STATUS "[libacquire] ${_library}_LIB:\t${${_library}_LIB}")
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

function(generate_source_file_from_header_only filepath header_filepath source_filepath)
    # --- Initialization ---
    if (NOT EXISTS "${filepath}")
        message(FATAL_ERROR "Input file does not exist: ${filepath}")
        return()
    endif ()

    get_filename_component(header_basename "${header_filepath}" NAME)
    set(header_contents "")
    set(source_contents "")

    # Parsing State
    set(parsing_mode "HEADER") # HEADER or SOURCE (based on LIBACQUIRE_IMPLEMENTATION block)
    set(current_line "")      # Accumulates characters for the current logical line/directive
    set(state "CODE")          # Current lexical state: CODE, BLOCK_COMMENT, LINE_COMMENT, STRING, CHAR_LITERAL
    set(escape_next_char OFF)  # Flag for handling backslash escapes

    # Parenthesis/Brace/Bracket Balancing (within CODE state)
    set(lparen 0)
    set(rparen 0)
    set(lbrace 0) # Not used for line ending detection, but might be useful
    set(rbrace 0) # Not used for line ending detection
    set(lsquare 0)
    set(rsquare 0)

    # Preprocessor Condition Stack
    # Each element is "TYPE;CONDITION_STRING" e.g., "IF;FEATURE_A" or "IFDEF;FOO"
    set(pp_conditions_stack "")
    # Tracks if we are inside the target implementation block
    set(in_implementation_block OFF)
    set(implementation_block_depth 0) # Depth specifically for LIBACQUIRE_IMPLEMENTATION blocks

    # --- Character-by-Character Processing ---
    file(READ "${filepath}" file_content)
    string(LENGTH "${file_content}" file_len)
    math(EXPR last_index "${file_len} - 1")

    set(prev_char "") # Store previous character for multi-char checks like /*, //

    foreach (char_index RANGE ${last_index})
        string(SUBSTRING "${file_content}" ${char_index} 1 char)
        string(APPEND current_line "${char}")

        # --- State Machine ---
        if (state STREQUAL "CODE")
            if (escape_next_char)
                set(escape_next_char OFF)
            elseif (char STREQUAL "\\")
                set(escape_next_char ON)
            elseif (char STREQUAL "/" AND prev_char STREQUAL "/")
                set(state "LINE_COMMENT")
            elseif (char STREQUAL "*" AND prev_char STREQUAL "/")
                set(state "BLOCK_COMMENT")
            elseif (char STREQUAL "\"")
                set(state "STRING")
                set(escape_next_char OFF)
            elseif (char STREQUAL "'")
                set(state "CHAR_LITERAL")
                set(escape_next_char OFF)
            else ()
                # Balance parens/brackets only in code state
                if (char STREQUAL "(")
                    math(EXPR lparen "${lparen} + 1")
                elseif (char STREQUAL ")")
                    math(EXPR rparen "${rparen} + 1")
                elseif (char STREQUAL "[")
                    math(EXPR lsquare "${lsquare} + 1")
                elseif (char STREQUAL "]")
                    math(EXPR rsquare "${rsquare} + 1")
                elseif (char STREQUAL "{")
                    math(EXPR lbrace "${lbrace} + 1")
                elseif (char STREQUAL "}")
                    math(EXPR rbrace "${rbrace} + 1")
                endif ()
            endif ()
        elseif (state STREQUAL "BLOCK_COMMENT")
            if (char STREQUAL "/" AND prev_char STREQUAL "*")
                set(state "CODE")
            endif ()
        elseif (state STREQUAL "LINE_COMMENT")
            if (char STREQUAL "\n")
                set(state "CODE")
            endif ()
        elseif (state STREQUAL "STRING")
            if (escape_next_char)
                set(escape_next_char OFF)
            elseif (char STREQUAL "\\")
                set(escape_next_char ON)
            elseif (char STREQUAL "\"")
                set(state "CODE")
            endif ()
        elseif (state STREQUAL "CHAR_LITERAL")
            if (escape_next_char)
                set(escape_next_char OFF)
            elseif (char STREQUAL "\\")
                set(escape_next_char ON)
            elseif (char STREQUAL "'")
                set(state "CODE")
            endif ()
        endif ()

        set(prev_char "${char}")

        # --- Line Boundary Detection (Highly Fragile) ---
        set(process_line OFF)
        if (char STREQUAL "\n" AND state STREQUAL "CODE")
            if (lparen EQUAL rparen AND lsquare EQUAL rsquare)
                string(STRIP "${current_line}" trimmed_line)
                string(LENGTH "${trimmed_line}" trimmed_len)
                if (trimmed_len GREATER 0)
                    math(EXPR last_trimmed_idx "${trimmed_len} - 1")
                    string(SUBSTRING "${trimmed_line}" ${last_trimmed_idx} 1 last_trimmed_char)
                    string(REGEX MATCH "^[ \t]*#" is_preprocessor "${trimmed_line}")

                    if (last_trimmed_char STREQUAL ";")
                        set(process_line ON)
                    elseif (last_trimmed_char STREQUAL "{")
                        set(process_line ON)
                    elseif (last_trimmed_char STREQUAL "}")
                        set(process_line ON)
                    elseif (is_preprocessor)
                        set(process_line ON)
                    elseif (last_trimmed_char STREQUAL ":")
                        if (NOT trimmed_line MATCHES "[;={}(]")
                            set(process_line ON)
                        endif ()
                    endif ()
                else ()
                    set(process_line ON)
                endif ()
            endif ()
        endif ()

        if (process_line)
            string(STRIP "${current_line}" line_for_match)
            set(append_to_header OFF)
            set(append_to_source OFF)

            if (in_implementation_block)
                set(append_to_source ON)
            else ()
                set(append_to_header ON)
            endif ()

            if (line_for_match MATCHES "^[ \t]*#")
                if (line_for_match MATCHES "^[ \t]*#[ \t]*if(n?def)?")
                    string(REGEX REPLACE "^[ \t]*#[ \t]*if(n?def)?[ \t]+" "" condition_str "${line_for_match}")
                    set(directive_type "IF")
                    if (line_for_match MATCHES "^[ \t]*#[ \t]*ifdef")
                        set(directive_type "IFDEF")
                    endif ()
                    if (line_for_match MATCHES "^[ \t]*#[ \t]*ifndef")
                        set(directive_type "IFNDEF")
                    endif ()

                    list(APPEND pp_conditions_stack "${directive_type};${condition_str}")

                    if (condition_str MATCHES "LIBACQUIRE_IMPLEMENTATION")
                        if (implementation_block_depth EQUAL 0)
                            set(in_implementation_block ON)
                            set(append_to_header OFF)
                            set(append_to_source ON)
                        endif ()
                        math(EXPR implementation_block_depth "${implementation_block_depth} + 1")
                    endif ()

                elseif (line_for_match MATCHES "^[ \t]*#[ \t]*else")
                    # pass through
                elseif (line_for_match MATCHES "^[ \t]*#[ \t]*elif")
                    # pass through
                elseif (line_for_match MATCHES "^[ \t]*#[ \t]*endif")
                    if (pp_conditions_stack)
                        set(is_impl_endif OFF)
                        if (line_for_match MATCHES "LIBACQUIRE_IMPLEMENTATION")
                            if (implementation_block_depth GREATER 0)
                                set(is_impl_endif ON)
                            endif ()
                        endif ()
                        if (is_impl_endif)
                            math(EXPR implementation_block_depth "${implementation_block_depth} - 1")
                            if (implementation_block_depth EQUAL 0)
                                set(in_implementation_block OFF)
                                set(append_to_header OFF)
                                set(append_to_source ON)
                            endif ()
                        endif ()
                        list(POP_BACK pp_conditions_stack)
                    else ()
                        message(WARNING "Unmatched #endif encountered: ${current_line}")
                    endif ()
                endif ()
            endif ()

            if (append_to_source)
                string(APPEND source_contents "${current_line}")
            elseif (append_to_header)
                string(APPEND header_contents "${current_line}")
            endif ()

            set(current_line "")
            set(lparen 0)
            set(rparen 0)
            set(lbrace 0)
            set(rbrace 0)
            set(lsquare 0)
            set(rsquare 0)
        endif ()
    endforeach ()

    if (NOT state STREQUAL "CODE")
        message(WARNING "File ended while in state: ${state}")
    endif ()
    if (NOT lparen EQUAL rparen OR NOT lsquare EQUAL rsquare)
        message(WARNING "File ended with unbalanced parentheses/brackets")
    endif ()
    if (pp_conditions_stack)
        message(WARNING "File ended with unbalanced preprocessor directives. Stack: ${pp_conditions_stack}")
    endif ()
    if (NOT implementation_block_depth EQUAL 0)
        message(WARNING "File ended with unbalanced LIBACQUIRE_IMPLEMENTATION blocks. Depth: ${implementation_block_depth}")
    endif ()

    file(WRITE "${header_filepath}" "${header_contents}")
    message(STATUS "Generated header (via character-by-character): ${header_filepath}")

    if (source_contents OR in_implementation_block)
        set(final_source_content "#include \"${header_basename}\"\n\n${source_contents}")
        file(WRITE "${source_filepath}" "${final_source_content}")
        message(STATUS "Generated source (via character-by-character): ${source_filepath}")
    else ()
        message(STATUS "No implementation content found for source (via character-by-character): ${source_filepath}")
    endif ()

endfunction()

if (LIBACQUIRE_HEADER_ONLY)
    add_library("${LIBRARY_NAME}" INTERFACE "${_Header_Files}" "${Header_Files}")
    set(lib_vis "INTERFACE")
else ()
    set(gen_header_files "")
    set(gen_source_files "")

    set(header_impls
            "acquire_checksums.h"
            "acquire_common_defs.h"
            "acquire_crc32c.h"
            "acquire_download.h"
            "acquire_extract.h"
            "acquire_fileutils.h"
            "acquire_net_common.h"
            "acquire_string_extras.h"
            "acquire_url_utils.h"
            "acquire_librhash.h"
    )

    foreach (header_file IN LISTS header_impls)
        get_filename_component(name_no_ext "${header_file}" NAME_WE)
        set(gen_header_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.h")
        set(gen_source_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.c")

        generate_source_file_from_header_only(
                "${CMAKE_CURRENT_SOURCE_DIR}/${header_file}"
                "${gen_header_file}"
                "${gen_source_file}"
        )

        if (EXISTS "${gen_header_file}")
            list(APPEND gen_header_files "${gen_header_file}")
            message(STATUS "Generated header: ${gen_header_file}")
        else()
            message(WARNING "Missing generated header: ${gen_header_file}")
        endif()

        if (EXISTS "${gen_source_file}")
            list(APPEND gen_source_files "${gen_source_file}")
            message(STATUS "Generated source: ${gen_source_file}")
        else()
            message(WARNING "Missing generated source: ${gen_source_file}")
        endif()
    endforeach()

    # Implement the internal implementation library as STATIC
    add_library("${LIBRARY_NAME}_impl" STATIC ${gen_header_files} ${gen_source_files})

    # Set compile definitions per source file to avoid multiple definitions
    foreach (src IN LISTS gen_source_files)
        if (src MATCHES "/gen_acquire_librhash.c$")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_LIBRHASH=1")
        elseif (src MATCHES "/gen_acquire_crc32c.c$")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_CRC32C=1")
        elseif (src MATCHES "/gen_acquire_fileutils.c$")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION")
        elseif (src MATCHES "/gen_acquire_checksums.c$")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION")
        elseif (src MATCHES "/gen_acquire_download.c$")
            if (USE_LIBCURL)
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_LIBCURL=1")
            elseif (USE_LIBFETCH)
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_LIBFETCH=1")
            elseif (USE_WININET)
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_WININET=1")
            elseif (USE_OPENBSD_FTP)
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_OPENBSD_FTP=1")
            else()
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_LIBCURL=1")
            endif()
        elseif (src MATCHES "/gen_acquire_extract.c$")
            # For extract, could detect MINIZ or LIBARCHIVE
            if (USE_MINIZ)
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_MINIZ=1")
            elseif (USE_LIBARCHIVE)
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION;USE_LIBARCHIVE=1")
            else()
                set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION")
            endif()
        elseif (src MATCHES "/gen_acquire_net_common.c$")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION")
        elseif (src MATCHES "/gen_acquire_string_extras.c$")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION")
        elseif (src MATCHES "/gen_acquire_url_utils.c$")
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "LIBACQUIRE_IMPLEMENTATION")
        else()
            set_source_files_properties(${src} PROPERTIES COMPILE_DEFINITIONS "")
        endif()
    endforeach()

    if (USE_LIBRHASH)
        message(STATUS "[libacquire] Using LibRHash for checksums")
        find_package(LibRHash REQUIRED)
        target_include_directories("${LIBRARY_NAME}_impl" PUBLIC "${LibRHash_INCLUDE_DIRS}")
        list(REMOVE_DUPLICATES LibRHash_LIBRARIES)
        target_link_libraries("${LIBRARY_NAME}_impl" PUBLIC "${LibRHash_LIBRARIES}")
        target_compile_definitions("${LIBRARY_NAME}_impl" PRIVATE USE_LIBRHASH=1)
    elseif (USE_CRC32C)
        message(STATUS "[libacquire] Using internal CRC32C implementation")
        target_compile_definitions("${LIBRARY_NAME}_impl" PRIVATE USE_CRC32C=1)
    else()
        message(FATAL_ERROR "No checksum implementation selected: define USE_LIBRHASH or USE_CRC32C")
    endif()

    if (NOT BSD)
        find_package(LibBSD)
        if (LibBSD_FOUND)
            if (NOT DEFINED LibBSD_LIBRARIES)
                set(LibBSD_LIBRARIES "${BSD_LIBRARY}")
            endif()
            target_link_directories("${LIBRARY_NAME}_impl" PUBLIC "${LibBSD_LIBRARIES}")
            if (DEFINED LibBSD_INCLUDEDIR)
                target_include_directories("${LIBRARY_NAME}_impl" PUBLIC "${LibBSD_INCLUDEDIR}")
            endif()
            target_compile_definitions("${LIBRARY_NAME}_impl" PUBLIC HAVE_STRNSTR=1)
            target_compile_definitions("${LIBRARY_NAME}_impl" PUBLIC HAVE_LIBBSD=1)
        endif()
    endif()

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
        get_target_property(linked_libs "${LIBRARY_NAME}" LINK_LIBRARIES)
        list(FIND linked_libs "${_lib}" lib_index)
        if(lib_index EQUAL -1)
            target_link_libraries("${LIBRARY_NAME}" "${lib_vis}" "${_lib}")
        else()
            message(STATUS "${LIBRARY_NAME} already linked with '${_lib}'")
        endif()
    endforeach (_lib ${LIBACQUIRE_LIBRARIES})
    message(STATUS "${LIBRARY_NAME} linked_libs = ${linked_libs}")
endif()

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
    get_target_property(linked_libs "${LIBRARY_NAME}" LINK_LIBRARIES)
    list(FIND linked_libs "${_lib}" lib_index)
    if(lib_index EQUAL -1)
        target_link_libraries("${LIBRARY_NAME}" "${lib_vis}" "${_lib}")
    else()
        message(STATUS "${LIBRARY_NAME} already linked with '${_lib}'")
    endif()
endforeach (_lib ${LIBACQUIRE_LIBRARIES})
message(STATUS "${LIBRARY_NAME} linked_libs = ${linked_libs}")
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
endif()

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
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

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

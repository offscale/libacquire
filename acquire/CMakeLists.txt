#get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(LIBRARY_NAME "lib${PROJECT_NAME}")

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
endforeach (_library "HTTPS" "CRYPTO" "EXTRACT")

set_checksum_lib()

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

#function (generate_source_file_from_header_only filepath header_filepath source_filepath)
#    # This function parses a header-only library file and creates a header and source file.
#    # Care is taken so that the same conditions hold, e.g., #if FEATURE\n#include "s.h"\n#endif /* FEATURE */
#    set(conditions "")
#    set(conditions_line "")
#    set(start_proto "")
#    set(end_proto "")
#    set(start_impl "")
#    set(end_impl "")
#    set(multi "")
#    set(line_no 0)
#    set(eat_from_line_no 0)
#    set(header_contents "")
#    get_filename_component(header_basename "${header_filepath}" NAME)
#    set(source_contents "")
#    set(eat_source "OFF")
#    set(eat_header "ON")
#    set(within_comment "OFF")
#    set(lparen 0)
#    set(rparen 0)
#    set(lbrace 0)
#    set(rbrace 0)
#    set(lsquare 0)
#    set(rsquare 0)
#
#
#    file(READ "${filepath}" contents)
#    string(LENGTH "${contents}" _len)
#    math(EXPR len "${_len} - 1")
#    set(line "")
#    foreach (char_index RANGE ${len})
#      string(SUBSTRING "${contents}" "${char_index}" "1" char)
#      string(LENGTH "${line}" line_len)
#      math(EXPR line_len_minus_1 "${line_len} - 1")
#      math(EXPR line_len_minus_2 "${line_len} - 2")
#      if (line_len_minus_1 GREATER 0)
#          string(SUBSTRING "${line}" "${line_len_minus_1}" "1" prev_char)
#      else ()
#          set(prev_char '')
#      endif ()
#      if (line_len_minus_2 GREATER 0)
#          string(SUBSTRING "${line}" "${line_len_minus_2}" "1" prev_prev_char)
#      else ()
#          set(prev_prev_char '')
#      endif ()
#      string(APPEND line "${char}")
#      # message(STATUS "prev_char = '${prev_char}'; char = '${char}'")
#
#      # Semicolon isn't sufficient, as we care about `concrete syntax` like:
#      # - function/struct start-of-definition;
#      # - statements;
#      # - multiline comments
#      #
#      # Form a 'line' out of anything ending in newline unless we're still in a:
#      # - comment
#      # - expression, ending in `||` or other boolean check?
#      # - parentheses (round || square)
#      if (prev_char STREQUAL '/' AND char STREQUAL '*')
#          set(within_comment "ON")
#      elseif (prev_char STREQUAL '*' AND char STREQUAL '/')
#          set(within_comment "OFF")
#      elseif (within_comment OR char STREQUAL '\\'
#              OR prev_prev_char STREQUAL '\\' OR prev_char STREQUAL '\\'
#              OR prev_prev_char STREQUAL '&' AND prev_char STREQUAL '&'
#              OR prev_char STREQUAL '&' AND char STREQUAL '&'
#              OR prev_prev_char STREQUAL '|' AND prev_char STREQUAL '|'
#              OR prev_char STREQUAL '|' AND char STREQUAL '|'
#              OR prev_prev_char STREQUAL '<' AND prev_char STREQUAL '<'
#              OR prev_prev_char STREQUAL '>' AND prev_char STREQUAL '>'
#              OR prev_prev_char STREQUAL '=' AND prev_char STREQUAL '='
#              OR prev_prev_char STREQUAL '!' AND prev_char STREQUAL '='
#              OR prev_prev_char STREQUAL '>' AND prev_char STREQUAL '='
#              OR prev_prev_char STREQUAL '<' AND prev_char STREQUAL '=')
#          
#      elseif (char STREQUAL "(")
#          math(EXPR lparen "${lparen} + 1")
#      #elseif (char STREQUAL '{')  # Actually brace is beginning of scope, don't confuse semantics with simple Concrete Syntax
#      #    math(EXPR lbrace "${lbrace} + 1")
#      elseif (char STREQUAL '[')
#          math(EXPR lsquare "${lsquare} + 1")
#      elseif (char STREQUAL ")")
#          math(EXPR rparen "${rparen} + 1")
#      #elseif (char STREQUAL '}')
#      #    math(EXPR rbrace "${rbrace} + 1")
#      elseif (char STREQUAL ']')
#          math(EXPR rsquare "${rsquare} + 1")
#      elseif (char STREQUAL "\n"
#              AND lparen EQUAL rparen
#              # AND lbrace EQUAL rbrace
#              AND lsquare EQUAL rsquare
#              AND (
#                prev_char STREQUAL "{" OR prev_char STREQUAL "}"
#                OR prev_prev_char STREQUAL "{" OR prev_prev_char STREQUAL "}"
#                # This will break on macro 'calls' that don't end in a semicolon
#                # and when literals contain one of these statements, like `s="for;while";`
#                OR line MATCHES "{"              # block start
#                OR line MATCHES "}"              # block end
#                OR prev_char STREQUAL ";"        # statement end
#                OR prev_prev_char STREQUAL ";"   # statement end
#                OR prev_prev_char STREQUAL "*" AND prev_char STREQUAL "/"  # C comment
#                OR line MATCHES "^#"             # macro
#                OR line MATCHES "^.*//"          # C++ style comment
#                OR line MATCHES "^.*for"         # for loop
#                OR line MATCHES "^.*while"       # while loop
#                OR line MATCHES "^.*if"          # if condition
#                OR line MATCHES "^.*else"        # else / else if condition
#                OR line MATCHES "^.*switch"      # switch statement
#                OR prev_char STREQUAL ":"        # label (incl. case & default)
#                OR prev_prev_char STREQUAL ":"   # label (incl. case & default)
#              ))
#        if (line_len)
#          if (DEBUG_TOKENISER)
#              message(STATUS "full_line = \"${line}\"")
#              message(STATUS "prev_prev_char = '${prev_prev_char}'; prev_char = '${prev_char}'; char = '${char}'")
#          endif (DEBUG_TOKENISER)
#          math(EXPR line_no "${line_no} + 1")
#          if (line MATCHES "^#.*endif")
#            list(POP_BACK conditions)
#            list(POP_BACK conditions_line)
#            if (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
#              list(APPEND end_impl "${line}")
#              list(APPEND end_line_impl "${line_no}")
#              list(SUBLIST "${contents}" "${start_line_impl}" "${end_line_impl}" new_contents_l)
#              if (DEBUG_TOKENISER)
#                  message(STATUS "start_line_impl = ${start_line_impl}")
#                  message(STATUS "end_line_impl = ${end_line_impl}")
#                  message(STATUS "new_contents_l = ${new_contents_l}")
#              endif (DEBUG_TOKENISER)
#
#              #string(SUBSTRING "${contents}" "${start_line_impl}" "${end_line_impl}" source_contents)
#              set(eat_source "OFF")
#              set(eat_header "ON")
#              set(eat_from_line_no "${line_no}")
#            endif (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
#          elseif (line MATCHES "^#.*if")
#            list(APPEND conditions "${line}")
#            list(APPEND conditions_line "${line_no}")
#            if (DEBUG_TOKENISER)
#                message(STATUS "[${line_no}] conditions = ${conditions}")
#                message(STATUS "[${line_no}] conditions_line = ${conditions_line}")
#            endif (DEBUG_TOKENISER)
#            if (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
#              list(APPEND start_impl "${line}")
#              list(APPEND start_line_impl "${line_no}")
#              if (DEBUG_TOKENISER)
#                  message(STATUS "start_impl = ${start_impl}")
#              endif (DEBUG_TOKENISER)
#              set(eat_from_line_no "${line_no}")
#              set(eat_source "ON")
#              set(eat_header "OFF")
#            endif (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
#          endif ()
#          if (eat_from_line_no STREQUAL "${line_no}")
#          elseif (eat_source)
#              string(APPEND source_contents "${line}")
#          elseif (eat_header)
#              string(APPEND header_contents "${line}")
#          endif ()
#          # TODO: Handle `elif` and multiline macros… and importantly nested ifs for feature flagging &etc.
#        endif (line_len)
#        set(skip_line 0)
#        set(line "")
#      endif ()
#    endforeach ()
#    file(WRITE "${header_filepath}" "${header_contents}")
#    if (source_contents)
#        file(WRITE "${source_filepath}" "#include \"${header_basename}\"\n\n" "${source_contents}")
#    endif (source_contents)
#endfunction (generate_source_file_from_header_only filepath header_filepath source_filepath)

# WARNING: This function attempts to parse C/C++ character-by-character in CMake.
# This approach is EXTREMELY INEFFICIENT, FRAGILE, and NOT RECOMMENDED.
# It is provided only because it was explicitly commanded.
# Expect performance issues and potential parsing errors on complex code.
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
                # Remove the second '/' from current_line if needed, or handle it as part of the comment line
            elseif (char STREQUAL "*" AND prev_char STREQUAL "/")
                set(state "BLOCK_COMMENT")
            elseif (char STREQUAL "\"")
                set(state "STRING")
                set(escape_next_char OFF) # Reset escape for start of string
            elseif (char STREQUAL "'")
                set(state "CHAR_LITERAL")
                set(escape_next_char OFF) # Reset escape for start of char literal
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
                # The newline itself will trigger line processing below
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
        # Try to detect the end of a C statement, preprocessor directive, or block scope change
        # Only process if we just hit a newline AND we are in CODE state AND parens/brackets seem balanced
        set(process_line OFF)
        if (char STREQUAL "\n" AND state STREQUAL "CODE")
            if (lparen EQUAL rparen AND lsquare EQUAL rsquare)
                # Trim whitespace from the line before checking end conditions
                string(STRIP "${current_line}" trimmed_line)
                string(LENGTH "${trimmed_line}" trimmed_len)
                if (trimmed_len GREATER 0)
                    math(EXPR last_trimmed_idx "${trimmed_len} - 1")
                    string(SUBSTRING "${trimmed_line}" ${last_trimmed_idx} 1 last_trimmed_char)
                    string(REGEX MATCH "^[ \t]*#" is_preprocessor "${trimmed_line}")

                    # Conditions considered "line ending":
                    # - Ends with semicolon (most statements)
                    # - Ends with { or } (scope changes)
                    # - Is a preprocessor directive (starts with #)
                    # - Ends with semicolon AND starts with # (simplified macro rule)
                    if (last_trimmed_char STREQUAL ";")
                        set(process_line ON)
                        # Handle simplified macro rule specifically
                        if (is_preprocessor)
                            # This is likely a macro definition/call ending in ;
                             # Already marked process_line ON
                        endif ()
                    elseif (last_trimmed_char STREQUAL "{")
                        set(process_line ON)
                    elseif (last_trimmed_char STREQUAL "}")
                        set(process_line ON)
                    elseif (is_preprocessor)
                        # Assume any line starting with # is a complete directive on this line
                        # (This is often FALSE for multi-line macros, but fits the simplified model)
                        set(process_line ON)
                        # Add other heuristic checks here if needed (e.g., specific keywords?), but they add fragility.
                        # Example: 'public:', 'private:' labels end with ':'
                    elseif (last_trimmed_char STREQUAL ":")
                        # Check if it's likely a label (heuristic!)
                        if (NOT trimmed_line MATCHES "[;={}(]") # If it doesn't contain typical statement chars... might be label
                            set(process_line ON)
                        endif ()
                    endif ()
                else ()
                    # Empty line (only whitespace), process it to preserve formatting
                    set(process_line ON)
                endif ()
            else ()
                # Newline occurred, but parens/brackets are unbalanced. Assume line continues.
                
            endif ()
        endif ()


        # --- Process Completed Line ---
        if (process_line)
            # Trim leading/trailing whitespace for matching, but use original `current_line` for output
            string(STRIP "${current_line}" line_for_match)
            set(append_to_header OFF)
            set(append_to_source OFF)

            # Determine destination (Header or Source) based on implementation block state FIRST
            if (in_implementation_block)
                set(append_to_source ON)
            else ()
                set(append_to_header ON)
            endif ()

            # Handle Preprocessor Directives & Update State
            if (line_for_match MATCHES "^[ \t]*#")
                if (line_for_match MATCHES "^[ \t]*#[ \t]*if(n?def)?") # Matches #if, #ifdef, #ifndef
                    string(REGEX REPLACE "^[ \t]*#[ \t]*if(n?def)?[ \t]+" "" condition_str "${line_for_match}")
                    set(directive_type "IF") # Default
                    if (line_for_match MATCHES "^[ \t]*#[ \t]*ifdef")
                        set(directive_type "IFDEF")
                    endif ()
                    if (line_for_match MATCHES "^[ \t]*#[ \t]*ifndef")
                        set(directive_type "IFNDEF")
                    endif ()

                    list(APPEND pp_conditions_stack "${directive_type};${condition_str}")

                    # Check if this starts the SPECIAL implementation block
                    if (condition_str MATCHES "LIBACQUIRE_IMPLEMENTATION")
                        if (implementation_block_depth EQUAL 0)
                            set(in_implementation_block ON)
                            # This line now belongs to SOURCE, override previous decision
                            set(append_to_header OFF)
                            set(append_to_source ON)
                        endif ()
                        math(EXPR implementation_block_depth "${implementation_block_depth} + 1")
                    endif ()

                elseif (line_for_match MATCHES "^[ \t]*#[ \t]*else")
                    # Doesn't change stack depth, might flip active state in a real parser
                    # For this splitter, we just pass it through based on current block state
                    
                elseif (line_for_match MATCHES "^[ \t]*#[ \t]*elif")
                    # Doesn't change stack depth, might flip active state in a real parser
                    # For this splitter, we just pass it through based on current block state
                    
                elseif (line_for_match MATCHES "^[ \t]*#[ \t]*endif")
                    if (pp_conditions_stack)
                        # Check if this #endif matches the LIBACQUIRE_IMPLEMENTATION block end
                        # We rely on the depth counter for the special block
                        set(is_impl_endif OFF)
                        # Check if the condition string on stack top contained the magic keyword (simplistic check)
                        # Or just rely on the depth counter decrementing to zero
                        # Using the comment simplification: "#endif /* FOO */"
                        if (line_for_match MATCHES "LIBACQUIRE_IMPLEMENTATION")
                            if (implementation_block_depth GREATER 0)
                                set(is_impl_endif ON)
                            endif ()
                        endif ()
                        # Alternative if comment isn't reliable: check stack top (more complex)
                        # list(GET pp_conditions_stack -1 top_condition_info)
                        # string(REPLACE ";" "|" top_condition_info_parts ${top_condition_info})
                        # list(GET top_condition_info_parts 1 top_condition_str)
                        # if(top_condition_str MATCHES "LIBACQUIRE_IMPLEMENTATION")
                        #     set(is_impl_endif ON)
                        # endif()

                        if (is_impl_endif)
                            math(EXPR implementation_block_depth "${implementation_block_depth} - 1")
                            if (implementation_block_depth EQUAL 0)
                                set(in_implementation_block OFF)
                                # Line belongs to SOURCE, but NEXT line belongs to HEADER
                                set(append_to_header OFF)
                                set(append_to_source ON)
                            endif ()
                        endif ()
                        # Always pop the stack for any #endif
                        list(POP_BACK pp_conditions_stack)
                    else ()
                        message(WARNING "Unmatched #endif encountered: ${current_line}")
                    endif ()
                    # Add other directives like #define, #include if specific handling needed
                    # By default, they just pass through based on current block state
                endif ()
            endif () # End preprocessor check

            # Append the line (including newline) to the correct content variable
            if (append_to_source)
                string(APPEND source_contents "${current_line}")
            elseif (append_to_header)
                string(APPEND header_contents "${current_line}")
            endif ()

            # Reset for next line
            set(current_line "")
            # Reset paren/bracket counts for the new line
            set(lparen 0)
            set(rparen 0)
            # set(lbrace 0) # Keep scope brace count if needed across lines? No, reset.
            # set(rbrace 0)
            set(lbrace 0)
            set(rbrace 0)
            set(lsquare 0)
            set(rsquare 0)
        endif () # End process_line
    endforeach () # End character loop

    # --- Finalization ---
    # Check for dangling state (e.g., unclosed comment/string)
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

    # Write Header
    file(WRITE "${header_filepath}" "${header_contents}")
    message(STATUS "Generated header (via character-by-character): ${header_filepath}")

    # Write Source (if applicable)
    if (source_contents OR in_implementation_block) # Check in_implementation_block for empty blocks
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
    foreach (header_file #${_Header_Files} ${Header_Files}
            "acquire_checksums.h"
            "acquire_common_defs.h"
            "acquire_crc32c.h"
            "acquire_download.h"
            "acquire_extract.h"
            "acquire_fileutils.h"
            "acquire_net_common.h"
            "acquire_string_extras.h"
            "acquire_url_utils.h")
        get_filename_component(name_no_ext "${header_file}" NAME_WLE)
        set(gen_header_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.h")
        set(gen_source_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.c")
        generate_source_file_from_header_only("${CMAKE_CURRENT_SOURCE_DIR}/${header_file}"
                "${gen_header_file}"
                "${gen_source_file}"
        )
        if (EXISTS "${gen_header_file}")
            list(APPEND gen_header_files "${gen_header_file}")
        endif (EXISTS "${gen_header_file}")
        if (EXISTS "${gen_source_file}")
            list(APPEND gen_source_files "${gen_source_file}")
        endif (EXISTS "${gen_source_file}")
    endforeach (header_file ${_Header_Files} ${Header_Files})
    #    generate_source_file_from_header_only("acquire_checksums.h"
    #            "${CMAKE_BINARY_DIR}/generated_checksums.h"
    #            "${CMAKE_BINARY_DIR}/generated_checksums.c")

    add_library("${LIBRARY_NAME}_lib" SHARED "${gen_header_files}" "${gen_source_files}")

    target_compile_definitions("${LIBRARY_NAME}_lib" PRIVATE LIBACQUIRE_IMPLEMENTATION=1)

    if (NOT BSD)
        find_package(LibBSD)
        if (LibBSD_FOUND)
            if (NOT DEFINED LibBSD_LIBRARIES)
                set(LibBSD_LIBRARIES "${BSD_LIBRARY}")
            endif (NOT DEFINED LibBSD_LIBRARIES)
            target_link_directories("${LIBRARY_NAME}_lib" PUBLIC "${LibBSD_LIBRARIES}")
            if (DEFINED LibBSD_INCLUDEDIR)
                target_include_directories("${LIBRARY_NAME}_lib" PUBLIC "${LibBSD_INCLUDEDIR}")
            endif (DEFINED LibBSD_INCLUDEDIR)
            target_compile_definitions("${LIBRARY_NAME}_lib" PUBLIC HAVE_STRNSTR=1)
            target_compile_definitions("${LIBRARY_NAME}_lib" PUBLIC HAVE_LIBBSD=1)
        endif (LibBSD_FOUND)
    endif (NOT BSD)

    message(STATUS "${LIBRARY_NAME}_lib::CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
    target_include_directories(
            "${LIBRARY_NAME}_lib"
            PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gen>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/acquire>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>"
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/acquire>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
    set_target_properties(
            "${LIBRARY_NAME}_lib"
            PROPERTIES
            LINKER_LANGUAGE
            C
    )
    install(FILES "${gen_header_files}"
            TYPE "INCLUDE")
    list(APPEND installable_libs "${LIBRARY_NAME}_lib")

    add_library("${LIBRARY_NAME}" SHARED "${_Header_Files}" "${Header_Files}")
    target_link_libraries("${LIBRARY_NAME}" PUBLIC "${LIBRARY_NAME}_lib")
    if(USE_LIBRHASH)
        find_package(LibRHash REQUIRED)
        target_include_directories(${LIBRARY_NAME}_lib PUBLIC ${LibRHash_INCLUDE_DIRS})
        target_link_libraries(${LIBRARY_NAME}_lib PUBLIC ${LibRHash_LIBRARIES})
    endif()
    foreach (lib ${LIBACQUIRE_LIBRARIES})
        message(STATUS "${LIBRARY_NAME} being linked with ${lib}")
        target_link_libraries("${LIBRARY_NAME}" PUBLIC "${lib}")
    endforeach ()
    set(lib_vis "PUBLIC")
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

target_link_libraries(
        "${LIBRARY_NAME}"
        "${lib_vis}"
        "${PROJECT_NAME}_compiler_flags"
        "${LIBACQUIRE_LIBRARIES}"
)

target_compile_definitions("${LIBRARY_NAME}" "${lib_vis}" "_${TARGET_ARCH}_")

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")

if (LIBACQUIRE_HEADER_ONLY)
    file(WRITE "${_export_file}"
            "#ifndef LIBACQUIRE_LIB_EXPORT_H\n"
            "#define LIBACQUIRE_LIB_EXPORT_H\n\n"
            "#  ifndef LIBACQUIRE_LIB_EXPORT\n"
            "#    define LIBACQUIRE_LIB_EXPORT\n"
            "#  endif /* !LIBACQUIRE_LIB_EXPORT */\n\n"
            "#endif /* !LIBACQUIRE_LIB_EXPORT_H */\n")
else ()
    generate_export_header("${LIBRARY_NAME}_lib" EXPORT_FILE_NAME "${_export_file}")
endif ()

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

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

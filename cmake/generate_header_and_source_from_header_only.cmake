# Function to split header-only .h file into separate header .h and source .c files
function(generate_header_and_source_from_header_only guard_name associated_header input_file output_header output_source)
    # limitations:
    # - expects a comment on endif, like `#endif /* LIBACQUIRE_IMPLEMENTATION */`
    # - only handles the first `LIBACQUIRE_IMPLEMENTATION` impl
    # - doesn't evaluate macros, so `!defined(LIBACQUIRE_IMPLEMENTATION)` will still move that block to .c file

    # implementation
    # - superior to basic find/extract, as it parses: str & char literals; comments (both kinds); and line-continuation.

    if (NOT EXISTS "${input_file}")
        message(FATAL_ERROR "Input file not found: ${input_file}")
    elseif (IS_DIRECTORY "${input_file}")
        message(FATAL_ERROR "Unexpected dir (expected file): ${input_file}")
    endif ()

    file(READ "${input_file}" content)
    string(LENGTH "${content}" content_len)

    set(i 0)
    set(in_char OFF)
    set(in_str OFF)
    set(in_c_comment OFF)
    set(in_cpp_comment OFF)
    set(in_macro OFF)

    set(macro_begin_start -1)
    set(macro_begin_end -1)
    set(macro_finish_start -1)
    set(macro_finish_end -1)

    set(prev_ch "")
    set(line "")
    set(line_starts_idx 0)
    set(clear_line OFF)
    set(end_guard_found OFF)

    # Track #if nesting level in impl block
    set(impl_nesting_level 0)
    set(in_impl_block OFF)

    while (i LESS content_len)
        string(SUBSTRING "${content}" ${i} 1 char)
        math(EXPR i_plus_1 "${i} + 1")

        if (in_char AND char STREQUAL "'" AND NOT prev_ch STREQUAL "\\")
            set(in_char OFF)
        elseif (in_str AND char STREQUAL "\"" AND NOT prev_ch STREQUAL "\\")
            set(in_str OFF)
        elseif (in_c_comment AND "${prev_ch}${char}" STREQUAL "*/")
            set(in_c_comment OFF)
        elseif (in_cpp_comment AND char STREQUAL "\n")
            set(in_cpp_comment OFF)
            set(clear_line ON)
            set(line_starts_idx ${i_plus_1})
        elseif (in_macro AND char STREQUAL "\n" AND NOT prev_ch STREQUAL "\\")
            set(in_macro OFF)
            if (macro_finish_start EQUAL -1)
                set(macro_begin_end ${i})
                math(EXPR macro_begin_len "${macro_begin_end} - ${macro_begin_start}")
                string(SUBSTRING "${content}" ${macro_begin_start} ${macro_begin_len} macro_begin)
            else ()
                set(macro_finish_end ${i})
                math(EXPR macro_finish_len "${macro_finish_end} - ${macro_finish_start}")
                string(SUBSTRING "${content}" ${macro_finish_start} ${macro_finish_len} macro_finish)

                if (macro_finish MATCHES ".*${guard_name}.*")
                    if (macro_finish MATCHES "^[ \t]*#[ \t]*(if|ifdef|ifndef)[ \t]+.*")
                        if (in_impl_block STREQUAL OFF)
                            set(in_impl_block ON)
                            set(impl_nesting_level 1)
                            set(macro_begin_start ${macro_finish_start})
                            set(macro_begin_end ${macro_finish_end})
                        else ()
                            math(EXPR impl_nesting_level "${impl_nesting_level} + 1")
                        endif ()
                    elseif (macro_finish MATCHES "^[ \t]*#[ \t]*endif.*")
                        if (in_impl_block STREQUAL ON)
                            math(EXPR impl_nesting_level "${impl_nesting_level} - 1")

                            if (impl_nesting_level EQUAL 0)
                                set(end_guard_found ON)
                                set(macro_finish_start ${macro_finish_start})
                                set(macro_finish_end ${macro_finish_end})
                                break()
                            endif (impl_nesting_level EQUAL 0)
                        endif (in_impl_block STREQUAL ON)
                    else ()
                        if (in_impl_block STREQUAL ON)
                            # Other macros inside impl block (#else, #elif): do nothing special
                        else ()
                            # message(STATUS "Unrelated macro: ${line}")
                        endif ()
                    endif ()
                else ()
                    # message(STATUS "Unrelated macro (no guard_name): ${line}")
                endif ()
            endif ()
            set(clear_line ON)
            set(line_starts_idx ${i_plus_1})
        else ()
            if (prev_ch STREQUAL "\\")
                set(i ${i_plus_1})
                set(prev_ch "${char}")
                continue()
            elseif (char STREQUAL "'")
                set(in_char ON)
            elseif (char STREQUAL "\"")
                set(in_str ON)
            elseif ("${prev_ch}${char}" STREQUAL "/*")
                set(in_c_comment ON)
            elseif ("${prev_ch}${char}" STREQUAL "//")
                set(in_cpp_comment ON)
            elseif (char STREQUAL "\n")
                set(clear_line ON)
                set(line_starts_idx ${i_plus_1})
            elseif (char STREQUAL "#" AND (line STREQUAL "" OR line MATCHES "^[ \t\r]+#+"))
                set(in_macro ON)
                if (macro_begin_start EQUAL -1)
                    set(macro_begin_start ${i})
                else ()
                    set(macro_finish_start ${i})
                endif ()
            endif ()

            set(line "${line}${char}")

            if (clear_line)
                set(line "")
                set(clear_line OFF)
            endif (clear_line)

            set(i ${i_plus_1})
            set(prev_ch "${char}")
        endif ()
    endwhile (i LESS content_len)

    if (NOT end_guard_found)
        # No guarded source found, write original content as header
        file(WRITE "${output_header}" "${content}")
        return()
    endif (NOT end_guard_found)

    # Compose header content WITHOUT the implementation block
    string(SUBSTRING "${content}" 0 ${macro_begin_start} head_start)
    string(SUBSTRING "${content}" ${macro_finish_end} -1 head_end)

    # Extract implementation content WITHOUT the opening #if.. and closing #endif lines
    math(EXPR impl_length "${macro_finish_start} - ${macro_begin_end}")
    string(SUBSTRING "${content}" ${macro_begin_end} ${impl_length} impl_content)

    # Optionally strip first #define <guard>_... line in impl_content
    string(FIND "${impl_content}" "\n" first_nl_pos)
    if (NOT first_nl_pos EQUAL -1)
        string(SUBSTRING "${impl_content}" 0 ${first_nl_pos} first_line)
        string(STRIP "${first_line}" first_line_stripped)
        # This regex matches lines like "#define LIBACQUIRE_ACQUIRE_FILEUTILS_IMPL"
        if (first_line_stripped MATCHES "^#define[ \t]+${guard_name}(_.*)?$")
            math(EXPR new_start_pos "${first_nl_pos} + 1")
            string(SUBSTRING "${impl_content}" ${new_start_pos} -1 impl_content)
        endif (first_line_stripped MATCHES "^#define[ \t]+${guard_name}(_.*)?$")
    endif (NOT first_nl_pos EQUAL -1)

    if (associated_header STREQUAL "")
        get_filename_component(associated_header "${output_header}" NAME)
    endif (associated_header STREQUAL "")

    file(WRITE "${output_header}" "${head_start}${head_end}")
    file(WRITE "${output_source}" "#include \"${associated_header}\"\n\n${impl_content}")
endfunction(generate_header_and_source_from_header_only guard_name associated_header input_file output_header output_source)

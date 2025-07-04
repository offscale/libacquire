#-----------------------------------------------------------------------
# rewrite_generated_includes
#
#   Scans generated C source files and rewrites their #include directives
#   to point to other generated headers instead of the original ones.
#
#   This is critical for a "header-only" library that has been split
#   into interface (.h) and implementation (.c) files at build time.
#
# Arguments:
#   SOURCE_FILES    - A list of source files to process (the generated .c files).
#   GEN_HEADERS     - A pre-calculated list of available generated header
#                     basenames (e.g., "gen_acquire_util.h").
#
function(rewrite_includes_to_gen)
    # Use the modern cmake_parse_arguments to correctly handle function parameters
    set(options)
    set(one_value_args)
    set(multi_value_args SOURCE_FILES GEN_HEADERS)
    cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    foreach (source_file IN LISTS ARG_SOURCE_FILES)
        file(READ "${source_file}" content)
        set(original_content "${content}")

        foreach (gen_header_basename IN LISTS ARG_GEN_HEADERS)
            string(REPLACE "gen_acquire_" "acquire_" original_header_basename "${gen_header_basename}")

            string(REPLACE "." "\\." escaped_original_basename "${original_header_basename}")

            set(search_pattern "#[ \t]*include[ \t]*([<\"])${escaped_original_basename}([\">])")

            set(replace_pattern "#include \\1${gen_header_basename}\\2")

            string(REGEX REPLACE "${search_pattern}" "${replace_pattern}" content "${content}")
        endforeach (gen_header_basename IN LISTS ARG_GEN_HEADERS)

        if (NOT "${content}" STREQUAL "${original_content}")
            file(WRITE "${source_file}" "${content}")
        endif (NOT "${content}" STREQUAL "${original_content}")
    endforeach (source_file IN LISTS ARG_SOURCE_FILES)
endfunction(rewrite_includes_to_gen)

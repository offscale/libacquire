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

function (generate_source_file_from_header_only filepath header_filepath source_filepath)
    # This function parses a header-only library file and creates a header and source file.
    # Care is taken so that the same conditions hold, e.g., #if FEATURE\n#include "s.h"\n#endif /* FEATURE */
    set(conditions "")
    set(conditions_line "")
    set(start_proto "")
    set(end_proto "")
    set(start_impl "")
    set(end_impl "")
    set(multi "")
    set(line_no 0)
    set(eat_from_line_no 0)
    set(header_contents "")
    get_filename_component(header_basename "${header_filepath}" NAME)
    set(source_contents "")
    set(eat_source "OFF")
    set(eat_header "ON")
    set(within_comment "OFF")
    set(lparen 0)
    set(rparen 0)
    set(lbrace 0)
    set(rbrace 0)
    set(lsquare 0)
    set(rsquare 0)


    file(READ "${filepath}" contents)
    string(LENGTH "${contents}" _len)
    math(EXPR len "${_len} - 1")
    set(line "")
    foreach (char_index RANGE ${len})
      string(SUBSTRING "${contents}" "${char_index}" "1" char)
      string(LENGTH "${line}" line_len)
      math(EXPR line_len_minus_1 "${line_len} - 1")
      math(EXPR line_len_minus_2 "${line_len} - 2")
      if (line_len_minus_1 GREATER 0)
          string(SUBSTRING "${line}" "${line_len_minus_1}" "1" prev_char)
      else ()
          set(prev_char '')
      endif ()
      if (line_len_minus_2 GREATER 0)
          string(SUBSTRING "${line}" "${line_len_minus_2}" "1" prev_prev_char)
      else ()
          set(prev_prev_char '')
      endif ()
      string(APPEND line "${char}")
      # message(STATUS "prev_char = '${prev_char}'; char = '${char}'")

      # Semicolon isn't sufficient, as we care about `concrete syntax` like:
      # - function/struct start-of-definition;
      # - statements;
      # - multiline comments
      #
      # Form a 'line' out of anything ending in newline unless we're still in a:
      # - comment
      # - expression, ending in `||` or other boolean check?
      # - parentheses (round || square)
      if (prev_char STREQUAL '/' AND char STREQUAL '*')
          set(within_comment "ON")
      elseif (prev_char STREQUAL '*' AND char STREQUAL '/')
          set(within_comment "OFF")
      elseif (within_comment OR char STREQUAL '\\'
              OR prev_prev_char STREQUAL '\\' OR prev_char STREQUAL '\\'
              OR prev_prev_char STREQUAL '&' AND prev_char STREQUAL '&'
              OR prev_char STREQUAL '&' AND char STREQUAL '&'
              OR prev_prev_char STREQUAL '|' AND prev_char STREQUAL '|'
              OR prev_char STREQUAL '|' AND char STREQUAL '|'
              OR prev_prev_char STREQUAL '<' AND prev_char STREQUAL '<'
              OR prev_prev_char STREQUAL '>' AND prev_char STREQUAL '>'
              OR prev_prev_char STREQUAL '=' AND prev_char STREQUAL '='
              OR prev_prev_char STREQUAL '!' AND prev_char STREQUAL '='
              OR prev_prev_char STREQUAL '>' AND prev_char STREQUAL '='
              OR prev_prev_char STREQUAL '<' AND prev_char STREQUAL '=')
          pass()
      elseif (char STREQUAL "(")
          math(EXPR lparen "${lparen} + 1")
      #elseif (char STREQUAL '{')  # Actually brace is beginning of scope, don't confuse semantics with simple Concrete Syntax
      #    math(EXPR lbrace "${lbrace} + 1")
      elseif (char STREQUAL '[')
          math(EXPR lsquare "${lsquare} + 1")
      elseif (char STREQUAL ")")
          math(EXPR rparen "${rparen} + 1")
      #elseif (char STREQUAL '}')
      #    math(EXPR rbrace "${rbrace} + 1")
      elseif (char STREQUAL ']')
          math(EXPR rsquare "${rsquare} + 1")
      elseif (char STREQUAL "\n"
              AND lparen EQUAL rparen
              # AND lbrace EQUAL rbrace
              AND lsquare EQUAL rsquare
              AND (
                prev_char STREQUAL "{" OR prev_char STREQUAL "}"
                OR prev_prev_char STREQUAL "{" OR prev_prev_char STREQUAL "}"
                # This will break on macro 'calls' that don't end in a semicolon
                # and when literals contain one of these statements, like `s="for;while";`
                OR line MATCHES "{"              # block start
                OR line MATCHES "}"              # block end
                OR prev_char STREQUAL ";"        # statement end
                OR prev_prev_char STREQUAL ";"   # statement end
                OR prev_prev_char STREQUAL "*" AND prev_char STREQUAL "/"  # C comment
                OR line MATCHES "^#"             # macro
                OR line MATCHES "^.*//"          # C++ style comment
                OR line MATCHES "^.*for"         # for loop
                OR line MATCHES "^.*while"       # while loop
                OR line MATCHES "^.*if"          # if condition
                OR line MATCHES "^.*else"        # else / else if condition
                OR line MATCHES "^.*switch"      # switch statement
                OR prev_char STREQUAL ":"        # label (incl. case & default)
                OR prev_prev_char STREQUAL ":"   # label (incl. case & default)
              ))
        if (line_len)
          if (DEBUG_TOKENISER)
              message(STATUS "full_line = \"${line}\"")
              message(STATUS "prev_prev_char = '${prev_prev_char}'; prev_char = '${prev_char}'; char = '${char}'")
          endif (DEBUG_TOKENISER)
          math(EXPR line_no "${line_no} + 1")
          if (line MATCHES "^#.*endif")
            list(POP_BACK conditions)
            list(POP_BACK conditions_line)
            if (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
              list(APPEND end_impl "${line}")
              list(APPEND end_line_impl "${line_no}")
              list(SUBLIST "${contents}" "${start_line_impl}" "${end_line_impl}" new_contents_l)
              if (DEBUG_TOKENISER)
                  message(STATUS "start_line_impl = ${start_line_impl}")
                  message(STATUS "end_line_impl = ${end_line_impl}")
                  message(STATUS "new_contents_l = ${new_contents_l}")
              endif (DEBUG_TOKENISER)

              #string(SUBSTRING "${contents}" "${start_line_impl}" "${end_line_impl}" source_contents)
              set(eat_source "OFF")
              set(eat_header "ON")
              set(eat_from_line_no "${line_no}")
            endif (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
          elseif (line MATCHES "^#.*if")
            list(APPEND conditions "${line}")
            list(APPEND conditions_line "${line_no}")
            if (DEBUG_TOKENISER)
                message(STATUS "[${line_no}] conditions = ${conditions}")
                message(STATUS "[${line_no}] conditions_line = ${conditions_line}")
            endif (DEBUG_TOKENISER)
            if (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
              list(APPEND start_impl "${line}")
              list(APPEND start_line_impl "${line_no}")
              if (DEBUG_TOKENISER)
                  message(STATUS "start_impl = ${start_impl}")
              endif (DEBUG_TOKENISER)
              set(eat_from_line_no "${line_no}")
              set(eat_source "ON")
              set(eat_header "OFF")
            endif (line MATCHES ".*LIBACQUIRE_IMPLEMENTATION.*")
          endif ()
          if (eat_from_line_no STREQUAL "${line_no}")
          elseif (eat_source)
              string(APPEND source_contents "${line}")
          elseif (eat_header)
              string(APPEND header_contents "${line}")
          endif ()
          # TODO: Handle `elif` and multiline macros… and importantly nested ifs for feature flagging &etc.
        endif (line_len)
        set(skip_line 0)
        set(line "")
      endif ()
    endforeach ()
    file(WRITE "${header_filepath}" "${header_contents}")
    if (source_contents)
        file(WRITE "${source_filepath}" "#include \"${header_basename}\"\n\n" "${source_contents}")
    endif (source_contents)
endfunction (generate_source_file_from_header_only filepath header_filepath source_filepath)

if (LIBACQUIRE_HEADER_ONLY)
    add_library("${LIBRARY_NAME}" INTERFACE "${_Header_Files}" "${Header_Files}")
    set(lib_vis "INTERFACE")
else ()
    set(gen_header_files "")
    set(gen_source_files "")
    foreach (header_file #${_Header_Files} ${Header_Files}
            "acquire_checksums.h" "acquire_common_defs.h" "acquire_download.h"
            "acquire_extract.h" "acquire_fileutils.h" "acquire_net_common.h"
            "acquire_string_extras.h" "acquire_url_utils.h")
        get_filename_component(name_no_ext "${header_file}" NAME_WLE)
        set(gen_header_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.h")
        set(gen_source_file "${CMAKE_BINARY_DIR}/gen/gen_${name_no_ext}.c")
        generate_source_file_from_header_only("${header_file}"
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

    if (NOT BSD)
        find_package(LibBSD)
        if (LibBSD_FOUND)
            target_link_directories("${LIBRARY_NAME}_lib" PUBLIC "${BSD_LIBRARY}")
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

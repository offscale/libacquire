get_filename_component(exec_name "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

include("${CMAKE_SOURCE_DIR}/cmake/FindCurlCustom.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/libacquire_helpers.cmake")

#########################
# Dependencies download #
#########################

set(GREATEST_SHA256 "1292593d95c35eeccc89ffa1c91d6fe53b49f81cbf2c2b7758842b7f3186fcc2")
set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/test_downloads")

file(TO_NATIVE_PATH "${DOWNLOAD_DIR}" DOWNLOAD_DIR)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
  string(REPLACE "\\" "\\\\" DOWNLOAD_DIR "${DOWNLOAD_DIR}")
endif ()

file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")
set(GREATEST_BASEFILENAME "greatest.h")
set(GREATEST_FILE "${DOWNLOAD_DIR}/${GREATEST_BASEFILENAME}")
file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
  string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
endif ()

set(GREATEST_URL "https://raw.githubusercontent.com/silentbicycle/greatest/11a6af1/greatest.h")
file(DOWNLOAD "${GREATEST_URL}" "${GREATEST_FILE}"
        EXPECTED_HASH "SHA256=${GREATEST_SHA256}")

################################
# Configure file (interpolate) #
################################

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" _CMAKE_CURRENT_SOURCE_DIR)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_LIST_FILE}" _CMAKE_CURRENT_LIST_FILE)
file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
download_extract_miniz("${DOWNLOAD_DIR}")
file(TO_NATIVE_PATH "${ZLIB_ZIP_FILE}" ZLIB_ZIP_FILE)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
  string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_SOURCE_DIR "${_CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_LIST_FILE "${_CMAKE_CURRENT_LIST_FILE}")
  string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
  string(REPLACE "\\" "\\\\" ZLIB_ZIP_FILE "${ZLIB_ZIP_FILE}")
endif ()

configure_file(config_for_tests.h.in "${PROJECT_BINARY_DIR}/src/config_for_tests.h" @ONLY)

###############################
# Set libacquire dependencies #
###############################

include("${CMAKE_SOURCE_DIR}/cmake/libacquire_helpers.cmake")

set_crypto_lib()

set_ssl_lib()

set_http_https_lib()

set_extract_lib()

include("${CMAKE_SOURCE_DIR}/cmake/FindLibAcquire.cmake")

set(_targets "${exec_name}" "test_amalgamation")
set(All_Header_Files "${GREATEST_FILE}")

foreach (_library "HTTPS" "CRYPTO" "EXTRACT")
  message(STATUS "[test_libacquire] ${_library}_LIB: \t${${_library}_LIB}")
endforeach (_library "HTTPS" "CRYPTO" "EXTRACT")

foreach (EXEC_NAME ${_targets})
  if (EXEC_NAME STREQUAL "test_amalgamation")
    set(Header_Files "${EXEC_NAME}.h")
    source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")

    set(Source_Files "${EXEC_NAME}.c")
    source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")
    add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
    target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBACQUIRE_LIBRARIES}" "miniz")
  else ()
    set(Header_Files "test_fileutils.h" "test_string_extras.h" "test_checksum.h")
    source_group("Header Files" FILES "${Header_Files}")

    set(Source_Files "test.c")
    source_group("Source Files" FILES "${Source_Files}")

    add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

    target_link_libraries("${EXEC_NAME}" PRIVATE "libacquire")
  endif ()

  target_include_directories(
          "${EXEC_NAME}"
          PRIVATE
          "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
          "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>"
          "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
          "$<INSTALL_INTERFACE:include>"
  )

  add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")

  #######################################
  # Export / installation configuration #
  #######################################

  # setup the version numbering
  #set_property(TARGET "${EXEC_NAME}" PROPERTY VERSION "1.0.0")
  #set_property(TARGET "${EXEC_NAME}" PROPERTY SOVERSION "1")

  # install rules
  set(installable_libs "${EXEC_NAME}")
  if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
  endif (TARGET "${DEPENDANT_LIBRARY}")
  list(APPEND All_Header_Files "${Header_Files}")
  install(TARGETS ${installable_libs}
          DESTINATION "bin"
          EXPORT "${EXEC_NAME}Targets")
endforeach (EXEC_NAME ${_targets})

install(FILES
        "${PROJECT_BINARY_DIR}/src/config_for_tests.h"
        "${All_Header_Files}"
        "${GREATEST_FILE}"
        DESTINATION
        "include")

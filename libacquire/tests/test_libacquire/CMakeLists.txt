get_filename_component(EXEC_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" EXEC_NAME "${EXEC_NAME}")

include(../../../cmake/libacquire_helpers.cmake)

set(Header_Files "test_fileutils.h" "test_string_extras.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "test.c")
source_group("Source Files" FILES "${Source_Files}")

#######################
# Dependency download #
#######################

file(DOWNLOAD https://raw.githubusercontent.com/silentbicycle/greatest/master/greatest.h
        "${PROJECT_BINARY_DIR}/downloads/greatest.h")

################################
# Configure file (interpolate) #
################################

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" _CMAKE_CURRENT_SOURCE_DIR)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_LIST_FILE}" _CMAKE_CURRENT_LIST_FILE)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
  string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_SOURCE_DIR "${_CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "\\" "\\\\" _CMAKE_CURRENT_LIST_FILE "${_CMAKE_CURRENT_LIST_FILE}")
endif ()

configure_file(config_for_tests.h.in "${PROJECT_BINARY_DIR}/src/config_for_tests.h" @ONLY)

###############################
# Set libacquire dependencies #
###############################

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

target_link_libraries("${EXEC_NAME}" PRIVATE "libacquire")
target_include_directories(
        "${EXEC_NAME}"
        PRIVATE
        "${PROJECT_BINARY_DIR}/downloads"
        "${PROJECT_BINARY_DIR}/src"
)

add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")

#######################################
# Export / installation configuration #
#######################################

# setup the version numbering
#set_property(TARGET "${EXEC_NAME}" PROPERTY VERSION "1.0.0")
#set_property(TARGET "${EXEC_NAME}" PROPERTY SOVERSION "1")

# install rules
set(installable_libs "${EXEC_NAME}" "${PROJECT_LOWER_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
  list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "bin"
        EXPORT "${EXEC_NAME}Targets")
install(FILES
        "${PROJECT_BINARY_DIR}/src/config_for_tests.h"
        "${Header_Files}"
        DESTINATION
        "include")

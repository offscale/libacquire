get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" LIBRARY_NAME "${LIBRARY_NAME}")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

include(GenerateExportHeader)

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

############################
# Library target formation #
############################

add_library("${LIBRARY_NAME}" SHARED "${Header_Files}" "${Source_Files}")

set(_libs "${PROJECT_LOWER_NAME}_compiler_flags")
if (DEFINED WITH_SSL)
    find_package(OpenSSL REQUIRED)
    list(APPEND _libs "OpenSSL::SSL")
endif ()

target_link_libraries("${LIBRARY_NAME}" PUBLIC "${_libs}")

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
set(_export_file "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

# install rules
set(installable_libs "${LIBRARY_NAME}" "${PROJECT_LOWER_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "bin"
        EXPORT "${LIBRARY_NAME}Targets")

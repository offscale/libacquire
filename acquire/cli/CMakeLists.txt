set(EXEC_NAME "acquire_cli")

############################
# Library target formation #
############################

set(LIBRARY_NAME "${EXEC_NAME}_lib")

set(Header_Files "cli.h")
source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")

set(Source_Files "cli.c")
source_group("${LIBRARY_NAME} Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")

target_link_libraries("${LIBRARY_NAME}" PRIVATE "${PROJECT_NAME}_compiler_flags")
#foreach (lib ${LIBACQUIRE_LIBRARIES})
#    message(STATUS "${LIBRARY_NAME} being linked with ${lib}")
#    target_link_libraries("${LIBRARY_NAME}" PUBLIC "${lib}")
#endforeach (lib ${LIBACQUIRE_LIBRARIES})

include(GNUInstallDirs)

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/acquire>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

###############################
# Executable target formation #
###############################

set(Source_Exec_Files "main.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Exec_Files}")

add_executable("${EXEC_NAME}" "${Source_Exec_Files}")
target_compile_definitions("${EXEC_NAME}" PUBLIC "_${TARGET_ARCH}_")
target_link_libraries(
        "${EXEC_NAME}"
        PRIVATE
        "${PROJECT_NAME}_compiler_flags"
        "${LIBRARY_NAME}"
        "${PROJECT_NAME}"
)

set_extract_lib()
if (WIN32 AND (NOT DEFINED EXTRACT_LIB OR EXTRACT_LIB STREQUAL "" OR EXTRACT_LIB STREQUAL "WINCOMPRESSAPI"))
    set(LIBACQUIRE_USE_MINIZ 1)
    set(EXTRACT_LIB "miniz")
endif (WIN32 AND (NOT DEFINED EXTRACT_LIB OR EXTRACT_LIB STREQUAL "" OR EXTRACT_LIB STREQUAL "WINCOMPRESSAPI"))

if (LIBACQUIRE_USE_LIBCURL)
    target_compile_definitions(${EXEC_NAME} PRIVATE LIBACQUIRE_USE_LIBCURL=1)
    set_source_files_properties(
            ${Source_Exec_Files} PROPERTIES
            COMPILE_DEFINITIONS "LIBACQUIRE_USE_LIBCURL=1"
    )
elseif (LIBACQUIRE_USE_LIBFETCH)
    target_compile_definitions(${EXEC_NAME} PRIVATE LIBACQUIRE_USE_LIBFETCH=1)
    set_source_files_properties(
            ${Source_Exec_Files} PROPERTIES
            COMPILE_DEFINITIONS "LIBACQUIRE_USE_LIBFETCH=1"
    )
elseif (LIBACQUIRE_USE_WININET)
    target_compile_definitions(${EXEC_NAME} PRIVATE LIBACQUIRE_USE_WININET=1)
    set_source_files_properties(
            ${Source_Exec_Files} PROPERTIES
            COMPILE_DEFINITIONS "LIBACQUIRE_USE_WININET=1"
    )
elseif (LIBACQUIRE_USE_OPENBSD_FTP)
    target_compile_definitions(${EXEC_NAME} PRIVATE LIBACQUIRE_USE_OPENBSD_FTP=1)
    set_source_files_properties(
            ${Source_Exec_Files} PROPERTIES
            COMPILE_DEFINITIONS "LIBACQUIRE_USE_OPENBSD_FTP=1"
    )
endif ()

if (LIBACQUIRE_USE_MINIZ)
    target_compile_definitions(${EXEC_NAME} PRIVATE LIBACQUIRE_USE_MINIZ=1)
    set_source_files_properties(
            ${Source_Exec_Files} PROPERTIES
            COMPILE_DEFINITIONS "LIBACQUIRE_USE_MINIZ=1"
    )
elseif (LIBACQUIRE_USE_LIBARCHIVE)
    target_compile_definitions(${EXEC_NAME} PRIVATE LIBACQUIRE_USE_LIBARCHIVE=1)
    set_source_files_properties(
            ${Source_Exec_Files} PROPERTIES
            COMPILE_DEFINITIONS "LIBACQUIRE_USE_LIBARCHIVE=1"
    )
endif ()

target_include_directories(
        "${EXEC_NAME}"
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/acquire>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/acquire>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

#######################################
# Export / installation configuration #
#######################################

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "${${PROJECT_NAME}_VERSION}")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}")

# install rules
set(installable_libs "${EXEC_NAME}" "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif (TARGET "${DEPENDANT_LIBRARY}")
install(TARGETS ${installable_libs}
        EXPORT "${EXEC_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES "${_export_file}" ${Header_Files}
        TYPE "INCLUDE")

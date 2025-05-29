get_filename_component(EXEC_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" EXEC_NAME "${EXEC_NAME}")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

include(GenerateExportHeader)

set(Header_Files "ftp_var.h" "extern.h" "stringlist.h" "small.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "ftp.c" "main.c")
source_group("Source Files" FILES "${Source_Files}")

##############################
# Sometimes dependent target #
##############################

include(CheckSymbolExists)

check_symbol_exists(asprintf "stdio.h" HAVE_ASPRINTF)
if (HAVE_ASPRINTF)
    add_compile_definitions(HAVE_ASPRINTF)
    add_subdirectory("morestd")
endif (HAVE_ASPRINTF)
check_symbol_exists(va_copy stdarg.h HAVE_VA_COPY)
if (HAVE_VA_COPY)
    add_compile_definitions(HAVE_VA_COPY)
endif (HAVE_VA_COPY)

###############################
# Executable target formation #
###############################

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

target_link_libraries("${EXEC_NAME}" INTERFACE "${PROJECT_NAME}_compiler_flags")

include(GNUInstallDirs)
target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

#target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBRARY_NAME}")

set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

#######################################
# Export / installation configuration #
#######################################

set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
#generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

# setup the version numbering
set_property(TARGET "${EXEC_NAME}" PROPERTY VERSION "${${PROJECT_NAME}_VERSION}")
set_property(TARGET "${EXEC_NAME}" PROPERTY SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}")

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

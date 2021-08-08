get_filename_component(EXEC_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" EXEC_NAME "${EXEC_NAME}")

set(Header_Files "cli.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "cli.c" "main.c")
source_group("Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

target_link_libraries("${EXEC_NAME}" PUBLIC "${PROJECT_LOWER_NAME}_compiler_flags")

target_link_libraries("${EXEC_NAME}" PUBLIC "acquire")

set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

# setup the version numbering
set_property(TARGET "${EXEC_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${EXEC_NAME}" PROPERTY SOVERSION "1")

# install rules
set(installable_libs "${EXEC_NAME}" "${PROJECT_LOWER_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "bin"
        EXPORT "${EXEC_NAME}Targets")

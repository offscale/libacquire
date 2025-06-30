get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(LIBRARY_NAME "${PROJECT_NAME}_${LIBRARY_NAME}")

set(Source_Files "download_example.c")
source_group("Source Files" FILES "${Source_Files}")

add_executable("${LIBRARY_NAME}" "${Source_Files}")

target_link_libraries("${LIBRARY_NAME}" PRIVATE "${PROJECT_NAME}")

target_include_directories("${LIBRARY_NAME}" PRIVATE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/acquire>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
)

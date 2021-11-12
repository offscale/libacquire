#[=======================================================================[

get_libacquire_header
---------------------

Function to download the amalgamated header from GitHub.

Usage
^^^^^

Arguments:

header_file
    Where to download the header file to

]=======================================================================]

function (get_libacquire_header header_file)
    set(json_file "${CMAKE_BINARY_DIR}/libacquire.json")

    if (EXISTS "${header_file}")
        string(LENGTH "${CMAKE_SOURCE_DIR}" source_dir_length)
        math(EXPR source_dir_length "${source_dir_length} + 1")
        string(SUBSTRING "${header_file}" "${source_dir_length}" -1 header_file_relative)
        message(STATUS "\"${header_file_relative}\" exists; skipping download.")
        return()
    elseif (NOT EXISTS "${json_file}")
        file(DOWNLOAD https://api.github.com/repos/offscale/libacquire/releases "${json_file}"
                HTTPHEADER "Accept: application/vnd.github.v3+json")
    endif ()

    string(LENGTH "${json_file}" json_filesize)
    if (json_filesize EQUAL 0)
        message(FATAL_ERROR "Unable to download \"${json_file}\"")
    endif ()

    file(READ "${json_file}" json_contents)
    string(JSON json_contents_n
            LENGTH "${json_contents}")
    math(EXPR json_contents_n "${json_contents_n} - 1")

    foreach (i RANGE 0 "${json_contents_n}")

        string(JSON type TYPE "${json_contents}" "${i}")
        if (NOT type STREQUAL "OBJECT")
            continue ()
        endif ()

        string(JSON type TYPE "${json_contents}" "${i}" "assets")
        if (NOT type STREQUAL "ARRAY")
            continue ()
        endif ()

        string(JSON assets_json
                GET "${json_contents}" "${i}" "assets")

        string(JSON assets_json_n
                LENGTH "${assets_json}")
        math(EXPR assets_json_n "${assets_json_n} - 1")

        foreach (j RANGE 0 "${assets_json_n}")
            string(JSON asset_json
                    GET "${assets_json}" "${j}")

            string(JSON type TYPE "${asset_json}")
            if (NOT type STREQUAL "OBJECT")
                continue ()
            endif ()

            string(JSON content_type
                    GET "${asset_json}" "content_type")
            string(JSON name
                    GET "${asset_json}" "name")

            if (name STREQUAL "acquire.h")
                string(JSON browser_download_url
                        GET "${asset_json}" "browser_download_url"
                        )
                file(DOWNLOAD "${browser_download_url}" "${header_file}")
                return()
            endif ()
        endforeach ()
    endforeach ()

    message(FATAL_ERROR "Unable to find downloadable \"acquire.h\"")
endfunction (get_libacquire_header)

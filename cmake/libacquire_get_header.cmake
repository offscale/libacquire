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
        return()
    elseif (NOT EXISTS "${json_file}")
        file(DOWNLOAD https://api.github.com/repos/offscale/libacquire/releases "${json_file}"
                HTTPHEADER "Accept: application/vnd.github.v3+json")
    endif ()
    file(READ "${json_file}" json_contents)

    string(JSON obj_json_n
            LENGTH "${json_contents}")
    math(EXPR obj_json_n "${obj_json_n} - 1")

    foreach (i RANGE 0 "${obj_json_n}")
        string(JSON assets_json
                GET "${json_contents}" "${i}" "assets")

        string(JSON assets_json_n
                LENGTH "${assets_json}")
        math(EXPR asset_json_n "${assets_json_n} - 1")

        foreach (j RANGE 0 "${assets_json_n}")
            string(JSON asset_json
                    GET "${json_contents}" "${i}" "assets" "${j}")
            string(JSON asset_name
                    GET "${asset_json}" "name")
            if (asset_name STREQUAL "acquire_amalgamation.h")
                string(JSON browser_download_url
                        GET "${asset_json}" "browser_download_url"
                        )
                file(DOWNLOAD "${browser_download_url}" "${header_file}")
                return()
            endif ()
        endforeach ()
    endforeach ()
endfunction (get_libacquire_header)

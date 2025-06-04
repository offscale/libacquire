# FindLibBSD.cmake
#
# Find libbsd, a compatibility library providing BSD functions on other platforms
#
# Defines:
#   LibBSD_FOUND
#   LibBSD_INCLUDE_DIRS
#   LibBSD_LIBRARIES
#
# Provides imported target:
#   LibBSD::LibBSD

cmake_policy(SET CMP0074 NEW)  # Use CMAKE_PREFIX_PATH in find_package()

unset(LibBSD_FOUND CACHE)
unset(LibBSD_INCLUDE_DIRS CACHE)
unset(LibBSD_LIBRARIES CACHE)
unset(LibBSD_IMPORTED_TARGET_CREATED CACHE)

# Search hint prefixes — add your custom install prefix by setting LIBBSD_ROOT environment variable
set(_LIBBSD_HINTS
        "$ENV{LIBBSD_ROOT}"
        "/usr/local"
        "/usr/lib"
        "/opt/local"
        "/opt"
        "/usr"
        "/usr/local/include"  # Homebrew macOS common prefix
        "/usr/include"
)

# Remove empty and duplicate entries
list(FILTER _LIBBSD_HINTS EXCLUDE REGEX "^$")
list(REMOVE_DUPLICATES _LIBBSD_HINTS)

# Remove project source directory to avoid matching your own bsd folder in source tree
if (DEFINED CMAKE_SOURCE_DIR)
    list(FILTER _LIBBSD_HINTS EXCLUDE REGEX "^${CMAKE_SOURCE_DIR}$")
endif ()

function(_libbsd_create_imported_target)
    if (NOT TARGET LibBSD::LibBSD)
        add_library(LibBSD::LibBSD UNKNOWN IMPORTED)
        set_target_properties(LibBSD::LibBSD PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${LibBSD_INCLUDE_DIRS}"
                IMPORTED_LOCATION "${LibBSD_LIBRARIES}"
        )
    endif ()
    set(LibBSD_IMPORTED_TARGET_CREATED TRUE PARENT_SCOPE)
endfunction()

# Function: resolve linker names like "bsd" into absolute library paths to avoid Ninja errors
function(_libbsd_resolve_libraries input_libs output_var)
    set(_resolved_libs)
    separate_arguments(_libs UNIX_COMMAND "${input_libs}")

    foreach (_lib IN LISTS _libs)
        string(REGEX REPLACE "^-l" "" _libname "${_lib}")

        set(_found_path "")

        # First attempt: search in pkgconfig paths without NO_DEFAULT_PATH to allow /usr/local/lib lookup
        if (DEFINED LIBBSD_LIBRARY_DIRS AND LIBBSD_LIBRARY_DIRS)
            find_library(_found_path NAMES ${_libname}
                    PATHS ${LIBBSD_LIBRARY_DIRS}
                    NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_CMAKE_SYSTEM_PATH
            )
        endif ()

        # If not found, try default system search paths without restrictions
        if (NOT _found_path)
            find_library(_found_path NAMES ${_libname})
        endif ()

        # If still not found, try to sanitize common library extensions manually
        if (NOT _found_path)
            if (NOT DEFINED TARGET_ARCH)
                include("${CMAKE_SOURCE_DIR}/cmake/get_arch.cmake")
                get_arch()
            endif (NOT DEFINED TARGET_ARCH)
            string(TOLOWER TARGET_ARCH arch)

            # try known install path hardcoded — add your known lib path:
            foreach (p
                    "/usr/local/lib/lib${_libname}.dylib"
                    "/usr/local/lib/lib${_libname}.a"
                    "/usr/lib/${arch}-linux-gnu/libbsd.so"
                    "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/libbsd.so"
                    "/usr/lib/x86_64-linux-gnu/libbsd.so"
                    "/usr/lib/aarch64-linux-gnu/libbsd.so"
                    "/usr/lib/arm-linux-gnueabihf/libbsd.so"
                    "/usr/lib/i386-linux-gnu/libbsd.so"
                    "/usr/lib/libbsd.so")
                if (EXISTS "${p}" AND NOT IS_DIRECTORY "${p}")
                    set(_found_path "${p}")
                endif (EXISTS "${p}" AND NOT IS_DIRECTORY "${p}")
            endforeach (p
                    "/usr/local/lib/lib${_libname}.dylib"
                    "/usr/local/lib/lib${_libname}.a"
                    "/usr/lib/${arch}-linux-gnu/libbsd.so"
                    "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/libbsd.so"
                    "/usr/lib/x86_64-linux-gnu/libbsd.so"
                    "/usr/lib/aarch64-linux-gnu/libbsd.so"
                    "/usr/lib/arm-linux-gnueabihf/libbsd.so"
                    "/usr/lib/i386-linux-gnu/libbsd.so"
                    "/usr/lib/libbsd.so")
        endif ()

        if (_found_path)
            list(APPEND _resolved_libs "${_found_path}")
        else ()
            list(APPEND _resolved_libs "${_lib}")
            message(WARNING "FindLibBSD: Failed to resolve absolute path for library: ${_lib}")
        endif ()
    endforeach ()
    set(${output_var} "${_resolved_libs}" PARENT_SCOPE)
endfunction()

# --- Attempt pkg-config detection first ---

find_package(PkgConfig QUIET)

if (PkgConfig_FOUND)
    # Prepare / prepend PKG_CONFIG_PATH with hints pkgconfig dirs
    set(_pkgconfig_paths)
    foreach (_hint IN LISTS _LIBBSD_HINTS)
        foreach (_subdir IN ITEMS "lib/pkgconfig" "lib64/pkgconfig" "share/pkgconfig")
            if (_hint AND IS_DIRECTORY "${_hint}/${_subdir}")
                list(APPEND _pkgconfig_paths "${_hint}/${_subdir}")
            endif ()
        endforeach ()
    endforeach ()
    list(REMOVE_DUPLICATES _pkgconfig_paths)

    if (WIN32)
        set(_path_sep ";")
    else ()
        set(_path_sep " :")
    endif ()

    if (_pkgconfig_paths)
        list(JOIN _pkgconfig_paths "${_path_sep}" _joined_pcp)
        if (DEFINED ENV{PKG_CONFIG_PATH} AND NOT "$ENV{PKG_CONFIG_PATH}" STREQUAL "")
            set(ENV{PKG_CONFIG_PATH} "${_joined_pcp}${_path_sep}$ENV{PKG_CONFIG_PATH}")
        else ()
            set(ENV{PKG_CONFIG_PATH} "${_joined_pcp}")
        endif ()
    endif ()

    pkg_check_modules(LIBBSD libbsd)

    if (LIBBSD_FOUND)
        # Resolve libraries to absolute paths
        _libbsd_resolve_libraries("${LIBBSD_LIBRARIES}" LibBSD_LIBRARIES)

        set(LibBSD_FOUND TRUE CACHE BOOL "LibBSD found via pkg-config")
        set(LibBSD_INCLUDE_DIRS ${LIBBSD_INCLUDE_DIRS} CACHE STRING "LibBSD include dirs")
        set(LibBSD_LIBRARIES "${LibBSD_LIBRARIES}" CACHE STRING "LibBSD libraries")

        # Normalize include dirs
        set(_abs_inc_dirs)
        foreach (dir IN LISTS LibBSD_INCLUDE_DIRS)
            get_filename_component(abs_dir "${dir}" ABSOLUTE)
            list(APPEND _abs_inc_dirs "${abs_dir}")
        endforeach ()
        list(REMOVE_DUPLICATES _abs_inc_dirs)
        set(LibBSD_INCLUDE_DIRS "${_abs_inc_dirs}" CACHE INTERNAL "")

        _libbsd_create_imported_target()

        message(STATUS "FindLibBSD: Found via pkg-config")
        message(STATUS " Include dirs: ${LibBSD_INCLUDE_DIRS}")
        message(STATUS " Libraries: ${LibBSD_LIBRARIES}")
    endif ()
endif ()

# --- Manual fallback if pkg-config fails ---

if (NOT LibBSD_FOUND)
    # Try to find unique header: prefer libbsd.h
    find_path(LibBSD_INCLUDE_DIRS
            NAMES libbsd.h
            PATHS ${_LIBBSD_HINTS}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_CMAKE_SYSTEM_PATH
    )

    # Optional fallback: less reliable and may pick local bsd/
    if (NOT LibBSD_INCLUDE_DIRS)
        find_path(LibBSD_INCLUDE_DIRS
                NAMES bsd/string.h
                PATHS ${_LIBBSD_HINTS}
                PATH_SUFFIXES include
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_CMAKE_SYSTEM_PATH
        )
    endif ()

    find_library(LibBSD_LIBRARIES
            NAMES bsd
            PATHS ${_LIBBSD_HINTS}
            PATH_SUFFIXES lib lib64
            NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_CMAKE_SYSTEM_PATH
    )

    if (LibBSD_INCLUDE_DIRS AND LibBSD_LIBRARIES)
        get_filename_component(LibBSD_INCLUDE_DIRS_ABS "${LibBSD_INCLUDE_DIRS}" ABSOLUTE)
        set(LibBSD_INCLUDE_DIRS "${LibBSD_INCLUDE_DIRS_ABS}" CACHE INTERNAL "")
        get_filename_component(LibBSD_LIBRARIES_ABS "${LibBSD_LIBRARIES}" ABSOLUTE)
        set(LibBSD_LIBRARIES "${LibBSD_LIBRARIES_ABS}" CACHE INTERNAL "")

        set(LibBSD_FOUND TRUE CACHE BOOL "LibBSD found via fallback")

        _libbsd_create_imported_target()

        message(STATUS "FindLibBSD: Found via manual fallback")
        message(STATUS " Include dirs: ${LibBSD_INCLUDE_DIRS}")
        message(STATUS " Libraries: ${LibBSD_LIBRARIES}")
    else ()
        set(LibBSD_FOUND FALSE CACHE BOOL "LibBSD found")
        message(STATUS "FindLibBSD: LibBSD not found")
    endif ()
endif ()

mark_as_advanced(LibBSD_FOUND LibBSD_INCLUDE_DIRS LibBSD_LIBRARIES)

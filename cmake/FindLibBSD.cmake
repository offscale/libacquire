# Finds the libbsd library, a compatibility library providing BSD functions.
#
# Defines:
#   LibBSD_FOUND          - True if libbsd was found.
#   LibBSD_INCLUDE_DIRS   - The include directories for libbsd.
#   LibBSD_LIBRARIES      - The libraries to link against for libbsd.
#
# Provides Imported Target:
#   LibBSD::LibBSD        - The imported target for libbsd.

cmake_policy(SET CMP0074 NEW)

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(PC_LibBSD QUIET libbsd)
endif ()

find_path(LibBSD_INCLUDE_DIR
        NAMES bsd/string.h libbsd.h
        HINTS ${PC_LibBSD_INCLUDE_HINTS} ${PC_LibBSD_INCLUDEDIR}
)

find_library(LibBSD_LIBRARY
        NAMES bsd
        HINTS ${PC_LibBSD_LIBRARY_HINTS} ${PC_LibBSD_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibBSD
        FOUND_VAR LibBSD_FOUND
        REQUIRED_VARS LibBSD_LIBRARY LibBSD_INCLUDE_DIR
)

if (LibBSD_FOUND)
    set(LibBSD_LIBRARIES ${LibBSD_LIBRARY})
    set(LibBSD_INCLUDE_DIRS ${LibBSD_INCLUDE_DIR})

    if (NOT TARGET LibBSD::LibBSD)
        add_library(LibBSD::LibBSD UNKNOWN IMPORTED)
        set_target_properties(LibBSD::LibBSD PROPERTIES
                IMPORTED_LOCATION "${LibBSD_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LibBSD_INCLUDE_DIR}"
        )
    endif (NOT TARGET LibBSD::LibBSD)
endif (LibBSD_FOUND)

mark_as_advanced(LibBSD_INCLUDE_DIR LibBSD_LIBRARY)

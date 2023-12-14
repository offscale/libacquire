# Find LibBSD compat lib
#
# Once done, this will define:
#
#  LibBSD_FOUND - system has LibBSD compat
#  LibBSD_INCLUDE_DIRS - the LibBSD compat include directories
#  LibBSD_LIBRARIES - link these to use LibBSD compat
#

if (DEFINED LibBSD_FOUND)
    return()
else ()
    set(LibBSD_FOUND 0)
endif (DEFINED LibBSD_FOUND)

include(FindPkgConfig)
find_package(PkgConfig)
if (PkgConfig_FOUND)
    set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

    pkg_check_modules(LibBSD libbsd REQUIRED)
    if (LibBSD_FOUND)
        if (NOT LibBSD_INCLUDE_DIRS)
            set(LibBSD_INCLUDE_DIRS "${LibBSD_INCLUDEDIR}")
        endif (NOT LibBSD_INCLUDE_DIRS)

        add_library(LibBSD SHARED IMPORTED)
        set_target_properties(LibBSD PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${LibBSD_INCLUDE_DIRS}")
        if (WIN32)
            set_target_properties(LibBSD PROPERTIES
                    IMPORTED_IMPLIB "${LibBSD_LINK_LIBRARIES}")
        else ()
            set_target_properties(LibBSD PROPERTIES
                    IMPORTED_LOCATION "${LibBSD_LINK_LIBRARIES}")
        endif ()
    endif ()
endif ()

#[=======================================================================[

FindLibArchiveCustom
--------------------

Find the LibArchive library. Falls back to pkg-config (if not found through `find_package`).

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following imported target:

LibArchive
    The LibArchive library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

LIBARCHIVE_FOUND
    System has the LibArchive library.
LibArchive_INCLUDE_DIRS
    The LibArchive include directory.
LIBARCHIVE_LIBRARY
    The LibArchive library, like:
      /Applications/Xcode.app/Contents/Developer/Platforms/
       MacOSX.platform/Developer/SDKs/MacOSX12.0.sdk/usr/lib/libarchive.tbd
LIBARCHIVE_LIBRARIES
    "archive"
LIBARCHIVE_VERSION
    This is set to $major.$minor.$revision (e.g. 2.6.8).

With pkg-config, you might additionally get vars like these set:

LibArchive_CFLAGS=-I/usr/local/Cellar/libarchive/3.5.2/include
LibArchive_INCLUDEDIR=/usr/local/Cellar/libarchive/3.5.2/include
LibArchive_INCLUDE_DIRS=/usr/local/Cellar/libarchive/3.5.2/include
LibArchive_LDFLAGS=-L/usr/local/Cellar/libarchive/3.5.2/lib;-larchive
LibArchive_LIBDIR=/usr/local/Cellar/libarchive/3.5.2/lib
LibArchive_LIBRARY_DIRS=/usr/local/Cellar/libarchive/3.5.2/lib
LibArchive_LINK_LIBRARIES=/usr/local/Cellar/libarchive/3.5.2/lib/libarchive.dylib
LibArchive_MODULE_NAME=LibArchive
LibArchive_PREFIX=/usr/local/Cellar/libarchive/3.5.2
LibArchive_STATIC_CFLAGS=-I/usr/local/Cellar/libarchive/3.5.2/include
LibArchive_STATIC_INCLUDE_DIRS=/usr/local/Cellar/libarchive/3.5.2/include
LibArchive_STATIC_LDFLAGS=-L/usr/local/Cellar/libarchive/3.5.2/lib;-larchive;-lexpat;-llzma;-lzstd;-llz4;-lb2;-lbz2;-lz
LibArchive_STATIC_LIBRARIES=archive;expat;lzma;zstd;lz4;b2;bz2;z
LibArchive_STATIC_LIBRARY_DIRS=/usr/local/Cellar/libarchive/3.5.2/lib

]=======================================================================]


find_package(LibArchive QUIET)
if (NOT LibArchive_FOUND)
    find_package(PkgConfig)
    if (PkgConfig_FOUND)
        set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
        pkg_search_module(LibArchive LibArchive)
    endif (PkgConfig_FOUND)
endif (NOT LibArchive_FOUND)

if (NOT LibArchive_FOUND AND NOT DEFINED LibArchive_QUIET)
    message(FATAL_ERROR "Could NOT find LibArchive")
endif (NOT LibArchive_FOUND AND NOT DEFINED LibArchive_QUIET)

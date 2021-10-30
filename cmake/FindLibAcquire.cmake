#[=======================================================================[

FindLibAcquire
--------------

Finds the dependencies that libacquire relies on (libacquire itself is header-only)

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

LIBACQUIRE_FOUND
    System has any of the libraries that libacquire depends on.
LIBACQUIRE_INCLUDE_DIR
    Include directories of any of the libraries that libacquire depends on. Use with `target_include_directories`.
LIBACQUIRE_LIBRARIES
    Libraries that libacquire depends on. Use `target_link_libraries` to link.

]=======================================================================]

if (DEFINED USE_LIBCURL AND DEFINED CURL_LINK_LIBRARIES)
    set(LIBACQUIRE_LIBRARIES "${CURL_LINK_LIBRARIES}")
elseif (DEFINED USE_WININET)
    set(LIBACQUIRE_LIBRARIES "wininet")
    list(APPEND LIBACQUIRE_LIBRARIES "crypt32")  # USE_WINCRYPT
elseif (DEFINED USE_MY_LIBFETCH)
    set(LIBACQUIRE_LIBRARIES "freebsd_libfetch")
elseif (DEFINED USE_LIBFETCH)
    set(APPEND LIBACQUIRE_LIBRARIES "fetch")
endif ()

if (DEFINED OPENSSL_LIBRARIES)
    list(APPEND LIBACQUIRE_LIBRARIES "OpenSSL::SSL")
    list(APPEND LIBACQUIRE_LIBRARIES "${OPENSSL_LIBRARIES}")
    set(LIBACQUIRE_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}")
endif ()

if (NOT DEFINED LIBACQUIRE_LIBRARIES)
    message(FATAL_ERROR "At least one network library must be specified for linkage")
endif ()

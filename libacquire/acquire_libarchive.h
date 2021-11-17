#if !defined(LIBACQUIRE_ACQUIRE_LIBARCHIVE_H) && defined(USE_LIBARCHIVE) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_ACQUIRE_LIBARCHIVE_H

#include "acquire_extract.h"
#include <sys/types.h>

#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int extract_archive(enum Archive archive, const char *archive_filepath, const char *output_folder) {
#define BUF_SIZE 16384
    FILE *fp;
    int exit_code;
    int r;
    ssize_t size;
    static char buff[BUF_SIZE];
    struct archive *a;
    struct archive_entry *ae;
    puts("acquire_libarchive.h");
    if (archive == LIBACQUIRE_UNSUPPORTED_ARCHIVE) return EXIT_FAILURE;
    /*
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    fopen_s(&fp, filename, "rb");
#else
    fp = fopen(archive_filepath, "rb");
#endif */ /* defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ */


    a = archive_read_new();
    archive_read_support_filter_all(a);
    /* archive_read_support_compression_all(a); */
    archive_read_support_format_raw(a);
    r = archive_read_open_filename(a, archive_filepath, BUF_SIZE);
    if (r != ARCHIVE_OK) {
        /* ERROR */
    }
    r = archive_read_next_header(a, &ae);
    if (r != ARCHIVE_OK) {
        /* ERROR */
    }

    for (;;) {
        size = archive_read_data(a, buff, BUF_SIZE);
        if (size < 0) {
            /* ERROR */
        }
        if (size == 0)
            break;
        write(1, buff, size);
    }

    archive_read_free(a);
#undef BUF_SIZE
    return EXIT_SUCCESS;
}

#endif /* !defined(LIBACQUIRE_ACQUIRE_LIBARCHIVE_H) && defined(USE_LIBARCHIVE) && defined(LIBACQUIRE_IMPLEMENTATION) */

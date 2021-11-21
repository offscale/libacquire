#if !defined(LIBACQUIRE_ACQUIRE_LIBARCHIVE_H) && defined(USE_LIBARCHIVE) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_ACQUIRE_LIBARCHIVE_H

/* Note this file is mostly the https://github.com/libarchive/libarchive/wiki/Examples#a-complete-extractor */

#include "acquire_extract.h"

#include <archive.h>
#include <archive_entry.h>

static la_ssize_t
copy_data(struct archive *ar, struct archive *aw)
{
    la_ssize_t r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return ARCHIVE_OK;
        if (r < ARCHIVE_OK)
            return r;
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fputs(archive_error_string(aw), stderr);
            return r;
        }
    }
}

int extract_archive(enum Archive archive, const char *archive_filepath, const char *output_folder) {
    if (archive == LIBACQUIRE_UNSUPPORTED_ARCHIVE) return EXIT_FAILURE;
    {
        struct archive *a;
        struct archive *ext;
        struct archive_entry *entry;
        int flags;
        la_ssize_t r;

        /* Select which attributes we want to restore. */
        flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;

        a = archive_read_new();
        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);
        ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, flags);
        archive_write_disk_set_standard_lookup(ext);
        if ((r = archive_read_open_filename(a, archive_filepath, 10240)))
            return EXIT_FAILURE;
        for (;;) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF)
                break;
            if (r < ARCHIVE_OK)
                fputs(archive_error_string(a), stderr);
            if (r < ARCHIVE_WARN)
                return EXIT_FAILURE;
            r = archive_write_header(ext, entry);
            if (r < ARCHIVE_OK)
                fputs(archive_error_string(ext), stderr);
            else if (archive_entry_size(entry) > 0) {
                r = copy_data(a, ext);
                if (r < ARCHIVE_OK)
                    fputs(archive_error_string(ext), stderr);
                if (r < ARCHIVE_WARN)
                    return EXIT_FAILURE;
            }
            r = archive_write_finish_entry(ext);
            if (r < ARCHIVE_OK)
                fputs(archive_error_string(ext), stderr);
            if (r < ARCHIVE_WARN)
                return EXIT_FAILURE;
        }
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
    }
    return EXIT_SUCCESS;
}

#endif /* !defined(LIBACQUIRE_ACQUIRE_LIBARCHIVE_H) && defined(USE_LIBARCHIVE) && defined(LIBACQUIRE_IMPLEMENTATION) */

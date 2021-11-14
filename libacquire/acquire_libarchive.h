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

/*
 * This file is in the public domain.  Use it as you see fit.
 */

/*
 * "untar" is an extremely simple tar extractor:
 *  * A single C source file, so it should be easy to compile
 *    and run on any system with a C compiler.
 *  * Extremely portable standard C.  The only non-ANSI function
 *    used is mkdir().
 *  * Reads basic ustar tar archives.
 *  * Does not require libarchive or any other special library.
 *
 * To compile: cc -o untar untar.c
 *
 * Usage:  untar <archive>
 *
 * In particular, this program should be sufficient to extract the
 * distribution for libarchive, allowing people to bootstrap
 * libarchive on systems that do not already have a tar program.
 *
 * To unpack libarchive-x.y.z.tar.gz:
 *    * gunzip libarchive-x.y.z.tar.gz
 *    * untar libarchive-x.y.z.tar
 *
 * Written by Tim Kientzle, March 2009.
 *
 * Released into the public domain.
 */

/* These are all highly standard and portable headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This is for mkdir(); this may need to be changed for some platforms. */
#include <sys/stat.h>  /* For mkdir() */
#include <errno.h>

#ifdef _MSC_VER
#define NUM_FORMAT "zu"
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define NUM_FORMAT "d"
#else
#define NUM_FORMAT "lu"
#endif /* _MSC_VER */

#define TO_STRING(x) #x
#define STR(x) TO_STRING(x)

/* Parse an octal number, ignoring leading and trailing nonsense. */
static int
parseoct(const char *p, size_t n)
{
    int i = 0;

    while ((*p < '0' || *p > '7') && n > 0) {
        ++p;
        --n;
    }
    while (*p >= '0' && *p <= '7' && n > 0) {
        i *= 8;
        i += *p - '0';
        ++p;
        --n;
    }
    return (i);
}

/* Returns true if this is 512 zero bytes. */
static int
is_end_of_archive(const char *p)
{
    int n;
    for (n = 511; n >= 0; --n)
        if (p[n] != '\0')
            return (0);
    return (1);
}

/* Create a directory, including parent directories as necessary. */
static void
create_dir(char *pathname, int mode)
{
    char *p;
    int r;

    /* Strip trailing '/' */
    if (pathname[strlen(pathname) - 1] == '/')
        pathname[strlen(pathname) - 1] = '\0';

    /* Try creating the directory. */
    r = mkdir(pathname, mode);

    if (r != 0) {
        /* On failure, try creating parent directory. */
        p = strrchr(pathname, '/');
        if (p != NULL) {
            *p = '\0';
            create_dir(pathname, 0755);
            *p = '/';
            r = mkdir(pathname, mode);
        }
    }
    if (r != 0)
        fprintf(stderr, "Could not create directory %s\n", pathname);
}

/* Create a file, including parent directory as necessary. */
static FILE *
create_file(char *pathname, int mode)
{
    FILE *f;
    f = fopen(pathname, "wb+");
    if (f == NULL) {
        /* Try creating parent dir and then creating file. */
        char *p = strrchr(pathname, '/');
        if (p != NULL) {
            *p = '\0';
            create_dir(pathname, 0755);
            *p = '/';
            f = fopen(pathname, "wb+");
        }
    }
    return (f);
}

/* Verify the tar checksum. */
static int
verify_checksum(const char *p)
{
    int n, u = 0;
    for (n = 0; n < 512; ++n) {
        if (n < 148 || n > 155)
            /* Standard tar checksum adds unsigned bytes. */
            u += ((unsigned char *)p)[n];
        else
            u += 0x20;

    }
    return (u == parseoct(p + 148, 8));
}

/* Extract an archive. */
int
extract_file(FILE *a, const char *path)
{
#define BUF_SIZE 512
    char buff[BUF_SIZE];
    FILE *f = NULL;
    size_t bytes_read;
    off_t filesize;
    int exit_code = EXIT_SUCCESS;

    printf("Extracting from %s\n", path);
    for (;;) {
        bytes_read = fread(buff, 1, BUF_SIZE, a);
        if (bytes_read < BUF_SIZE) {
            fprintf(stderr,
                    "Short read on %s: expected " STR(BUF_SIZE) ", got %"NUM_FORMAT"\n",
                    path, bytes_read);
            exit_code = EXIT_FAILURE;
            goto cleanup;
        }
        if (is_end_of_archive(buff)) {
            printf("End of %s\n", path);
            exit_code = EOF;
            goto cleanup;
        }
        if (!verify_checksum(buff)) {
            fprintf(stderr, "Checksum failure\n");
            exit_code = EXIT_FAILURE;
            goto cleanup;
        }
        filesize = parseoct(buff + 124, 12);
        switch (buff[156]) {
            case '1':
                printf(" Ignoring hardlink %s\n", buff);
                break;
            case '2':
                printf(" Ignoring symlink %s\n", buff);
                break;
            case '3':
                printf(" Ignoring character device %s\n", buff);
                break;
            case '4':
                printf(" Ignoring block device %s\n", buff);
                break;
            case '5':
                printf(" Extracting dir %s\n", buff);
                create_dir(buff, parseoct(buff + 100, 8));
                filesize = 0;
                break;
            case '6':
                printf(" Ignoring FIFO %s\n", buff);
                break;
            default:
                printf(" Extracting file %s\n", buff);
                f = create_file(buff, parseoct(buff + 100, 8));
                break;
        }
        while (filesize > 0) {
            bytes_read = fread(buff, 1, BUF_SIZE, a);
            if (bytes_read < BUF_SIZE) {
                fprintf(stderr,
                        "Short read on %s: Expected " STR(BUF_SIZE) ", got %"NUM_FORMAT"\n",
                        path, bytes_read);
                exit_code = EXIT_FAILURE;
                goto cleanup;
            }
            if (filesize < BUF_SIZE)
                bytes_read = filesize;
            if (f != NULL) {
                if (fwrite(buff, 1, bytes_read, f)
                    != bytes_read)
                {
                    fprintf(stderr, "Failed write\n");
                    fclose(f);
                    f = NULL;
                }
            }
            filesize -= (off_t)bytes_read;
        }
        if (f != NULL) {
            fclose(f);
            f = NULL;
        }
    }
cleanup:
    if (f != NULL) {
        fclose(f);
        f = NULL;
    }

    return exit_code;
#undef BUF_SIZE
}

/*{
        a = fopen(*argv, "rb");
        if (a == NULL)
            fprintf(stderr, "Unable to open %s\n", *argv);
        else {
            extract_file(a, *argv);
            fclose(a);
        }
    }*/


int extract_archive(enum Archive archive, const char *archive_filepath, const char *output_folder) {
    FILE *fp;
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    fopen_s(&fp, filename, "r");
#else
    fp = fopen(archive_filepath, "r");
#endif /* defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ */

    if(!fp) {
        fprintf(stderr, "Could not open %s: %s", archive_filepath, strerror(errno));
        return ENOENT;
    }
    extract_file(fp, output_folder);
    return EXIT_SUCCESS;
}

int _extract_archive(enum Archive archive, const char *archive_filepath, const char *output_folder) {
#define BLOCK_SIZE 16384
    int r;
    ssize_t size;
    static char buff[BLOCK_SIZE];
    struct archive_entry *ae;

    struct archive *a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_raw(a);
    r = archive_read_open_filename(a, archive_filepath, BLOCK_SIZE);
    if (r != ARCHIVE_OK) {
        /* ERROR */
        return r;
    }
    r = archive_read_next_header(a, &ae);
    if (r != ARCHIVE_OK) {
        /* ERROR */
        return r;
    }

    for (;;) {
        size = archive_read_data(a, buff, BLOCK_SIZE);
        if (size < 0) {
            /* ERROR */
            return EOF;
        }
        if (size == 0)
            break;
        write(1, buff, size);
    }

    archive_read_free(a);
    return EXIT_SUCCESS;
#undef BLOCK_SIZE
}

#endif /* !defined(LIBACQUIRE_ACQUIRE_LIBARCHIVE_H) && defined(USE_LIBARCHIVE) && defined(LIBACQUIRE_IMPLEMENTATION) */

/*
 * WiP, based off https://nachtimwald.com/2019/09/08/making-minizip-easier-to-use
 */

#if !defined(LIBACQUIRE_ACQUIRE_ZLIB_H) && defined(USE_ZLIB) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_ACQUIRE_ZLIB_H

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minizip/unzip.h>
#include <minizip/zip.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <minizip/iowin32.h>
#endif

#define BUF_SIZE 8192
#define MAX_NAMELEN 256

enum zipper_result_t {
    ZIPPER_RESULT_ERROR = 0,
    ZIPPER_RESULT_SUCCESS,
    ZIPPER_RESULT_SUCCESS_EOF
};

typedef void (*zipper_read_cb_t)(const unsigned char *buf, size_t size, void *thunk);

extern bool zipper_add_file(zipFile zfile, const char *filename);
extern bool zipper_add_buf(zipFile zfile, const char *zfilename, const unsigned char *buf, size_t buflen);
extern bool zipper_add_dir(zipFile zfile, const char *dirname);

extern enum zipper_result_t zipper_read(unzFile zfile, zipper_read_cb_t cb, void *thunk);
extern enum zipper_result_t zipper_read_buf(unzFile zfile, unsigned char **buf, size_t *buflen);

extern bool zipper_skip_file(unzFile zfile);
extern char *zipper_filename(unzFile zfile, bool *isutf8);
extern bool zipper_isdir(unzFile zfile);
extern uint64_t zipper_filesize(unzFile zfile);


zipper_result_t zipper_read(unzFile zfile, zipper_read_cb_t cb, void *thunk)
{
    unsigned char tbuf[BUF_SIZE];
    int           red;
    int           ret;

    if (zfile == NULL || cb == NULL)
        return ZIPPER_RESULT_ERROR;

    ret = unzOpenCurrentFile(zfile);
    if (ret != UNZ_OK)
        return ZIPPER_RESULT_ERROR;

    while ((red = unzReadCurrentFile(zfile, tbuf, sizeof(tbuf))) > 0) {
        cb(tbuf, red, thunk);
    }

    if (red < 0) {
        unzCloseCurrentFile(zfile);
        return ZIPPER_RESULT_ERROR;
    }

    unzCloseCurrentFile(zfile);
    if (unzGoToNextFile(zfile) != UNZ_OK)
        return ZIPPER_RESULT_SUCCESS_EOF;
    return ZIPPER_RESULT_SUCCESS;
}

static void zipper_read_buf_cb(const unsigned char *buf, size_t buflen, void *thunk)
{
    str_builder_t *sb = thunk;
    str_builder_add_str(sb, (const char *)buf, buflen);
}

zipper_result_t zipper_read_buf(unzFile zfile, unsigned char **buf, size_t *buflen)
{
    str_builder_t   *sb;
    zipper_result_t  ret;

    sb = str_builder_create();
    ret = zipper_read(zfile, zipper_read_buf_cb, sb);
    if (ret != ZIPPER_RESULT_ERROR)
        *buf = (unsigned char *)str_builder_dump(sb, buflen);
    str_builder_destroy(sb);
    return ret;
}

#endif /* !defined(LIBACQUIRE_ACQUIRE_ZLIB_H) && defined(USE_ZLIB) && defined(LIBACQUIRE_IMPLEMENTATION) */

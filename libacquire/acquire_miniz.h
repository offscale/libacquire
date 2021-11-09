#if !defined(LIBACQUIRE_ACQUIRE_MINIZ_H) && defined(USE_MINIZ) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_ACQUIRE_MINIZ_H

#include <stdlib.h>
#include <stdio.h>

#include "acquire_extract.h"
#include "acquire_errors.h"

int extract_archive(enum Archive archive, const char *archive_filepath, const char *output_folder) {
    FILE *fp;

    if (archive != LIBACQUIRE_ZIP) return UNIMPLEMENTED;
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    fopen_s(&fp, fname, "rb");
#else
    fp = fopen(archive_filepath, "rb");
#endif

    fclose(fp);

    return EXIT_SUCCESS;
}

/* zip_extract_file(zip* z, unsigned index, FILE *out) */

#endif /* !defined(LIBACQUIRE_ACQUIRE_MINIZ_H) && defined(USE_MINIZ) && defined(LIBACQUIRE_IMPLEMENTATION) */

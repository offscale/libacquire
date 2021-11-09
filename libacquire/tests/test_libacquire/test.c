#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <greatest.h>

#include "test_checksum.h"
#include "test_download.h"
#include "test_fileutils.h"
#include "test_string_extras.h"
#include "test_miniz.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(checksums_suite);
    RUN_SUITE(fileutils_suite);
    RUN_SUITE(strnstr_suite);
    RUN_SUITE(downloads_suite);
    RUN_SUITE(miniz_suite);
    GREATEST_MAIN_END();
}

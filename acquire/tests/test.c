#include <greatest.h>

#include "acquire_common_defs.h"

#include "test_checksum.h"
#include "test_checksums_dispatch.h"
#include "test_cli.h"
#include "test_download.h"
#include "test_extract.h"
#include "test_fileutils.h"
#include "test_handle.h"
#if defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_MY_LIBFETCH
#include "test_libfetch.h"
#endif /* defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_MY_LIBFETCH */
#include "acquire_config.h"
#include "test_net_common.h"
#if (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL) ||             \
    (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO) || \
    (defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL)
#include "test_openssl.h"
#endif

#if defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH
#include "test_librhash.h"
#endif /* defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH */

#include "test_string_extras.h"
#include "test_url_utils.h"

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".test_downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(handle_suite);
  RUN_SUITE(fileutils_suite);
  RUN_SUITE(url_utils_suite);
  RUN_SUITE(string_extras_suite);
  RUN_SUITE(checksum_dispatch_suite);
  RUN_SUITE(checksums_suite);
  RUN_SUITE(downloads_suite);
  RUN_SUITE(net_common_suite);

#if (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL) ||             \
    (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO) || \
    (defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL)
  RUN_SUITE(openssl_backend_suite);
#endif
#if defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH
  RUN_SUITE(librhash_backend_suite);
#endif /* defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH */
#if defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_MY_LIBFETCH
  RUN_SUITE(libfetch_suite);
#endif /* defined(LIBACQUIRE_USE_LIBFETCH) && LIBACQUIRE_USE_MY_LIBFETCH */
  RUN_SUITE(cli_suite);
  GREATEST_MAIN_END();
}

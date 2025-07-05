#include <greatest.h>

#include "acquire_common_defs.h"

#if (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL) ||             \
    (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO) || \
    (defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL)
#include "test_openssl.h"
#endif
#include "acquire_config.h"

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".test_downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
#if (defined(LIBACQUIRE_USE_OPENSSL) && LIBACQUIRE_USE_OPENSSL) ||             \
    (defined(LIBACQUIRE_USE_COMMON_CRYPTO) && LIBACQUIRE_USE_COMMON_CRYPTO) || \
    (defined(LIBACQUIRE_USE_LIBRESSL) && LIBACQUIRE_USE_LIBRESSL)
  if (GREATEST_IS_VERBOSE()) {
    printf("\n--- Running OpenSSL/CommonCrypto Backend Tests ---\n");
  }
  RUN_SUITE(openssl_backend_suite);
#endif
  GREATEST_MAIN_END();
}

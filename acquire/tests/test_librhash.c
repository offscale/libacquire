#if defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH

#include "acquire_config.h"
#include <greatest.h>

#include "test_librhash.h"

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".test_downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
#if defined(LIBACQUIRE_USE_LIBRHASH)
  if (GREATEST_IS_VERBOSE()) {
    printf("\n--- Running LibRHash Backend Tests ---\n");
  }
  RUN_SUITE(librhash_backend_suite);
#else
  (void)argc;
  (void)argv;
#endif
  GREATEST_MAIN_END();
}
#endif /* defined(LIBACQUIRE_USE_LIBRHASH) && LIBACQUIRE_USE_LIBRHASH */

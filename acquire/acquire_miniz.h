#if !defined(LIBACQUIRE_ACQUIRE_MINIZ_H) && defined(LIBACQUIRE_USE_MINIZ) &&   \
    LIBACQUIRE_USE_MINIZ && defined(LIBACQUIRE_IMPLEMENTATION) &&              \
    !defined(LIBACQUIRE_EXTRACT_IMPL)
#define LIBACQUIRE_ACQUIRE_MINIZ_H
#define LIBACQUIRE_EXTRACT_IMPL

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>

#include <zip.h>

#include "acquire_errors.h"
#include "acquire_extract.h"

int on_extract_entry(const char *filename, void *arg) {
  /*
   * nop is fine, if a `verbose` mode is added, then can throw in:
  static int i = 0;
  int n = *(int *)arg;
  printf("Extracted: %s (%d of %d)\n", filename, ++i, n);
  */

  return 0;
}

int extract_archive(enum Archive archive, const char *archive_filepath,
                    const char *output_folder) {
  if (archive != LIBACQUIRE_ZIP)
    return UNIMPLEMENTED;
  {
    static int arg = 2;
    printf("Extracting archive %s to %s\n", archive_filepath, output_folder);
    return zip_extract(archive_filepath, output_folder, on_extract_entry, &arg);
  }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(LIBACQUIRE_ACQUIRE_MINIZ_H) &&                              \
          defined(LIBACQUIRE_USE_MINIZ) && LIBACQUIRE_USE_MINIZ &&                                                       \
          defined(LIBACQUIRE_IMPLEMENTATION) &&                                \
          !defined(LIBACQUIRE_EXTRACT_IMPL) */

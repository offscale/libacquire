/*
 * Prototype for extract API
 *
 * Always `#include` this when adding new extraction implementations,
 * to ensure the implementation matches the prototype.
 *
 * The extract API is for expanding the contents of archives
 * */

#ifndef LIBACQUIRE_ACQUIRE_EXTRACT_H
#define LIBACQUIRE_ACQUIRE_EXTRACT_H

#include "libacquire_export.h"

enum LIBACQUIRE_LIB_EXPORT Archive {
  LIBACQUIRE_ZIP,
  LIBACQUIRE_INFER,
  LIBACQUIRE_UNSUPPORTED_ARCHIVE
};

extern LIBACQUIRE_LIB_EXPORT int extract_archive(enum Archive, const char *,
                                                 const char *);

extern LIBACQUIRE_LIB_EXPORT enum Archive extension2archive(const char *);

#ifdef LIBACQUIRE_IMPLEMENTATION

#include <acquire_string_extras.h>

extern enum Archive extension2archive(const char *const s) {
  if (strncasecmp(s, ".zip", 6) == 0)
    return LIBACQUIRE_ZIP;
  else if (strlen(s) == 0)
    return LIBACQUIRE_UNSUPPORTED_ARCHIVE;
  else
    return LIBACQUIRE_INFER;
}

#endif /* LIBACQUIRE_IMPLEMENTATION */

#endif /* ! LIBACQUIRE_ACQUIRE_EXTRACT_H */

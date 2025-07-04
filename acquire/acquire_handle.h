#ifndef ACQUIRE_HANDLE_H
#define ACQUIRE_HANDLE_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "acquire_common_defs.h"
#include "libacquire_export.h"

#ifdef _MSC_VER
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#else
#include <limits.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#define ACQUIRE_PRINTF_FORMAT(fmt, arg)                                        \
  __attribute__((format(printf, fmt, arg)))
#else
#define ACQUIRE_PRINTF_FORMAT(fmt, arg)
#endif

struct acquire_error_info {
  enum acquire_error_code code;
  char message[256];
};

enum acquire_backend_type {
  ACQUIRE_BACKEND_NONE,
  ACQUIRE_BACKEND_CHECKSUM_OPENSSL,
  ACQUIRE_BACKEND_CHECKSUM_WINCRYPT,
  ACQUIRE_BACKEND_CHECKSUM_LIBRHASH,
  ACQUIRE_BACKEND_CHECKSUM_CRC32C
};

struct acquire_handle {
  volatile off_t bytes_processed;
  volatile off_t total_size;
  char current_file[PATH_MAX];
  volatile enum acquire_status status;
  void *backend_handle;
  FILE *output_file;
  struct acquire_error_info error;
  volatile int cancel_flag;
  enum acquire_backend_type active_backend;
};

extern LIBACQUIRE_EXPORT struct acquire_handle *acquire_handle_init(void);
extern LIBACQUIRE_EXPORT void
acquire_handle_free(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT enum acquire_error_code
acquire_handle_get_error_code(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT const char *
acquire_handle_get_error_string(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT void
acquire_handle_set_error(struct acquire_handle *handle,
                         enum acquire_error_code code, const char *fmt, ...)
    ACQUIRE_PRINTF_FORMAT(3, 4);

#if defined(LIBACQUIRE_IMPLEMENTATION)
#ifndef ACQUIRE_HANDLE_IMPL_
#define ACQUIRE_HANDLE_IMPL_
#include "acquire_status_codes.h"
#include <string.h>

struct acquire_handle *acquire_handle_init(void) {
  struct acquire_handle *h =
      (struct acquire_handle *)calloc(1, sizeof(struct acquire_handle));
  if (h) {
    h->total_size = -1;
    h->status = ACQUIRE_IDLE;
    h->error.code = ACQUIRE_OK;
    h->active_backend = ACQUIRE_BACKEND_NONE;
  }
  return h;
}
void acquire_handle_free(struct acquire_handle *h) {
  if (h)
    free(h);
}
enum acquire_error_code
acquire_handle_get_error_code(struct acquire_handle *h) {
  return h ? h->error.code : ACQUIRE_ERROR_INVALID_ARGUMENT;
}
const char *acquire_handle_get_error_string(struct acquire_handle *h) {
  return h ? h->error.message : "Invalid handle provided.";
}
void acquire_handle_set_error(struct acquire_handle *h,
                              enum acquire_error_code c, const char *fmt, ...) {
  if (!h)
    return;
  h->status = ACQUIRE_ERROR;
  h->error.code = c;
  if (fmt) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(h->error.message, sizeof(h->error.message), fmt, args);
    va_end(args);
  } else {
    h->error.message[0] = '\0';
  }
}
#endif /* ACQUIRE_HANDLE_IMPL_ */
#endif /* defined(LIBACQUIRE_IMPLEMENTATION) */

#endif /* !ACQUIRE_HANDLE_H */

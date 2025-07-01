/* acquire/acquire_handle.h */
#ifndef ACQUIRE_HANDLE_H
#define ACQUIRE_HANDLE_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "acquire_status_codes.h" /* Central status and error codes */
#include "libacquire_export.h"

#ifdef _MSC_VER
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif
#else
#include <limits.h>
#endif

/* --- Define a portable printf format attribute --- */
#if defined(__GNUC__) || defined(__clang__)
#define ACQUIRE_PRINTF_FORMAT(fmt_index, arg_index)                            \
  __attribute__((format(printf, fmt_index, arg_index)))
#else
#define ACQUIRE_PRINTF_FORMAT(fmt_index, arg_index)
#endif

struct acquire_error {
  enum acquire_error_code code;
  char message[256];
};

struct acquire_handle {
  off_t bytes_processed;
  off_t total_size;
  char current_file[PATH_MAX];
  enum acquire_status status;
  void *backend_handle;
  FILE *output_file;
  struct acquire_error error;
  int cancel_flag;
};

extern LIBACQUIRE_EXPORT struct acquire_handle *acquire_handle_init(void);
extern LIBACQUIRE_EXPORT void
acquire_handle_free(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT enum acquire_error_code
acquire_handle_get_error_code(struct acquire_handle *handle);
extern LIBACQUIRE_EXPORT const char *
acquire_handle_get_error_string(struct acquire_handle *handle);

/**
 * @brief Sets the error state on a handle (for internal use by backends).
 *
 * @param handle The handle to modify.
 * @param code The error code to set.
 * @param fmt The printf-style format string for the error message.
 * @param ... Variable arguments for the format string.
 */
extern LIBACQUIRE_EXPORT void
acquire_handle_set_error(struct acquire_handle *handle,
                         enum acquire_error_code code, const char *fmt, ...)
    ACQUIRE_PRINTF_FORMAT(3, 4);

#if defined(LIBACQUIRE_IMPLEMENTATION) && defined(LIBACQUIRE_HANDLE_IMPL)

struct acquire_handle *acquire_handle_init(void) {
  struct acquire_handle *handle =
      (struct acquire_handle *)calloc(1, sizeof(struct acquire_handle));
  if (handle) {
    handle->total_size = -1;
    handle->status = ACQUIRE_IDLE;
    handle->error.code = ACQUIRE_OK;
  }
  return handle;
}

void acquire_handle_free(struct acquire_handle *handle) {
  if (!handle)
    return;
  if (handle->output_file)
    fclose(handle->output_file);
  free(handle->backend_handle);
  free(handle);
}

enum acquire_error_code
acquire_handle_get_error_code(struct acquire_handle *handle) {
  return handle ? handle->error.code : ACQUIRE_ERROR_INVALID_ARGUMENT;
}

const char *acquire_handle_get_error_string(struct acquire_handle *handle) {
  return handle ? handle->error.message : "Invalid handle provided.";
}

void acquire_handle_set_error(struct acquire_handle *handle,
                              enum acquire_error_code code, const char *fmt,
                              ...) {
  if (!handle)
    return;
  handle->status = ACQUIRE_ERROR;
  handle->error.code = code;
  if (fmt) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(handle->error.message, sizeof(handle->error.message), fmt, args);
    va_end(args);
  } else {
    handle->error.message[0] = '\0';
  }
}
#endif /* LIBACQUIRE_IMPLEMENTATION && LIBACQUIRE_HANDLE_IMPL */

#endif /* !ACQUIRE_HANDLE_H */

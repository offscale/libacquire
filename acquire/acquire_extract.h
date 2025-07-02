/* acquire/acquire_extract.h */
#ifndef LIBACQUIRE_ACQUIRE_EXTRACT_H
#define LIBACQUIRE_ACQUIRE_EXTRACT_H

#include "acquire_handle.h"
#include "libacquire_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Asynchronous API --- */

extern LIBACQUIRE_EXPORT int
acquire_extract_async_start(struct acquire_handle *handle,
                            const char *archive_path, const char *dest_path);

extern LIBACQUIRE_EXPORT enum acquire_status
acquire_extract_async_poll(struct acquire_handle *handle);

extern LIBACQUIRE_EXPORT void
acquire_extract_async_cancel(struct acquire_handle *handle);

/* --- Synchronous API --- */

extern LIBACQUIRE_EXPORT int acquire_extract_sync(struct acquire_handle *handle,
                                                  const char *archive_path,
                                                  const char *dest_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !LIBACQUIRE_ACQUIRE_EXTRACT_H */

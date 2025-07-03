/* acquire/acquire_download.h */
#ifndef LIBACQUIRE_ACQUIRE_DOWNLOAD_H
#define LIBACQUIRE_ACQUIRE_DOWNLOAD_H

#include "acquire_handle.h"
#include "libacquire_export.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* --- Synchronous API --- */
extern LIBACQUIRE_EXPORT int
acquire_download_sync(struct acquire_handle *handle, const char *url,
                      const char *dest_path);

/* --- Asynchronous API --- */
extern LIBACQUIRE_EXPORT int
acquire_download_async_start(struct acquire_handle *handle, const char *url,
                             const char *dest_path);

extern LIBACQUIRE_EXPORT enum acquire_status
acquire_download_async_poll(struct acquire_handle *handle);

extern LIBACQUIRE_EXPORT void
acquire_download_async_cancel(struct acquire_handle *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBACQUIRE_ACQUIRE_DOWNLOAD_H */

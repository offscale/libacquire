/*
 * openbsd's `ftp` implementation of libacquire's download API
 *
 * Very much a WiP
 * */

#if !defined(LIBACQUIRE_OPENBSD_FTP_H) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_OPENBSD_FTP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "openbsd.ftp/extern.h"

#ifdef LIBACQUIRE_DOWNLOAD_DIR_IMPL
const char *get_download_dir(void) { return ".downloads"; }
#endif /* LIBACQUIRE_DOWNLOAD_DIR_IMPL */

#ifdef LIBACQUIRE_DOWNLOAD_IMPL
int download(const char *url, enum Checksum checksum, const char *hash,
             const char *target_location /*[NAME_MAX]*/, bool follow,
             size_t retry, size_t verbosity) {
  sendrequest();
}
#endif /* LIBACQUIRE_DOWNLOAD_IMPL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(LIBACQUIRE_OPENBSD_FTP_H) &&                                \
          defined(LIBACQUIRE_IMPLEMENTATION) */

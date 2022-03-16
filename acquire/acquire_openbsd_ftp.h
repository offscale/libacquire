/*
 * openbsd's `ftp` implementation of libacquire's download API
 *
 * Very much a WiP
 * */

#if !defined(LIBACQUIRE_OPENBSD_FTP_H) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_OPENBSD_FTP_H

#include "openbsd.ftp/extern.h"

const char *get_download_dir() {
    return ".downloads";
}

int download(const char* url, enum Checksum checksum, const char* hash, const char target_location[NAME_MAX],
             bool follow, size_t retry, size_t verbosity) {
    sendrequest();
}

#endif /* !defined(LIBACQUIRE_OPENBSD_FTP_H) && defined(LIBACQUIRE_IMPLEMENTATION) */

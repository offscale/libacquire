#ifndef LIBACQUIRE_OPENBSD_FTP_H
#define LIBACQUIRE_OPENBSD_FTP_H

#include "openbsd.ftp/extern.h"

const char *get_download_dir() {
    return ".downloads";
}

int download(const char* url, enum Checksum checksum, const char* hash, const char target_directory[248],
             bool follow, size_t retry, size_t verbosity) {
    sendrequest()
}

#endif /* LIBACQUIRE_OPENBSD_FTP_H */

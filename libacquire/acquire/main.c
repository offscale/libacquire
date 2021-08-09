#include <stdio.h>
#include <stdlib.h>

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include <acquire_stdbool.h>
#endif

#include <acquire_config.h>
#include <acquire.h>
#include <acquire_errors.h>

#include "cli.h"

#ifdef USE_LIBCURL

#include <acquire_libcurl.h>

#elif defined(USE_WININET)

#include <acquire_wininet.h>

#elif defined(USE_LIBFETCH)

#include <acquire_libfetch.h>

#elif defined(USE_OPENBSD_FTP)

#include <acquire_openbsd_ftp.h>

#endif

#if defined(USE_OPENSSL) || defined(USE_LIBRESSL)
#include <acquire_openssl.h>
#elif defined(USE_WINCRYPT)
#include <acquire_wincrypt.h>
#elif defined(USE_COMMON_CRYPTO)
#include <acquire_common_crypto.h>
#endif



int main(int argc, char *argv[]) {
    struct DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ VERSION);
    enum Checksum checksum = LIBACQUIRE_SHA256;

    /* TODO: Ensure environment variables don't take priority over CLI arguments */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    char *check;
    size_t len;
    errno_t err = _dupenv_s(&check, &len, "CHECK");
    if (err) check = NULL;
#else
    const char *check = getenv("CHECK");
#endif

    if (check != NULL && args.check == 0) args.check = (bool) check;
    if (args.directory == 0) args.directory = TMPDIR;
    if (args.url == 0) {
        switch (argc) {
            case 2:
                if (is_url(argv[1])) {
                    args.url = argv[1];
                    break;
                }
            case 1:
                return UNIMPLEMENTED;
            default:
                if (is_url(argv[1]))
                    args.url = argv[1];
                else if (is_url(argv[argc - 1]))
                    args.url = argv[argc - 1];
                else
                    return UNIMPLEMENTED;
                printf("`args.url`:\t\"%s\"\n", args.url);
        }
    }
    if (args.checksum != NULL)
        checksum = string2checksum((const char *) args.checksum);

    if (args.check)
        return is_downloaded(args.url, checksum, args.hash, args.directory) ?
               EXIT_SUCCESS : EXIT_FAILURE;

    return download(args.url, checksum, args.hash, args.directory, false, 0, 0);
}
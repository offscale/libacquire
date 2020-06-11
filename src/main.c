#include <stdio.h>
#include <stdlib.h>
#include "stdbool.h"

#include "config.h"
#include "cli.h"
#include "errors.h"
#include "acquire.h"

#ifdef USE_LIBCURL

#include "libcurl.h"

#elif defined(USE_WININET)

#include "wininet.h"

#elif defined(USE_LIBFETCH)

#include "libfetch.h"

#elif defined(USE_OPENBSD_FTP)

#include "openbsd_ftp.h"

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
    size_t i;
    for(i=0; i<argc; i++)
        printf("main::argv[%lu] = %s\n", i, argv[i]);

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
        }
    }
    if (args.checksum != NULL)
        checksum = string2checksum((const char *) args.checksum);

    if (args.check)
        return is_downloaded(args.url, checksum, args.hash, args.directory) ?
               EXIT_SUCCESS : EXIT_FAILURE;

    printf("`args.url`:\t\"%s\"\n", args.url);
    download(args.url, checksum, args.hash, args.directory, false, 0, 0);

    return EXIT_SUCCESS;
}

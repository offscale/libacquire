#include <stdio.h>
#include <stdlib.h>
#include "stdbool.h"

#include "config.h"
#include "cli.h"
#include "errors.h"
#include "acquire.h"
#ifdef USE_LIBCURL
#include "libcurl.h"
#endif

int main(int argc, char *argv[]) {
    struct DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ VERSION);

    /* TODO: Ensure environment variables don't take priority over CLI arguments */
    const char *check = getenv("CHECK");
    if (check != NULL && args.check == 0) args.check = (bool) check;

    if (args.check)
        return UNIMPLEMENTED;
    else if (args.url == 0) {
        if (argc == 2) args.url = argv[1];
        else return UNIMPLEMENTED;
    }
    if (args.directory == 0)
        args.directory = TMPDIR;

    printf("`args.url`:\t\"%s\"\n", args.url);
    download(args.url, NULL, args.directory, false, 0, 0);

    return EXIT_SUCCESS;
}

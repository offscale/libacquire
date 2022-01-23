/*
 * librhash implementation of libacquire's checksum API
 * */

#if !defined(LIBACQUIRE_ACQUIRE_LIBRHASH_H) && defined(LIBACQUIRE_IMPLEMENTATION) && defined(USE_LIBRHASH)
#define LIBACQUIRE_ACQUIRE_LIBRHASH_H

#include <errno.h>

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "acquire_stdbool.h"
#endif

#include <rhash.h>

#include <acquire_string_extras.h>

struct CodeHash {
    int code;
    char hash[130];
};

struct CodeHash hash_file(const char* filepath, enum rhash_ids hash)
{
    char digest[64];

    rhash_library_init(); /* initialize static data */

    int res = rhash_file(hash, filepath, (unsigned char*)digest);
    if(res < 0) {
        const struct CodeHash code_hash = {res, 0};
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
        size_t errmsglen = strerrorlen_s(errno) + 1;
        char *errmsg = malloc(errmsglen);
        strerror_s(errmsg, errmsglen, errno);
        fprintf(stderr, "LibRHash error: %s: %s\n", filepath, errmsg);
        free(errmsg);
#else
        fprintf(stderr, "LibRHash error: %s: %s\n", filepath, strerror(errno));
#endif
        return code_hash;
    }

    /* convert binary digest to hexadecimal string */
    rhash_print_bytes((char *) hash, (const unsigned char*)digest, rhash_get_digest_size(RHASH_TTH),
                      (RHPR_BASE32 | RHPR_UPPERCASE));

    printf("%s (%s) = %s\n", rhash_get_name(RHASH_TTH), filepath, (unsigned char*)hash);
    const struct CodeHash code_hash = {res, hash};
    return code_hash;
}

bool crc32c(const char *filename, const char *hash) {
    const struct CodeHash code_hash = hash_file(filename, RHASH_CRC32C);
    return code_hash.code == EXIT_SUCCESS ? strcmp(code_hash.hash, hash) == 0 : false;
}

#endif /* !defined(LIBACQUIRE_ACQUIRE_LIBRHASH_H) && defined(LIBACQUIRE_IMPLEMENTATION) && defined(USE_LIBRHASH) */

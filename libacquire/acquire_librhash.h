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
    int res;
    struct CodeHash code_hash;

    rhash_library_init(); /* initialize static data */
    res = rhash_file(hash, filepath, (unsigned char*)digest);
    code_hash.code = res;
    if(code_hash.code < 0) {
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
        size_t errmsglen = strerrorlen_s(errno) + 1;
        char *errmsg = malloc(errmsglen);
        strerror_s(errmsg, errmsglen, errno);
        fprintf(stderr, "LibRHash error: %s: %s\n", filepath, errmsg);
        free(errmsg);
#else
        fprintf(stderr, "LibRHash error: %s: %s\n", filepath, strerror(errno));
#endif
    } else
        /* convert binary digest to hexadecimal string */
        rhash_print_bytes(code_hash.hash, (const unsigned char*)digest, rhash_get_digest_size(hash),
                          RHPR_BASE64);
    return code_hash;
}

bool crc32c(const char *filename, const char *hash) {
    const struct CodeHash code_hash = hash_file(filename, RHASH_CRC32C);
    return code_hash.code < 0 ? false : strcmp(code_hash.hash, hash) == 0;
}

#endif /* !defined(LIBACQUIRE_ACQUIRE_LIBRHASH_H) && defined(LIBACQUIRE_IMPLEMENTATION) && defined(USE_LIBRHASH) */

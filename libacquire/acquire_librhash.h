/*
 * librhash implementation of libacquire's checksum API
 * */

#if !defined(LIBACQUIRE_ACQUIRE_LIBRHASH_H) && defined(LIBACQUIRE_IMPLEMENTATION)
#define LIBACQUIRE_ACQUIRE_LIBRHASH_H

#include <errno.h>
#include <rhash.h>

struct CodeHash {
    int code;
    char hash[130];
};

struct CodeHash hash_file(const char* filepath, enum rhash_ids hash)
{
    char digest[64];

    rhash_library_init(); /* initialize static data */

    int res = rhash_file(hash, filepath, digest);
    if(res < 0) {
        const struct CodeHash code_hash = {res, NULL};
        fprintf(stderr, "LibRHash error: %s: %s\n", filepath, strerror(errno));
        return code_hash;
    }

    /* convert binary digest to hexadecimal string */
    rhash_print_bytes(hash, digest, rhash_get_digest_size(RHASH_TTH),
                      (RHPR_BASE32 | RHPR_UPPERCASE));

    printf("%s (%s) = %s\n", rhash_get_name(RHASH_TTH), filepath, hash);
    const struct CodeHash code_hash = {res, hash};
    return code_hash;
}

#endif /* !defined(LIBACQUIRE_ACQUIRE_LIBRHASH_H) && defined(LIBACQUIRE_IMPLEMENTATION) */

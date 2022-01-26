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

struct CodeHash hash_file(const char* filepath, enum rhash_ids hash, enum rhash_print_sum_flags print_sum_flags)
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
        rhash_print_bytes(code_hash.hash, (const unsigned char*)digest, rhash_get_digest_size(hash),
                          print_sum_flags);
    return code_hash;
}

bool crc32c(const char *filename, const char *hash) {
    const struct CodeHash code_hash = hash_file(filename, RHASH_CRC32C, RHPR_BASE64);
    return code_hash.code < 0 ? false : strcmp(code_hash.hash, hash) == 0;
}

#ifndef LIBACQUIRE_IMPL_SHA256
#define LIBACQUIRE_IMPL_SHA256
bool sha256(const char *filename, const char *hash) {
    const struct CodeHash code_hash = hash_file(filename, RHASH_SHA256, RHPR_DEFAULT);
    return code_hash.code < 0 ? false : strcmp(code_hash.hash, hash) == 0;
}
#endif /* !LIBACQUIRE_IMPL_SHA256 */

#ifndef LIBACQUIRE_IMPL_SHA512
#define LIBACQUIRE_IMPL_SHA512
bool sha512(const char *filename, const char *hash) {
    const struct CodeHash code_hash = hash_file(filename, RHASH_SHA512, RHPR_DEFAULT);
    return code_hash.code < 0 ? false : strcmp(code_hash.hash, hash) == 0;
}
#endif /* !LIBACQUIRE_IMPL_SHA512 */

#endif /* !defined(LIBACQUIRE_ACQUIRE_LIBRHASH_H) && defined(LIBACQUIRE_IMPLEMENTATION) && defined(USE_LIBRHASH) */

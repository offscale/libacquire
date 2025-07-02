#ifndef TEST_CHECKSUMS_DISPATCH_H
#define TEST_CHECKSUMS_DISPATCH_H

#include "acquire_checksums.h"
#include "acquire_handle.h"
#include <greatest.h>

TEST test_string2checksum(void) {
  ASSERT_EQ(LIBACQUIRE_CRC32C, string2checksum("CRC32C"));
  ASSERT_EQ(LIBACQUIRE_SHA256, string2checksum("SHA256"));
  ASSERT_EQ(LIBACQUIRE_SHA512, string2checksum("SHA512"));
  ASSERT_EQ(LIBACQUIRE_UNSUPPORTED_CHECKSUM, string2checksum("UnknownAlgo"));
  ASSERT_EQ(LIBACQUIRE_UNSUPPORTED_CHECKSUM, string2checksum(NULL));
  PASS();
}

TEST test_dispatch_verify_sync_invalid_args(void) {
  struct acquire_handle *h = acquire_handle_init();
  int res;

  res = acquire_verify_sync(NULL, "file", LIBACQUIRE_SHA256, "hash");
  ASSERT_EQ(-1, res);

  res = acquire_verify_sync(h, NULL, LIBACQUIRE_SHA256, "hash");
  ASSERT_EQ(-1, res);

  res = acquire_verify_sync(h, "file", LIBACQUIRE_SHA256, NULL);
  ASSERT_EQ(-1, res);

  acquire_handle_free(h);
  PASS();
}

SUITE(checksum_dispatch_suite) {
  RUN_TEST(test_string2checksum);
  RUN_TEST(test_dispatch_verify_sync_invalid_args);
}

#endif /* !TEST_CHECKSUMS_DISPATCH_H */

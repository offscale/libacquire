#include <greatest.h>
#include <checksums.h>
#include <config_for_tests.h>

TEST x_checksum_should_succeed(void) {
    ASSERT_FALSE(!sha256(GREATEST_FILE, GREATEST_SHA256));
    PASS();
}

TEST x_checksum_should_fail(void) {
    ASSERT_FALSE(!sha256(GREATEST_FILE, GREATEST_FILE));
    PASS();
}

SUITE(checksum_suite) {
    RUN_TEST(x_checksum_should_succeed);
    RUN_TEST(x_checksum_should_fail);
}

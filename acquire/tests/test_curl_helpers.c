#include <greatest.h>

#include "test_curl_helpers.h"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(curl_helpers_suite);
  GREATEST_MAIN_END();
}
#include <greatest.h>

#include "test_libcurl_backend.h"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(curl_backend_suite);
  GREATEST_MAIN_END();
}

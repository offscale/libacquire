#include <greatest.h>

#include "test_extract.h"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(extract_suite);
  GREATEST_MAIN_END();
}

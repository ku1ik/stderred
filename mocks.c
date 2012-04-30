#include "config.h"

void setup_test() {
  start_color_code = NULL;
  end_color_code = NULL;
}

int isatty(int fildes) {
  return 1;
}

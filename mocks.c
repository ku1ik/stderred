#include "config.h"

// Signature of library initialization function
void init();

extern char *start_color_code;
extern size_t start_color_code_size;

extern char *end_color_code;
extern size_t end_color_code_size;

void setup_test() {
  start_color_code = NULL;
  setenv("STDERRED_ESC_CODE", ">", 1);
  end_color_code = "<";
  init();
}

int isatty(int fildes) {
  return 1;
}

#include "config.h"
#include "mocks.h"

// Signature of library initialization function
void init();

extern char *start_color_code;
extern size_t start_color_code_size;

extern char *end_color_code;
extern size_t end_color_code_size;

extern bool is_valid_env;
bool mock_tty = true;

void reset_stderred() {
  is_valid_env = false;
  mock_tty = true;
  start_color_code = NULL;
  unsetenv("STDERRED_BLACKLIST");
  setenv("STDERRED_ESC_CODE", ">", 1);
  end_color_code = "<";
}

int isatty(int fildes) {
  return mock_tty;
}

void init_mocks(struct stderred *stderred) {
  stderred->init = &init;
  stderred->reset = &reset_stderred;
  stderred->has_valid_env = &is_valid_env;
  stderred->mock_tty = &mock_tty;
  reset_stderred();
}

#ifndef MOCKS_H_AEXVXBM4
#define MOCKS_H_AEXVXBM4

#include <stdbool.h>

struct stderred {
  void (*init)();
  void (*reset)();
  bool *has_valid_env;
  bool *mock_tty;
};

#endif /* end of include guard: MOCKS_H_AEXVXBM4 */


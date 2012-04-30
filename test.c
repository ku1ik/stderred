#include "config.h"
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <dlfcn.h>

void test_vfprintf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

int main() {
#ifdef __APPLE__
  void *lib = dlopen(getenv("DYLIB_INSERT_LIBRARIES"), RTLD_LAZY);
#else
  void *lib = dlopen(getenv("LD_PRELOAD"), RTLD_LAZY);
#endif
  if (!lib) {
    printf("Failed loading lib: %s\n", dlerror());
    exit -1;
  }
  void (*setup_test)() = dlsym(lib, "setup_test");

  if (!setup_test) {
    printf("Failed getting setup test: %s\n", dlerror());
    exit -2;
  }
  dlclose(lib);

  setenv("STDERRED_ESC_CODE", ">", 1);
  setenv("STDERRED_END_CODE", "<", 1);
  dup2(STDOUT_FILENO, STDERR_FILENO);

  setup_test();
  printf("1 printf\n");
  fflush(stdout);

  setup_test();
  write(2, "2 write\n", 8);

  setup_test();
  fprintf(stderr, "2 fprintf\n");

#ifdef HAVE_FPRINTF_UNLOCKED
  setup_test();
  fprintf_unlocked(stderr, "2 fprintf_unlocked\n");
#endif

  setup_test();
  fwrite("2 fwrite\n", 9, 1, stderr);

#ifdef HAVE_FWRITE_UNLOCKED
  setup_test();
  fwrite_unlocked("2 fwrite_unlocked\n", 18, 1, stderr);
#endif

  setup_test();
  fputc(0x32, stderr); fflush(stderr); printf(" <= fputc\n"); fflush(stdout);

#ifdef HAVE_FPUTC_UNLOCKED
  setup_test();
  fputc_unlocked(0x32, stderr); fflush(stderr);
  printf(" <= fputc_unlocked\n"); fflush(stdout);
#endif

  setup_test();
  fputs("2 fputs\n", stderr);

#ifdef HAVE_FPUTS_UNLOCKED
  setup_test();
  fputs_unlocked("2 fputs_unlocked\n", stderr);
#endif

  setup_test();
  test_vfprintf("2 %s\n", "vfprintf");

  setup_test();
  errno = ENOSYS; perror("2 perror");

#ifdef HAVE_ERROR
  setup_test();
  error(0, ENOSYS, "%s", "2 error");
#endif

#ifdef HAVE_ERROR_AT_LINE
  setup_test();
  error_at_line(0, ENOENT, __FILE__, __LINE__, "%s", "2 error_at_line");
#endif

  return 0;
}

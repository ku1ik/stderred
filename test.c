#include "config.h"
#include "polyfill.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <dlfcn.h>

#ifdef __APPLE__
  #define LIB_PRELOAD_ENV_VAR "DYLIB_INSERT_LIBRARIES"
#else
  #define LIB_PRELOAD_ENV_VAR "LD_PRELOAD"
#endif

typedef void (*test)();
typedef struct {
  char *name;
  test test;
} unit_test;

#define TEST(name) static void test_##name()
#define UNIT(name) {#name, &test_##name}

TEST(printf) {
  printf("1 printf");
  fflush(stdout);
}

TEST(write) {
  write(2, "2 write", 7);
}

TEST(fwrite) {
  fwrite("2 fwrite", 8, 1, stderr);
}

TEST(fwrite_unlocked) {
  fwrite_unlocked("2 fwrite_unlocked", 17, 1, stderr);
}

TEST(fputc) {
  fputc(0x32, stderr); fflush(stderr); printf(" <= fputc"); fflush(stdout);
}

TEST(fputc_unlocked) {
  fputc_unlocked(0x32, stderr); fflush(stderr);
  printf(" <= fputc_unlocked"); fflush(stdout);
}

TEST(fputs) {
  fputs("2 fputs", stderr);
}

TEST(fputs_unlocked) {
  fputs_unlocked("2 fputs_unlocked", stderr);
}

TEST(fprintf) {
  fprintf(stderr, "%s", "2 fprintf");
}

TEST(fprintf_unlocked) {
  fprintf_unlocked(stderr, "%s", "2 fprintf_unlocked");
}

void test_vfprintf_helper(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

TEST(vfprintf) {
  test_vfprintf_helper("2 %s", "vfprintf");
}

TEST(perror) {
  errno = ENOSYS; perror("2 perror");
}

TEST(error) {
  error(0, ENOSYS, "%s", "2 error");
}

TEST(error_at_line) {
  error_at_line(0, ENOENT, __FILE__, __LINE__, "%s", "2 error_at_line");
}

unit_test tests[] = {
  UNIT(printf),
  UNIT(write),
  UNIT(fwrite),
  UNIT(fwrite_unlocked),
  UNIT(fputc),
  UNIT(fputc_unlocked),
  UNIT(fputs),
  UNIT(fputs_unlocked),
  UNIT(fprintf),
  UNIT(fprintf_unlocked),
  UNIT(vfprintf),
  UNIT(perror),
  UNIT(error),
  UNIT(error_at_line),
};

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Must supply a test name to run, choices are:\n");
    for (int i = 0, num = sizeof(tests)/sizeof(unit_test); i < num; i++) {
      printf("  %s\n", tests[i].name);
    }
    return EXIT_FAILURE;
  }

  void *lib = dlopen(getenv(LIB_PRELOAD_ENV_VAR), RTLD_LAZY);
  if (!lib) {
    printf("Failed loading lib: %s\n", dlerror());
    return EXIT_FAILURE;
  }
  void (*setup_test)() = dlsym(lib, "setup_test");

  if (!setup_test) {
    printf("Failed getting setup test: %s\n", dlerror());
    return EXIT_FAILURE;
  }
  dlclose(lib);

  // Needed so ctest can get output for comparison
  dup2(STDOUT_FILENO, STDERR_FILENO);

  for (int i = 0, num = sizeof(tests)/sizeof(unit_test); i < num; i++) {
    if(!strcmp(tests[i].name, argv[1])) {
      setup_test();
      tests[i].test();
      return EXIT_SUCCESS;
    }
  }

  printf("Test \"%s\" does not exist.\n"
         "Try supplying no arguments to see available tests\n", argv[1]);
  return EXIT_FAILURE;
}

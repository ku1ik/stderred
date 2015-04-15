#include "config.h"
#include "polyfill.h"
#include "mocks.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <assert.h>
#include <signal.h>

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

struct stderred stderred;

void signal_handler(int sig) {
  if (sig == SIGABRT) {
    exit(EXIT_FAILURE);
  }
}

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

TEST(err) {
  err(EXIT_SUCCESS, "%s", "2 err");
}

TEST(err_empty) {
  err(EXIT_SUCCESS, "2 err_empty");
}

static void test_verr_helper(int eval, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  verr(eval, fmt, args);
  va_end(args);
}

TEST(verr) {
  test_verr_helper(EXIT_SUCCESS, "%s", "2 verr");
}

TEST(errc) {
  errc(EXIT_SUCCESS, ENOSYS, "%s", "2 errc");
}

TEST(errc_empty) {
  errc(EXIT_SUCCESS, ENOSYS, "2 errc_empty");
}

static void test_verrc_helper(int eval, int code, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  verrc(eval, code, fmt, args);
  va_end(args);
}

TEST(verrc) {
  test_verrc_helper(EXIT_SUCCESS, ENOSYS, "%s", "2 verrc");
}

TEST(errx) {
  errx(EXIT_SUCCESS, "%s", "2 errx");
}

TEST(errx_empty) {
  errx(EXIT_SUCCESS, "2 errx_empty");
}

static void test_verrx_helper(int eval, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  verrx(eval, fmt, args);
  va_end(args);
}

TEST(verrx) {
  test_verrx_helper(EXIT_SUCCESS, "%s", "2 verrx");
}

TEST(warn) {
  warn("%s", "2 warn");
}

TEST(warn_empty) {
  warn("2 warn_empty");
}

static void test_vwarn_helper(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vwarn(fmt, args);
  va_end(args);
}

TEST(vwarn) {
  test_vwarn_helper("%s", "2 vwarn");
}

TEST(warnc) {
  warnc(ENOSYS, "%s", "2 warnc");
}

TEST(warnc_empty) {
  warnc(ENOSYS, "2 warnc_empty");
}

static void test_vwarnc_helper(int code, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vwarnc(code, fmt, args);
  va_end(args);
}

TEST(vwarnc) {
  test_vwarnc_helper(ENOSYS, "%s", "2 vwarnc");
}

TEST(warnx) {
  warnx("%s", "2 warnx");
}

TEST(warnx_empty) {
  warnx("2 warnx_empty");
}

static void test_vwarnx_helper(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vwarnx(fmt, args);
  va_end(args);
}

TEST(vwarnx) {
  test_vwarnx_helper("%s", "2 vwarnx");
}

TEST(err_uses_set_file) {
  err_set_file(stdout);
  warnx("%s", "1 warnx");
  fflush(stdout);

  err_set_file(NULL);
  warnx("%s", "2 warnx");
}

TEST(blacklist) {
  // Verify on a normal startup that the env is good
  assert(*stderred.has_valid_env);

  // Reset env, blacklist ourself explicitly and reinit
  stderred.reset();
  setenv("STDERRED_BLACKLIST", PROGRAM_NAME, 1);
  stderred.init();

  // Verify that with ourself blacklisted the env is invalid
  assert(!*stderred.has_valid_env);

  // Reset again and use a regex for our name
  stderred.reset();
  setenv("STDERRED_BLACKLIST", "test_.*", 1);
  stderred.init();

  // Verify the blacklist accepts the regex and marks env as invalid
  assert(!*stderred.has_valid_env);

  // Test to make sure above did not pass because of false positives
  stderred.reset();
  stderred.init();
  assert(*stderred.has_valid_env);
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
  UNIT(err),
  UNIT(err_empty),
  UNIT(verr),
  UNIT(errc),
  UNIT(errc_empty),
  UNIT(verrc),
  UNIT(errx),
  UNIT(errx_empty),
  UNIT(verrx),
  UNIT(warn),
  UNIT(warn_empty),
  UNIT(vwarn),
  UNIT(warnc),
  UNIT(warnc_empty),
  UNIT(vwarnc),
  UNIT(warnx),
  UNIT(warnx_empty),
  UNIT(vwarnx),
  UNIT(err_uses_set_file),
  UNIT(blacklist)
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

  void (*init_mocks)() = dlsym(lib, "init_mocks");

  if (!init_mocks) {
    printf("Failed getting mocks init function: %s\n", dlerror());
    return EXIT_FAILURE;
  }
  dlclose(lib);

  init_mocks(&stderred);

  struct sigaction act;
  act.sa_handler = signal_handler;
  sigaction(SIGABRT, &act, NULL);

  // Needed so ctest can get output for comparison
  dup2(STDOUT_FILENO, STDERR_FILENO);

  for (int i = 0, num = sizeof(tests)/sizeof(unit_test); i < num; i++) {
    if(!strcmp(tests[i].name, argv[1])) {
      stderred.init();
      tests[i].test();
      stderred.reset();
      return EXIT_SUCCESS;
    }
  }

  printf("Test \"%s\" does not exist.\n"
         "Try supplying no arguments to see available tests\n", argv[1]);
  return EXIT_FAILURE;
}

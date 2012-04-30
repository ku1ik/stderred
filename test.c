#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

int isatty(int fd) {
  return 1;
}

void test_vfprintf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

int main() {
  setenv("STDERRED_ESC_CODE", ">", 1);
  setenv("STDERRED_END_CODE", "<", 1);
  dup2(STDOUT_FILENO, STDERR_FILENO);

  printf("1 printf\n");
  fflush(stdout);

  write(2, "2 write\n", 8);

  fprintf(stderr, "2 fprintf\n");

#ifdef HAVE_FPRINTF_UNLOCKED
  fprintf_unlocked(stderr, "2 fprintf_unlocked\n");
#endif

  fwrite("2 fwrite\n", 9, 1, stderr);

#ifdef HAVE_FWRITE_UNLOCKED
  fwrite_unlocked("2 fwrite_unlocked\n", 18, 1, stderr);
#endif

  fputc(0x32, stderr); fflush(stderr); printf(" <= fputc\n"); fflush(stdout);

#ifdef HAVE_FPUTC_UNLOCKED
  fputc_unlocked(0x32, stderr); fflush(stderr);
  printf(" <= fputc_unlocked\n"); fflush(stdout);
#endif

  fputs("2 fputs\n", stderr);

#ifdef HAVE_FPUTS_UNLOCKED
  fputs_unlocked("2 fputs\n", stderr);
#endif

  test_vfprintf("2 %s\n", "vfprintf");

  errno = ENOSYS; perror("2 perror");

#ifdef HAVE_ERROR
  error(0, ENOSYS, "2 error");
#endif

#ifdef HAVE_ERROR_AT_LINE
  error_at_line(0, ENOENT, __FILE__, __LINE__, "2 error_at_line");
#endif

  return 0;
}

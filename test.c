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
  dup2(1, 2);

  printf("1 printf\n");
  fflush(stdout);

  write(2, "2 write\n", 8);
  fprintf(stderr, "2 fprintf\n");
  fwrite("2 fwrite\n", 9, 1, stderr);
  fputc(0x32, stderr); fflush(stderr); printf(" <= fputc\n"); fflush(stdout);
  fputs("2 fputs\n", stderr);
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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

void test_vfprintf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

int main() {
  setenv("STDERRED_ESC_CODE", ">", 1);
  setenv("STDERRED_END_CODE", "<", 1);

  printf("[1] printf\n");
  write(2, "[2] write\n", 11);
  fprintf(stderr, "[2] fprintf\n");
  fwrite("[2] fwrite\n", 12, 1, stderr);
  fputc(0x32, stderr); printf(" <= fputc\n");
  fputs("[2] fputs\n", stderr);
  test_vfprintf("[2] %s\n", "vfprintf");
  errno = ENOSYS; perror("[2] perror");

  return 0;
}

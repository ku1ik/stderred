#include <stdio.h>

int main() {
  printf("[1] printf\n");
  write(2, "[2] write\n", 11);
  fprintf(stderr, "[2] fprintf\n");
  fwrite("[2] fwrite\n", 12, 1, stderr);
  fputc(0x32, stderr);
  printf(" <= fputc\n");
  fputs("[2] fputs\n", stderr);

  return 0;
}

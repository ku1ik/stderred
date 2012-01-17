#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>

#undef write

static const char RED[]       = "\x1b[31m";
static const char COL_RESET[] = "\x1b[0m";
#define COL_RESET_SIZE sizeof(COL_RESET)-1

char *color_code;
int color_code_size;

void _reset() {
  struct iovec vec = { (char *)COL_RESET, COL_RESET_SIZE };
  writev(STDERR_FILENO, &vec, 1);
}

ssize_t write(int fd, const void* buf, size_t count) {
  if (color_code == NULL) {
    color_code = getenv("STDERRED_ESC_CODE");

    if (color_code == NULL) {
      color_code = (char *)RED;
    }

    color_code_size = strlen(color_code);
  }

  if (fd == STDERR_FILENO && isatty(STDERR_FILENO)) {
    struct iovec vec[3] = {
      { (char *)color_code, color_code_size },
      { (char *)buf, count },
      { (char *)COL_RESET, COL_RESET_SIZE }
    };

    ssize_t written = writev(fd, vec, sizeof(vec) / sizeof(vec[0]));

    if (written <= 0)
      return written;

    if (written <= color_code_size) {
      _reset();
      return 0;
    }

    written -= color_code_size;

    if (written <= count) {
      _reset();
      return written;
    }

    written -= count;

    if (written < COL_RESET_SIZE)
      _reset();

    return count;
  }
  else {
    struct iovec vec = { (char *)buf, count };
    return writev(fd, &vec, 1);
  }
}

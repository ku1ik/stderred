#define write ye_olde_write
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>

#undef write

static const char COL_RESET[] = "\x1b[0m";

/*  Foreground colors are in form of 3x, background are 4x */
static const char RED[]     = "\x1b[31m";
static const char GREEN[]   = "\x1b[32m";
static const char YELLOW[]  = "\x1b[33m";
static const char BLUE[]    = "\x1b[34m";
static const char MAGENTA[] = "\x1b[35m";
static const char CYAN[]    = "\x1b[36m";

#define STDERR_COLOR RED

#define STDERR_COLOR_SIZE sizeof(STDERR_COLOR)-1
#define COL_RESET_SIZE sizeof(COL_RESET)-1

ssize_t write(int fd, const void* buf, size_t count) {
  if (fd == 2 && isatty(2)) {
    struct iovec vec[3] = {
      { (char *)STDERR_COLOR, STDERR_COLOR_SIZE },
      { (char *)buf, count },
      { (char *)COL_RESET, COL_RESET_SIZE }
    };

    ssize_t written = writev(fd, vec, sizeof(vec) / sizeof(vec[0]));
    if (written < 0)
      return written;
    else if (written <= STDERR_COLOR_SIZE)
      return 0;

    written -= STDERR_COLOR_SIZE;
    return written >= count ? count : written;
  }
  else {
    struct iovec vec = { (char *)buf, count };
    return writev(fd, &vec, 1);
  }
}

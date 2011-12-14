#define write ye_olde_write
#include <unistd.h>
#include <string.h>
#include <alloca.h>
#include <sys/uio.h>
#include <dlfcn.h>
#include <sys/syscall.h>

#undef write

/* SOURCE: http://deathray.us/code/color_output.html */

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

/* Not including background colors for no good reason */

static struct iovec iov[] = {
  {.iov_base = STDERR_COLOR, .iov_len = STDERR_COLOR_SIZE },
  {.iov_base = NULL, .iov_len = 0 },
  {.iov_base = COL_RESET, .iov_len = COL_RESET_SIZE }
};

int write(int fd, const void* buf, int count) {
  if (fd == 2 && isatty(2)) {
    iov[1].iov_base = buf;
    iov[1].iov_len = count;
    return writev(fd, iov, 3);
  }
  return syscall(SYS_write, fd, buf, count);
}

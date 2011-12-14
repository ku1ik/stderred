#define write ye_olde_write
#include <unistd.h>
#include <string.h>
#ifndef __FreeBSD__
#include <alloca.h>
#else
extern char *alloca(size_t);
#endif

#include <dlfcn.h>

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

static ssize_t (*lol_write) (int, const void *, size_t);

ssize_t write(int fd, const void *buf, size_t count) {
  if (lol_write == NULL) {
    *(void **) (&lol_write) = dlsym(RTLD_NEXT, "write");
    if (lol_write == NULL) {
      /* Could not find the original write, return with error */
      return -1;
    }
  }

  if (fd == 2 && isatty(2)) {
    /* Do crazy nonsense to buf and count */
    int new_count = count + STDERR_COLOR_SIZE + COL_RESET_SIZE;
    void * new_buf = alloca(new_count);
    memcpy(new_buf, STDERR_COLOR, STDERR_COLOR_SIZE);
    memcpy(new_buf + STDERR_COLOR_SIZE, buf, count);
    memcpy(new_buf + STDERR_COLOR_SIZE + count, COL_RESET, COL_RESET_SIZE);
    (*lol_write)(fd, new_buf, new_count);
    return count;
  }
  else {
    return (*lol_write)(fd, buf, count);
  }
}

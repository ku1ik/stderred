#define write ye_olde_write
#include <unistd.h>

#include <dlfcn.h>

#undef write

/* SOURCE: http://deathray.us/code/color_output.html */

static const char COL_RESET[] = "\x1b[0m";

/*  Foreground colors are in form of 3x, bacground are 4x */
static const char RED[]     = "\x1b[31m";
static const char GREEN[]   = "\x1b[32m";
static const char YELLOW[]  = "\x1b[33m";
static const char BLUE[]    = "\x1b[34m";
static const char MAGENTA[] = "\x1b[35m";
static const char CYAN[]    = "\x1b[36m";

/* Not including background colors for no good reason */

int write(int fd, const void* buf, int count) {
  int destination_fd = fd;
  void * libc = dlopen("/lib/libc.so.6", RTLD_LAZY); /* never closed, rofl */
  int (*lol_write) (int, const void *, int);
  *(void **) (&lol_write) = dlsym(libc, "write");

  /* always do the write, but if it was to either 1 or 2 put it on 1 */
  if ( (fd == 1) ||
       (fd == 2)) {
    destination_fd = 1;
  }

  if (fd == 2) /* stderr */ {
    (*lol_write)(destination_fd, RED, sizeof(RED));
  }

  (*lol_write)(destination_fd, buf, count);
  
  if (fd == 2) {
    (*lol_write)(destination_fd, COL_RESET, sizeof(COL_RESET));
  }  
  
}

  

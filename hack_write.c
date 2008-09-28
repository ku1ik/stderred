#define fwrite ye_olde_fwrite
#define fwrite_unlocked ye_olde_fwrite_unlocked
#include <stdio.h>

#include <dlfcn.h>

#undef fwrite
#undef fwrite_unlocked

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

int fwrite(const void *ptr, int size, int nmemb,
	   FILE *stream) {
  
  void * libc = dlopen("/lib/libc.so.6", RTLD_LAZY); /* never closed, rofl */
  int (*lol_fwrite) (const void *, int, int, FILE *);
  *(void **) (&lol_fwrite) = dlsym(libc, "fwrite");
  
  if (stream  == stderr) {
    (*lol_fwrite)(RED, 1, sizeof(RED), stream);
  } else if (stream == stdout) {
    (*lol_fwrite)(BLUE, 1, sizeof(BLUE), stream);
  }

  /* always actually do the write() */
  (*lol_fwrite)(ptr, size, nmemb, stream);
  
  if ( (stream == stderr) ||
       (stream == stdout )) {
    (*lol_fwrite)(COL_RESET, 1, sizeof(COL_RESET), stream);
  }
  
}


int fwrite_unlocked(const void *ptr, int size, int nmemb,
	   FILE *stream) {
  
  void * libc = dlopen("/lib/libc.so.6", RTLD_LAZY); /* never closed, rofl */
  int (*lol_fwrite_unlocked) (const void *, int, int, FILE *);
  *(void **) (&lol_fwrite_unlocked) = dlsym(libc, "fwrite_unlocked");
  
  if (stream  == stderr) {
    (*lol_fwrite_unlocked)(RED, 1, sizeof(RED), stream);
  } else if (stream == stdout) {
    (*lol_fwrite_unlocked)(BLUE, 1, sizeof(BLUE), stream);
  }

  /* always actually do the write() */
  (*lol_fwrite_unlocked)(ptr, size, nmemb, stream);
  
  if ( (stream == stderr) ||
       (stream == stdout )) {
    (*lol_fwrite_unlocked)(COL_RESET, 1, sizeof(COL_RESET), stream);
  }
  
}


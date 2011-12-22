#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <stdarg.h>
#include <errno.h>

#undef write

#undef fwrite
#undef fprintf
#undef fputs
#undef fputc

#ifdef INCLUDE_UNLOCKED
  #undef fwrite_unlocked
  #undef fprintf_unlocked
  #undef fputs_unlocked
  #undef fputc_unlocked
#endif

#if INCLUDE_ERROR
  #undef perror
  #undef error
  #undef error_at_line
#endif

#ifndef __USE_GNU
int asprintf(char **, char *, ...);

#define vasprintf(buffer, format, ap)                       \
({                                                          \
  int ret = 0;                                              \
  int size = 1;                                             \
  int nchars;                                               \
                                                            \
  if ((*buffer = malloc(size)) == NULL) {                   \
    ret = -ENOMEM;                                          \
    goto vas_done;                                          \
  }                                                         \
                                                            \
  nchars = vsnprintf(*buffer, size, format, ap);            \
  va_end(ap);                                               \
                                                            \
  if (nchars >= size) {                                     \
    char *tmpbuff;                                          \
    size = nchars + 1;                                      \
    if ((tmpbuff = realloc(*buffer, size)) == NULL) {       \
      free(*buffer);                                        \
      ret = -ENOMEM;                                        \
      goto vas_done;                                        \
    }                                                       \
    *buffer = tmpbuff;                                      \
                                                            \
    va_start(ap, format);                                   \
    nchars = vsnprintf(*buffer, size, format, ap);          \
  }                                                         \
                                                            \
  if (nchars < 0)                                           \
    ret = nchars;                                           \
  ret = size - 1;                                           \
vas_done:                                                   \
    ret;                                                    \
})
#endif

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

extern char *program_invocation_name;

ssize_t write(int fd, const void* buf, size_t count) {
  if (fd == STDERR_FILENO && isatty(fd)) {
    struct iovec vec[3] = {
      { (char *)STDERR_COLOR, STDERR_COLOR_SIZE },
      { (char *)buf, count },
      { (char *)COL_RESET, COL_RESET_SIZE }
    };

    ssize_t written = writev(fd, vec, sizeof(vec) / sizeof(vec[0]));
    fsync(fd);
    if (written < 0)
      return written;
    else if (written <= STDERR_COLOR_SIZE)
      return 0;

    written -= (STDERR_COLOR_SIZE + COL_RESET_SIZE);
    return written >= count ? count : written;
  }
  else {
    struct iovec vec = { (char *)buf, count };
    return writev(fd, &vec, 1);
  }
}

#if 0
size_t fwrite(const void *data, size_t size, size_t count, FILE *stream) {
    size_t e = 0;
    //flockfile(stream);
    e = write(fileno(stream), data, size * count);
    //funlockfile(stream);
    return e == size * count ? count : e;
}
#endif

int fprintf(FILE *stream, const char *format, ...) {
  int e = 0;
  va_list args;
  va_start(args, format);

  if (stream == stderr) {
    char *buf = NULL;
    vasprintf(&buf, format, args);
    e = write(fileno(stream), buf, strlen(buf));
    free(buf);
  }
  else
    e = vfprintf(stream, format, args);

  va_end(args);
  return e;
}

int fputs(const char *s, FILE *stream) {
    int e = fprintf(stream, "%s", s);
    return e < 0 ? EOF : e;
}

int fputc(int c, FILE *stream) {
    int e = fprintf(stream, "%c", (char)c) < 0 ? EOF : c;
    return e;
}


#ifdef INCLUDE_UNLOCKED
#if 0
size_t fwrite_unlocked(const void *data, size_t size, size_t count, FILE *stream) {
    size_t e = write(fileno_unlocked(stream), data, size * count);
    return e == size * count ? count : e;
}
#endif

int fprintf_unlocked(FILE *stream, const char *format, ...) {
  int e = 0;
  va_list args;
  va_start(args, format);

  if (stream == stderr) {
    char *buf = NULL;
    vasprintf(&buf, format, args);
    e = write(fileno_unlocked(stream), buf, strlen(buf));
    free(buf);
  }
  else
    e = vfprintf(stream, format, args);

  va_end(args);
  return e;
}

int fputs_unlocked(const char *s, FILE* stream) {
    int e = fprintf_unlocked(stream, "%s", s);
    return e < 0 ? EOF : e;
}

int fputc_unlocked(int c, FILE *stream) {
    return fprintf_unlocked(stream, "%c", (char)c) < 0 ? EOF : c;
}
#endif


#ifdef INCLUDE_ERROR
void perror(const char *message) {
    char *buf = NULL;
    if (message == NULL)
      asprintf(&buf, "%s\n", strerror(errno));
    else
      asprintf(&buf, "%s: %s\n", message, strerror(errno));
    write(STDERR_FILENO, buf, strlen(buf));
    free(buf);
}

void error(int status, int errnum, const char *format, ...) {
  char *buf = NULL;
  char *msg = NULL;
  va_list args;
  va_start(args, format);

  vasprintf(&msg, format, args);
  asprintf(&buf, "%s: %s: %s\n", program_invocation_name, msg, strerror(errnum));
  write(STDERR_FILENO, buf, strlen(buf));

  free(msg);
  free(buf);
  va_end(args);
}

void error_at_line(int status, int errnum, const char *fname, unsigned int lineno, const char *format, ...) {
  char *buf = NULL;
  char *msg = NULL;
  va_list args;
  va_start(args, format);

  vasprintf(&msg, format, args);
  asprintf(&buf, "%s:%s:%u %s: %s\n", program_invocation_name, fname, lineno, msg, strerror(errnum));
  write(STDERR_FILENO, buf, strlen(buf));

  free(msg);
  free(buf);
  va_end(args);
}
#endif


#ifndef __USE_GNU
/*
 * Copyright (C) 2001 Federico Di Gregorio <fog@debian.org> 
 * Copyright (C) 1991, 1994-1999, 2000, 2001 Free Software Foundation, Inc.
 *
 * This code has been derived from an example in the glibc2 documentation.
 * This file is part of the psycopg module.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

int asprintf(char **buffer, char *format, ...) {
  /* guess we need no more than 200 chars of space */
  int size = 200;
  int nchars;
  va_list ap;

  if ((*buffer = malloc(size)) == NULL)
    return -ENOMEM;

  /* try to print in the allocated space */
  va_start(ap, format);
  nchars = vsnprintf(*buffer, size, format, ap);
  va_end(ap);

  if (nchars >= size) {
    char *tmpbuff;
    /* reallocate buffer now that we know how much space is needed */
    size = nchars + 1;
    if ((tmpbuff = realloc(*buffer, size)) == NULL) {
      /* we need to free it */
      free(buffer);
      return -ENOMEM;
    }
    *buffer = tmpbuff;
    /* try again */
    va_start(ap, format);
    nchars = vsnprintf(*buffer, size, format, ap);
    va_end(ap);
  }

  if (nchars < 0)
    return nchars;
  return size;
}
#endif

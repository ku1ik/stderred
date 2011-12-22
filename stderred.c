#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <stdarg.h>
#include <errno.h>
#include <dlfcn.h>

#undef write
#undef fputc
#undef fputs
#undef fprintf
#undef fwrite
#undef fputc_unlocked
#undef fputs_unlocked
#undef fprintf_unlocked
#undef fwrite_unlocked
#undef perror
#undef error
#undef error_at_line

#ifndef __USE_GNU
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

void _reset() {
  struct iovec vec = { (char *)COL_RESET, COL_RESET_SIZE };
  writev(STDERR_FILENO, &vec, 1);
}

ssize_t write(int fd, const void* buf, size_t count) {
  static ssize_t (*x_write)(int, const void *, size_t);
  if (!x_write)
    *(void **)(&x_write) = dlsym(RTLD_NEXT, "write");
  if (!x_write)
    return ENOSYS;

  if (fd != STDERR_FILENO || !isatty(fd))
      return x_write(fd, buf, count);

  {
    struct iovec vec[3] = {
      { (char *)STDERR_COLOR, STDERR_COLOR_SIZE },
      { (char *)buf, count },
      { (char *)COL_RESET, COL_RESET_SIZE }
    };
    ssize_t written = writev(fd, vec, sizeof(vec) / sizeof(vec[0]));
    if (written <= 0)
      return written;
    if (written <= STDERR_COLOR_SIZE) {
      _reset();
      return 0;
    }
    written -= STDERR_COLOR_SIZE;
    if (written <= count) {
      _reset();
      return written;
    }
    written -= count;
    if (written < COL_RESET_SIZE)
      _reset();
    return count;
  }
}

/*
 * standard stream output functions
 */

int fputc(int chr, FILE *stream) {
  static int (*x_fputc)(int, FILE *);
  if (!x_fputc)
    *(void **)(&x_fputc) = dlsym(RTLD_NEXT, "fputc");
  if (!x_fputc)
    return EOF;

  if (stream != stderr)
    return x_fputc(chr, stream);

  {
    const unsigned char c[] = { (unsigned char)chr };
    return (write(fileno(stream), c, sizeof c) == sizeof c) ? chr : EOF;
  }
}

int fputs(const char *str, FILE *stream) {
  static int (*x_fputs)(const char *, FILE *);
  if (!x_fputs)
    *(void **)(&x_fputs) = dlsym(RTLD_NEXT, "fputs");
  if (!x_fputs)
    return EOF;

  if (stream != stderr)
    return x_fputs(str, stream);

  {
    const size_t len = strlen(str);
    return (write(fileno(stream), str, len) == len) ? len : EOF;
  }
}

int fprintf(FILE *stream, const char *format, ...) {
  static int (*x_fprintf)(FILE *, const char *, ...);
  char *buf = NULL;
  int err;
  va_list args;

  if (!x_fprintf)
    *(void **)(&x_fprintf) = dlsym(RTLD_NEXT, "fprintf");
  if (!x_fprintf)
    return ENOSYS;

  va_start(args, format);
  vasprintf(&buf, format, args);
  if (stream != stderr)
    err = x_fprintf(stream, buf);
  else
    err = write(fileno(stream), buf, strlen(buf));
  free(buf);
  va_end(args);
  return err;
}

size_t fwrite(const void *data, size_t size, size_t count, FILE *stream) {
  static size_t (*x_fwrite)(const void *, size_t, size_t, FILE *);
  if (!x_fwrite)
    *(void **)(&x_fwrite) = dlsym(RTLD_NEXT, "fwrite");
  if (!x_fwrite)
    return ENOSYS;

  if (stream != stderr)
    return x_fwrite(data, size, count, stream);

  return write(fileno(stream), data, size * count);
}

/*
 * "unlocked" stream output functions
 */

int fputc_unlocked(int chr, FILE *stream) {
  static int (*x_fputc_unlocked)(int, FILE *);
  if (!x_fputc_unlocked)
    *(void **)(&x_fputc_unlocked) = dlsym(RTLD_NEXT, "fputc_unlocked");
  if (!x_fputc_unlocked)
    return EOF;

  if (stream != stderr)
    return x_fputc_unlocked(chr, stream);

  {
    const unsigned char c[] = { (unsigned char)chr };
    return (write(fileno_unlocked(stream), c, sizeof c) == sizeof c) ? chr : EOF;
  }
}

int fputs_unlocked(const char *str, FILE *stream) {
  static int (*x_fputs_unlocked)(const char *, FILE *);
  if (!x_fputs_unlocked)
    *(void **)(&x_fputs_unlocked) = dlsym(RTLD_NEXT, "fputs_unlocked");
  if (!x_fputs_unlocked)
    return EOF;

  if (stream != stderr)
    return x_fputs_unlocked(str, stream);

  {
    const size_t len = strlen(str);
    return (write(fileno_unlocked(stream), str, len) == len) ? len : EOF;
  }
}

int fprintf_unlocked(FILE *stream, const char *format, ...) {
  static int (*x_fprintf_unlocked)(FILE *, const char *, ...);
  char *buf = NULL;
  int err;
  va_list args;

  if (!x_fprintf_unlocked)
    *(void **)(&x_fprintf_unlocked) = dlsym(RTLD_NEXT, "fprintf_unlocked");
  if (!x_fprintf_unlocked)
    return ENOSYS;

  va_start(args, format);
  vasprintf(&buf, format, args);
  if (stream != stderr)
    err = x_fprintf_unlocked(stream, buf);
  else
    err = write(fileno_unlocked(stream), buf, strlen(buf));
  free(buf);
  va_end(args);
  return err;
}

size_t fwrite_unlocked(const void *data, size_t size, size_t count, FILE *stream) {
  static size_t (*x_fwrite_unlocked)(const void *, size_t, size_t, FILE *);
  if (!x_fwrite_unlocked)
    *(void **)(&x_fwrite_unlocked) = dlsym(RTLD_NEXT, "fwrite_unlocked");
  if (!x_fwrite_unlocked)
    return ENOSYS;

  if (stream != stderr)
    return x_fwrite_unlocked(data, size, count, stream);

  return write(fileno_unlocked(stream), data, size * count);
}

/*
 * error reporting functions
 */

void perror(const char *message) {
  if (message == NULL)
    fprintf(stderr, "%s\n", strerror(errno));
  else
    fprintf(stderr, "%s: %s\n", message, strerror(errno));
}

void error(int status, int errnum, const char *format, ...) {
  char *buf = NULL;
  va_list args;
  va_start(args, format);
  vasprintf(&buf, format, args);
  fprintf(stderr, "%s: %s: %s\n", program_invocation_name, buf, strerror(errnum));
  free(buf);
  va_end(args);
}

void error_at_line(int status, int errnum, const char *fname, unsigned int lineno, const char *format, ...) {
  char *buf = NULL;
  va_list args;
  va_start(args, format);
  vasprintf(&buf, format, args);
  fprintf(stderr, "%s:%s:%u %s: %s\n", program_invocation_name, fname, lineno, buf, strerror(errnum));
  free(buf);
  va_end(args);
}

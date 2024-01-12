#include "polyfill.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#ifndef HAVE_FWRITE_UNLOCKED
size_t fwrite_unlocked(const void *ptr, size_t size, size_t n, FILE *stream) {
  int fd = fileno(stream);
  size_t total = 0, written = 0, len = size * n;
  while (len > 0) {
    do {
      written = write(fd, ptr + total, len);
    } while((ssize_t)written < 0 && written == EINTR);

    if (written == (size_t)-1) break;
    if (written == 0) {
      errno = ENOSPC;
      return -1;
    }

    total += written;
    len -= written;
  }

  return total;
}
#endif

#ifndef HAVE_FPUTC_UNLOCKED
int fputc_unlocked(int c, FILE *stream) {
  return fputc(c, stream);
}
#endif

#ifndef HAVE_FPUTS_UNLOCKED
int fputs_unlocked(const char *s, FILE *stream) {
  return fputs(s, stream);
}
#endif

#ifndef HAVE_FPRINTF_UNLOCKED
int fprintf_unlocked(FILE *stream, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int status = vfprintf(stream, format, ap);
  va_end(ap);
  return status;
}
#endif

#ifndef HAVE_ERROR
void error(int status, int errnum, const char *format, ...) {
  fflush(stdout);

  fprintf(stderr, "%s: ", PROGRAM_NAME);
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  if (errnum) fprintf(stderr, ": %s\n", strerror(errnum));
  if (status) exit(status);
}
#endif

#ifndef HAVE_ERROR_AT_LINE
void error_at_line(int status, int errnum, const char *filename,
    unsigned int linenum, const char *format, ...) {
  fflush(stdout);

  fprintf(stderr, "%s:%s:%u: ", PROGRAM_NAME, filename, linenum);
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  if (errnum) fprintf(stderr, ": %s\n", strerror(errnum));
  if (status) exit(status);
}
#endif

#ifndef HAVE_ERR_SET_FILE
void *err_output = NULL;
void err_set_file(void *fp) {
  err_output = fp;
}
#define ERR_OUTPUT err_output ? err_output : stderr
#else
  #define ERR_OUTPUT stderr
#endif

#ifndef HAVE_ERR
void err(int eval, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verrc(eval, errno, fmt, ap);
  va_end(ap);
}
#endif

#ifndef HAVE_VERR
void verr(int eval, const char *fmt, va_list args) {
  verrc(eval, errno, fmt, args);
}
#endif

#ifndef HAVE_ERRC
void errc(int eval, int code, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verrc(eval, code, fmt, ap);
  va_end(ap);
}
#endif

#ifndef HAVE_VERRC
void verrc(int eval, int code, const char *fmt, va_list args) {
  vwarnc(code, fmt, args);
  exit(eval);
}
#endif

#ifndef HAVE_ERRX
void errx(int eval, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verrx(eval, fmt, ap);
  va_end(ap);
}
#endif

#ifndef HAVE_VERRX
void verrx(int eval, const char *fmt, va_list args) {
  vwarnx(fmt, args);
  exit(eval);
}
#endif

#ifndef HAVE_WARN
void warn(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vwarnc(errno, fmt, ap);
  va_end(ap);
}
#endif

#ifndef HAVE_VWARN
void vwarn(const char *fmt, va_list args) {
  vwarnc(errno, fmt, args);
}
#endif

#ifndef HAVE_WARNC
void warnc(int code, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vwarnc(code, fmt, ap);
  va_end(ap);
}
#endif

#ifndef HAVE_VWARNC
void vwarnc(int code, const char *fmt, va_list args) {
  char *buf1, *buf2 = NULL;

  if (fmt && vasprintf(&buf1, fmt, args)) {
    if (!asprintf(&buf2, "%s: %s", buf1, strerror(code))) buf2 = NULL;
    free(buf1);
  }

  fprintf(ERR_OUTPUT, "%s: %s\n", PROGRAM_NAME, buf2 ? buf2 : "\0");

  if (buf2) free(buf2);
}
#endif

#ifndef HAVE_WARNX
void warnx(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vwarnx(fmt, ap);
  va_end(ap);
}
#endif

#ifndef HAVE_VWARNX
void vwarnx(const char *fmt, va_list args) {
  char *buf = NULL;

  if (fmt) vasprintf(&buf, fmt, ap);

  fprintf(ERR_OUTPUT, "%s: %s\n", PROGRAM_NAME, buf ? buf : "\0");

  if (buf) free(buf);
}
#endif

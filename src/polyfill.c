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


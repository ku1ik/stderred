#include "config.h"
#include "polyfill.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <regex.h>

#ifdef __APPLE__
  #define FUNC(name) _##name
  #define ORIGINAL(name) name
  #define GET_ORIGINAL(...)
#else
  #include <dlfcn.h>
  #define FUNC(name) (name)
  #define ORIGINAL(name) original_##name
  #define GET_ORIGINAL(ret, name, ...) \
    static ret (*ORIGINAL(name))(__VA_ARGS__) = NULL; \
    if (!ORIGINAL(name)) { \
      *(void **)(&ORIGINAL(name)) = dlsym(RTLD_NEXT, #name); \
    } \
    if (!ORIGINAL(name)) { \
      errno = ENOSYS; \
      abort(); \
    }
#endif


char *start_color_code;
size_t start_color_code_size;

char *end_color_code = "\x1b[0m";
size_t end_color_code_size;

#define COLORIZE(fd) (is_valid_env && fd == STDERR_FILENO)
bool is_valid_env = false;

__attribute__((constructor, visibility ("hidden"))) void init() {
  if (!strcmp("bash", PROGRAM_NAME)) return;
  if (!isatty(STDERR_FILENO)) return;

  char *blacklist;
  if ((blacklist = getenv("STDERRED_BLACKLIST"))) {
    regex_t regex;
    if (regcomp(&regex, blacklist, REG_EXTENDED | REG_NOSUB)) return;
    if (!regexec(&regex, PROGRAM_NAME, 0, NULL, 0)) {
      regfree(&regex);
      return;
    }
    regfree(&regex);
  }

  is_valid_env = true;

  start_color_code = getenv("STDERRED_ESC_CODE");
  if (start_color_code == NULL) {
    start_color_code = "\x1b[31m";
  }
  start_color_code_size = strlen(start_color_code);
  end_color_code_size = strlen(end_color_code);
}

ssize_t FUNC(write)(int fd, const void* buf, size_t count) {
  if (!count) return 0;

  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);

  if (COLORIZE(fd)) {
    ssize_t written = ORIGINAL(write)(fd, start_color_code, start_color_code_size);
    if (written <= 0) return written;
    if (written < start_color_code_size) {
      ORIGINAL(write)(fd, end_color_code, end_color_code_size);
      return 0;
    }
  }

  ssize_t written = ORIGINAL(write)(fd, buf, count);

  if (written > 0 && COLORIZE(fd)) {
    ORIGINAL(write)(fd, end_color_code, end_color_code_size);
  }

  return written;
}

size_t FUNC(fwrite_unlocked)(const void *data, size_t size, size_t count, FILE *stream) {
  if (size * count == 0) return 0;

  size_t result;
  int fd = fileno_unlocked(stream);

  GET_ORIGINAL(ssize_t, fwrite_unlocked, const void*, size_t, size_t, FILE *);

  if (COLORIZE(fd)) {
    result = ORIGINAL(fwrite_unlocked)(start_color_code, sizeof(char), start_color_code_size, stream);
    if ((ssize_t)result < 0) return result;
  }

  result = ORIGINAL(fwrite_unlocked)(data, size, count, stream);
  if (result > 0 && COLORIZE(fd)) {
    ORIGINAL(fwrite_unlocked)(end_color_code, sizeof(char), end_color_code_size, stream);
  }

  return result;
}

size_t FUNC(fwrite)(const void *data, size_t size, size_t count, FILE *stream) {
  if (size * count == 0) return 0;

  size_t result;
  int fd = fileno(stream);

  GET_ORIGINAL(ssize_t, fwrite, const void*, size_t, size_t, FILE *);

  if (COLORIZE(fd)) {
    result = ORIGINAL(fwrite)(start_color_code, sizeof(char), start_color_code_size, stream);
    if ((ssize_t)result < 0) return result;
  }

  result = ORIGINAL(fwrite)(data, size, count, stream);
  if (result > 0 && COLORIZE(fd)) {
    ORIGINAL(fwrite)(end_color_code, sizeof(char), end_color_code_size, stream);
  }

  return result;
}

int FUNC(fputc)(int chr, FILE *stream) {
  size_t result;
  int fd = fileno(stream);

  GET_ORIGINAL(int, fputc, int, FILE *);
  GET_ORIGINAL(ssize_t, fwrite, const void*, size_t, size_t, FILE *);

  if (COLORIZE(fd)) {
    result = ORIGINAL(fwrite)(start_color_code, sizeof(char), start_color_code_size, stream);
    if ((ssize_t)result < 0) return result;
  }

  result = ORIGINAL(fputc)(chr, stream);
  if (COLORIZE(fd)) {
    ORIGINAL(fwrite)(end_color_code, sizeof(char), end_color_code_size, stream);
  }

  return result;
}

int FUNC(fputc_unlocked)(int chr, FILE *stream) {
  size_t result;
  int fd = fileno(stream);

  GET_ORIGINAL(int, fputc_unlocked, int, FILE *);
  GET_ORIGINAL(ssize_t, fwrite, const void*, size_t, size_t, FILE *);

  if (COLORIZE(fd)) {
    result = ORIGINAL(fwrite)(start_color_code, sizeof(char), start_color_code_size, stream);
    if ((ssize_t)result < 0) return result;
  }

  result = ORIGINAL(fputc_unlocked)(chr, stream);
  if (COLORIZE(fd)) {
    ORIGINAL(fwrite)(end_color_code, sizeof(char), end_color_code_size, stream);
  }

  return result;
}

int FUNC(fputs)(const char *str, FILE *stream) {
  return FUNC(fwrite)(str, sizeof(char), strlen(str)/sizeof(char), stream);
}

int FUNC(fputs_unlocked)(const char *str, FILE *stream) {
  return FUNC(fwrite_unlocked)(str, sizeof(char), strlen(str)/sizeof(char), stream);
}

int FUNC(vfprintf)(FILE *stream, const char *format, va_list ap) {
  char *buf = NULL;

  int nprinted = vasprintf(&buf, format, ap);
  if (nprinted > 0) {
    int result = FUNC(fwrite)(buf, sizeof(char), nprinted, stream);
    free(buf);
    return result;
  } else {
    return -1;
  }
}

int FUNC(fprintf)(FILE *stream, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int result = FUNC(vfprintf)(stream, format, args);
  va_end(args);
  return result;
}

#ifdef HAVE__FPRINTF_CHK
int FUNC(__fprintf_chk)(FILE *fp, int flag, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int result = FUNC(vfprintf)(fp, format, args);
  va_end(args);
  return result;
}
#endif

int FUNC(fprintf_unlocked)(FILE *stream, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char *buf = NULL;
  int result = -1;

  int nprinted = vasprintf(&buf, format, args);
  if (nprinted > 0) {
    result = FUNC(fwrite_unlocked)(buf, sizeof(char), nprinted, stream);
    free(buf);
  }

  va_end(args);
  return result;
}

void FUNC(perror)(const char *msg) {
  if (msg == NULL) {
    FUNC(fprintf)(stderr, "%s\n", strerror(errno));
  } else {
    FUNC(fprintf)(stderr, "%s: %s\n", msg, strerror(errno));
  }
}

void FUNC(error)(int status, int errnum, const char *format, ...) {
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);
  GET_ORIGINAL(void, error, int, int, const char *, ...);

  fflush(stdout);

  if (COLORIZE(STDERR_FILENO))
    ORIGINAL(write)(STDERR_FILENO, start_color_code, start_color_code_size);

  char *buf;
  va_list args;
  va_start(args, format);
  if (vasprintf(&buf, format, args) > 0) {
    ORIGINAL(error)(0, errnum, "%s", buf);
    free(buf);
  }
  va_end(args);

  if (COLORIZE(STDERR_FILENO))
    ORIGINAL(write)(STDERR_FILENO, end_color_code, end_color_code_size);

  if (status) exit(status);
}

void FUNC(error_at_line)(int status, int errnum, const char *filename, unsigned int linenum, const char *format, ...) {
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);
  GET_ORIGINAL(void, error_at_line, int, int, const char *, unsigned int, const char *, ...);

  fflush(stdout);

  if (COLORIZE(STDERR_FILENO))
    ORIGINAL(write)(STDERR_FILENO, start_color_code, start_color_code_size);

  char *buf;
  va_list args;
  va_start(args, format);
  if (vasprintf(&buf, format, args) > 0) {
    ORIGINAL(error_at_line)(0, errnum, filename, linenum, "%s", buf);
    free(buf);
  }
  va_end(args);

  if (COLORIZE(STDERR_FILENO))
    ORIGINAL(write)(STDERR_FILENO, end_color_code, end_color_code_size);

  if (status) exit(status);
}

int colorize_err_funcs = true;
void FUNC(err_set_file)(void *fp) {
  GET_ORIGINAL(void, err_set_file, void *);
  ORIGINAL(err_set_file)(fp);
  colorize_err_funcs = (fp == NULL && COLORIZE(STDERR_FILENO)) || COLORIZE(fileno((FILE *)fp));
}

void FUNC(vwarn)(const char *fmt, va_list args) {
  GET_ORIGINAL(void, vwarn, const char *, va_list);
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);

  if (colorize_err_funcs)
    ORIGINAL(write)(STDERR_FILENO, start_color_code, start_color_code_size);

  ORIGINAL(vwarn)(fmt, args);

  if (colorize_err_funcs)
    ORIGINAL(write)(STDERR_FILENO, end_color_code, end_color_code_size);
}


void FUNC(vwarnx)(const char *fmt, va_list args) {
  GET_ORIGINAL(void, vwarnx, const char *, va_list);
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);

  if (colorize_err_funcs)
    ORIGINAL(write)(STDERR_FILENO, start_color_code, start_color_code_size);

  ORIGINAL(vwarnx)(fmt, args);

  if (colorize_err_funcs)
    ORIGINAL(write)(STDERR_FILENO, end_color_code, end_color_code_size);
}

void FUNC(vwarnc)(int code, const char *fmt, va_list args) {
  GET_ORIGINAL(void, vwarnc, int, const char *, va_list);
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);

  if (colorize_err_funcs)
    ORIGINAL(write)(STDERR_FILENO, start_color_code, start_color_code_size);

  ORIGINAL(vwarnc)(code, fmt, args);

  if (colorize_err_funcs)
    ORIGINAL(write)(STDERR_FILENO, end_color_code, end_color_code_size);
}

void FUNC(verr)(int eval, const char *fmt, va_list args) {
  FUNC(vwarn)(fmt, args);
  exit(eval);
}

void FUNC(verrc)(int eval, int code, const char *fmt, va_list args) {
  FUNC(vwarnc)(code, fmt, args);
  exit(eval);
}

void FUNC(err)(int eval, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FUNC(verr)(eval, fmt, ap);
  va_end(ap);
  exit(eval); // Added to keep gcc from complaining - never reached
}

void FUNC(errc)(int eval, int code, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FUNC(verrc)(eval, code, fmt, ap);
  va_end(ap);
  exit(eval); // Added to keep gcc from complaining - never reached
}

void FUNC(verrx)(int eval, const char *fmt, va_list args) {
  FUNC(vwarnx)(fmt, args);
  exit(eval);
}

void FUNC(errx)(int eval, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FUNC(verrx)(eval, fmt, ap);
  va_end(ap);
  exit(eval); // Added to keep gcc from complaining - never reached
}

void FUNC(warn)(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FUNC(vwarn)(fmt, ap);
  va_end(ap);
}

void FUNC(warnc)(int code, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FUNC(vwarnc)(code, fmt, ap);
  va_end(ap);
}

void FUNC(warnx)(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FUNC(vwarnx)(fmt, ap);
  va_end(ap);
}

#ifdef HAVE__WRITE_NOCANCEL
ssize_t __write_nocancel(int fd, const void * cbuf, size_t nbyte);

ssize_t FUNC(__write_nocancel)(int fd, const void * cbuf, size_t nbyte) {
  if (nbyte == 0) return 0;

  ssize_t result;

  GET_ORIGINAL(ssize_t, __write_nocancel, int, const void *, size_t);

  if (COLORIZE(fd)) {
    result = ORIGINAL(__write_nocancel)(fd, (const void *)start_color_code, start_color_code_size);
    if (result < 0) return result;
  }

  result = ORIGINAL(__write_nocancel)(fd, cbuf, nbyte);
  if (result > 0 && COLORIZE(fd)) {
    ORIGINAL(__write_nocancel)(fd, (const void *)end_color_code, end_color_code_size);
  }

  return result;
}
#endif

#ifdef __APPLE__
  #define INTERPOSE(name) { (void *)FUNC(name), (void *)name }
  typedef struct { void *new; void *old; } interpose;
  __attribute__((used)) static const interpose interposers[] \
    __attribute__((section("__DATA,__interpose"))) = {
      INTERPOSE(write),
  #ifdef HAVE__WRITE_NOCANCEL
      INTERPOSE(__write_nocancel),
  #endif
      INTERPOSE(fwrite),
      INTERPOSE(fwrite_unlocked),
      INTERPOSE(fputc),
      INTERPOSE(fputc_unlocked),
      INTERPOSE(fputs),
      INTERPOSE(fputs_unlocked),
      INTERPOSE(fprintf),
      INTERPOSE(fprintf_unlocked),
      INTERPOSE(vfprintf),
      INTERPOSE(perror),
      INTERPOSE(error),
      INTERPOSE(error_at_line),
      INTERPOSE(err),
      INTERPOSE(verr),
      INTERPOSE(errc),
      INTERPOSE(verrc),
      INTERPOSE(errx),
      INTERPOSE(verrx),
      INTERPOSE(warn),
      INTERPOSE(vwarn),
      INTERPOSE(warnc),
      INTERPOSE(vwarnc),
      INTERPOSE(warnx),
      INTERPOSE(vwarnx),
      INTERPOSE(err_set_file),
  };
#endif

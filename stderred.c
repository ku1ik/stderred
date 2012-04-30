#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#ifdef __APPLE__
  // This macro was taken from
  // http://www.mikeash.com/pyblog/friday-qa-2009-01-30-code-injection.html#comment-3fb6e4b8cf65ec984e7836e2b86a2875
  #define DYLD_INTERPOSE(_replacment,_replacee) \
    __attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
    __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };
  #define FUNC(name) _##name
  #define ORIGINAL(name) name
  #define GET_ORIGINAL(...)
#else
  #include <dlfcn.h>
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
  #define FUNC(name) name
#endif


#define RESET() { \
  if (start_color_code_size > 0 && end_color_code_size > 0) { \
    ORIGINAL(write)(STDERR_FILENO, end_color_code, end_color_code_size); \
  } \
}

char *start_color_code;
size_t start_color_code_size;

char *end_color_code;
size_t end_color_code_size;
#define GET_COLOR_CODE() { \
  if (start_color_code == NULL) { \
    start_color_code = getenv("STDERRED_ESC_CODE"); \
    if (start_color_code == NULL) { \
      start_color_code = "\x1b[31m"; \
    } \
    start_color_code_size = strlen(start_color_code); \
  } \
  if (end_color_code == NULL) { \
    end_color_code = getenv("STDERRED_END_CODE"); \
    if (end_color_code == NULL) { \
      end_color_code = "\x1b[0m"; \
    } \
    end_color_code_size = strlen(end_color_code); \
  } \
}

ssize_t FUNC(write)(int fd, const void* buf, size_t count) {
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);

  if (fd != STDERR_FILENO || !isatty(fd)) {
    return ORIGINAL(write)(fd, buf, count);
  }

  GET_COLOR_CODE();
  if (start_color_code_size > 0 && count > 0) {
    ssize_t written = ORIGINAL(write)(fd, start_color_code, start_color_code_size);
    if (written <= 0) return written;
    if (written < start_color_code_size) {
      RESET();
      return 0;
    }
  }

  ssize_t written = ORIGINAL(write)(fd, buf, count);
  if (written > 0) RESET();
  return written;
}

#ifdef HAVE_FWRITE_UNLOCKED
size_t FUNC(fwrite_unlocked)(const void *data, size_t size, size_t count, FILE *stream) {
  ssize_t result;
  int is_stderr = stream == stderr && isatty(fileno_unlocked(stream)) ? 1 : 0;

  GET_ORIGINAL(ssize_t, fwrite_unlocked, const void*, size_t, size_t, FILE *);
  GET_COLOR_CODE();

  if (is_stderr && start_color_code_size > 0 && size * count > 0) {
    result = ORIGINAL(fwrite_unlocked)(start_color_code, sizeof(char), start_color_code_size, stream);
    if (result < 0) return result;
  }

  result = ORIGINAL(fwrite_unlocked)(data, size, count, stream);
  if (result > 0 && is_stderr && start_color_code_size > 0 && end_color_code_size > 0)
    ORIGINAL(fwrite_unlocked)(end_color_code, sizeof(char), end_color_code_size, stream);
  return result;
}
#endif

size_t FUNC(fwrite)(const void *data, size_t size, size_t count, FILE *stream) {
  ssize_t result;
  int is_stderr = stream == stderr && isatty(fileno(stream)) ? 1 : 0;

  GET_ORIGINAL(ssize_t, fwrite, const void*, size_t, size_t, FILE *);
  GET_COLOR_CODE();

  if (is_stderr && start_color_code_size > 0 && size * count > 0) {
    result = ORIGINAL(fwrite)(start_color_code, sizeof(char), start_color_code_size, stream);
    if (result < 0) return result;
  }

  result = ORIGINAL(fwrite)(data, size, count, stream);
  if (result > 0 && is_stderr && start_color_code_size > 0 && end_color_code_size > 0)
    ORIGINAL(fwrite)(end_color_code, sizeof(char), end_color_code_size, stream);
  return result;
}

int FUNC(fputc)(int chr, FILE *stream) {
  const unsigned char c[] = { (unsigned char)chr };
  return FUNC(fwrite)(c, sizeof(char), sizeof(char), stream);
}

int FUNC(fputs)(const char *str, FILE *stream) {
  return FUNC(fwrite)(str, sizeof(char), strlen(str)/sizeof(char), stream);
}

int FUNC(vfprintf)(FILE *stream, const char *format, va_list ap) {
  char *buf = NULL;

  if (vasprintf(&buf, format, ap) > 0) {
    int result = FUNC(fwrite)(buf, sizeof(char), strlen(buf)/sizeof(char), stream);
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

void FUNC(perror)(const char *msg) {
  if (msg == NULL) {
    FUNC(fprintf)(stderr, "%s\n", strerror(errno));
  } else {
    FUNC(fprintf)(stderr, "%s: %s\n", msg, strerror(errno));
  }
}


#ifdef HAVE_ERROR
void FUNC(error)(int status, int errnum, const char *format, ...) {
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);
  GET_ORIGINAL(void, error, int, int, const char *);
  fflush(stdout);
  if (start_color_code_size > 0)
    ORIGINAL(write)(STDERR_FILENO, start_color_code, start_color_code_size);
  ORIGINAL(error)(0, errnum, format);
  RESET();
  if (status) exit(status);
}
#endif

#ifdef HAVE_ERROR_AT_LINE
void FUNC(error_at_line)(int status, int errnum, const char *filename, unsigned int linenum, const char *format, ...) {
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);
  GET_ORIGINAL(void, error_at_line, int, int, const char *, unsigned int, const char *);
  fflush(stdout);
  if (start_color_code_size > 0)
    ORIGINAL(write)(STDERR_FILENO, start_color_code, start_color_code_size);
  ORIGINAL(error_at_line)(0, errnum, filename, linenum, format);
  RESET();
  if (status) exit(status);
}
#endif

#ifdef DYLD_INTERPOSE
  DYLD_INTERPOSE(FUNC(write), write);
  DYLD_INTERPOSE(FUNC(fputc), fputc);
  DYLD_INTERPOSE(FUNC(fputs), fputs);
  DYLD_INTERPOSE(FUNC(fprintf), fprintf);
  DYLD_INTERPOSE(FUNC(fwrite), fwrite);
  DYLD_INTERPOSE(FUNC(vfprintf), vfprintf);
  DYLD_INTERPOSE(FUNC(perror), perror);
#endif

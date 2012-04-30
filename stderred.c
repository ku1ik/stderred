#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

static const char RED[]       = "\x1b[31m";
static const char COL_RESET[] = "\x1b[0m";
#define COL_RESET_SIZE sizeof(COL_RESET)-1

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
      return -1; \
    }
  #define FUNC(name) name
#endif

#ifdef __BSD__
  #define PROGNAME getprogname()
#else
  extern char *program_invocation_name;
  #define PROGNAME program_invocation_name
#endif

char *color_code;
int color_code_size;

#define RESET() ORIGINAL(write)(STDERR_FILENO, COL_RESET, COL_RESET_SIZE)

#define GET_COLOR_CODE() { \
  if (color_code == NULL) { \
    color_code = getenv("STDERRED_ESC_CODE"); \
    if (color_code == NULL) { \
      color_code = (char *)RED; \
    } \
    color_code_size = strlen(color_code); \
  } \
}

ssize_t FUNC(write)(int fd, const void* buf, size_t count) {
  GET_ORIGINAL(ssize_t, write, int, const void *, size_t);

  if (fd != STDERR_FILENO || !isatty(fd)) {
    return ORIGINAL(write)(fd, buf, count);
  }

  GET_COLOR_CODE();

  ssize_t written;
  if (color_code_size > 0) {
    written = ORIGINAL(write)(fd, color_code, color_code_size);
    if (written <= 0) return written;

    if (written < color_code_size) {
      RESET();
      return 0;
    }
  }

  written = ORIGINAL(write)(fd, buf, count);
  if (written > 0 && color_code_size > 0) RESET();
  return written;
}

size_t FUNC(fwrite)(const void *data, size_t size, size_t count, FILE *stream) {
  ssize_t result;
  int is_stderr = stream == stderr && isatty(fileno(stream)) ? 1 : 0;

  GET_ORIGINAL(ssize_t, fwrite, const void*, size_t, size_t, FILE *);
  GET_COLOR_CODE();

  if (is_stderr && color_code_size > 0) {
    result = ORIGINAL(fwrite)(color_code, sizeof(char), color_code_size, stream);
    if (result < 0) return result;
  }

  result = ORIGINAL(fwrite)(data, size, count, stream);
  if (result > 0 && is_stderr && color_code_size > 0)
    ORIGINAL(fwrite)(COL_RESET, sizeof(char), COL_RESET_SIZE, stream);
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

#ifdef DYLD_INTERPOSE
  DYLD_INTERPOSE(FUNC(write), write);
  DYLD_INTERPOSE(FUNC(fputc), fputc);
  DYLD_INTERPOSE(FUNC(fputs), fputs);
  DYLD_INTERPOSE(FUNC(fprintf), fprintf);
  DYLD_INTERPOSE(FUNC(fwrite), fwrite);
  DYLD_INTERPOSE(FUNC(vfprintf), vfprintf);
  DYLD_INTERPOSE(FUNC(perror), perror);
#endif

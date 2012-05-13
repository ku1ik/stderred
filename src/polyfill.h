#ifndef POLYFIL_H_WFSH9MOG
#define POLYFIL_H_WFSH9MOG

#include "config.h"

#ifndef HAVE_FWRITE_UNLOCKED
size_t fwrite_unlocked(const void *ptr, size_t size, size_t n, FILE *stream); 
#endif

#ifndef HAVE_FPUTC_UNLOCKED
int fputc_unlocked(int c, FILE *stream); 
#endif

#ifndef HAVE_FPUTS_UNLOCKED
int fputs_unlocked(const char *s, FILE *stream);
#endif

#ifndef HAVE_FPRINTF_UNLOCKED
int fprintf_unlocked(FILE *stream, const char *format, ...);
#endif

#ifndef HAVE_ERROR
void error(int status, int errnum, const char *format, ...);
#endif

#ifndef HAVE_ERROR_AT_LINE
void error_at_line(int status, int errnum, const char *filename,
    unsigned int linenum, const char *format, ...);
#endif

#ifndef HAVE_ERR
void err(int eval, const char *fmt, ...);
#endif

#ifndef HAVE_VERR
void verr(int eval, const char *fmt, va_list args);
#endif

#ifndef HAVE_ERRC
void errc(int eval, int code, const char *fmt, ...);
#endif

#ifndef HAVE_VERRC
void verrc(int eval, int code, const char *fmt, va_list args);
#endif

#ifndef HAVE_ERRX
void errx(int eval, const char *fmt, ...);
#endif

#ifndef HAVE_VERRX
void verrx(int eval, const char *fmt, va_list args);
#endif

#ifndef HAVE_WARN
void warn(const char *fmt, ...);
#endif

#ifndef HAVE_VWARN
void vwarn(const char *fmt, va_list args);
#endif

#ifndef HAVE_WARNC
void warnc(int code, const char *fmt, ...);
#endif

#ifndef HAVE_VWARNC
void vwarnc(int code, const char *fmt, va_list args);
#endif

#ifndef HAVE_WARNX
void warnx(const char *fmt, ...);
#endif

#ifndef HAVE_VWARNX
void vwarnx(const char *fmt, va_list args);
#endif

#ifndef HAVE_ERR_SET_FILE
void err_set_file(void *fp);
#endif

#endif /* end of include guard: POLYFIL_H_WFSH9MOG */


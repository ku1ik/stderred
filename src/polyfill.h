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

#endif /* end of include guard: POLYFIL_H_WFSH9MOG */


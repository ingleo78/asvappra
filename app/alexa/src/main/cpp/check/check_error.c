#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <setjmp.h>
#include "libcompat.h"
#include "check_error.h"

jmp_buf error_jmp_buffer;
void eprintf(const char *fmt, const char *file, int line, ...) {
    va_list args;
    fflush(stderr);
    fprintf(stderr, "%s:%d: ", file, line);
    va_start(args, line);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if(fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':') fprintf(stderr, " %s", strerror(errno));
    fprintf(stderr, "\n");
    exit(2);
}
void *emalloc(size_t n) {
    void *p;
    p = malloc(n);
    if(p == NULL) eprintf("malloc of " CK_FMT_ZU " bytes failed:", __FILE__, __LINE__ - 2, n);
    return p;
}
void *erealloc(void *ptr, size_t n) {
    void *p;
    p = realloc(ptr, n);
    if(p == NULL) eprintf("realloc of " CK_FMT_ZU " bytes failed:", __FILE__, __LINE__ - 2, n);
    return p;
}
#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>
#include "libcompat.h"

extern jmp_buf error_jmp_buffer;
void eprintf(const char *fmt, const char *file, int line, ...) CK_ATTRIBUTE_NORETURN CK_ATTRIBUTE_FORMAT(printf, 1, 4);
void *emalloc(size_t n);
void *erealloc(void *, size_t n);

#endif
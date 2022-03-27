#ifndef _VASNPRINTF_H
#define _VASNPRINTF_H

#include <stdarg.h>
#include <stddef.h>

typedef unsigned int size_t;
#ifndef __attribute__
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#  define __attribute__(Spec) /* empty */
# endif
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif
#ifdef	__cplusplus
extern "C" {
#endif
extern char *asnprintf(char *resultbuf, size_t *lengthp, const char *format, ...) __attribute__((__format__(__printf__, 3, 4)));
extern char *vasnprintf(char *resultbuf, size_t *lengthp, const char *format, va_list args) __attribute__((__format__(__printf__, 3, 0)));
#ifdef	__cplusplus
}
#endif

#endif
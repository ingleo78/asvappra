#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdarg.h>
#include "g-gnulib.h"
#include "vasnprintf.h"

char *asnprintf(char *resultbuf, size_t *lengthp, const char *format, ...) {
  va_list args;
  char *result;
  va_start(args, format);
  result = vasnprintf(resultbuf, lengthp, format, args);
  va_end(args);
  return result;
}

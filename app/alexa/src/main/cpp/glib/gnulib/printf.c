#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "g-gnulib.h"
#include "vasnprintf.h"
#include "printf.h"

int _g_gnulib_printf(char const *format, ...) {
  va_list args;
  int retval;
  va_start(args, format);
  retval = _g_gnulib_vprintf(format, args);
  va_end (args);
  return retval;
}
int _g_gnulib_fprintf(FILE *file, char const *format, ...) {
  va_list args;
  int retval;
  va_start(args, format);
  retval = _g_gnulib_vfprintf(file, format, args);
  va_end(args);
  return retval;
}
int _g_gnulib_sprintf(char *string, char const *format, ...) {
  va_list args;
  int retval;
  va_start(args, format);
  retval = _g_gnulib_vsprintf(string, format, args);
  va_end(args);
  return retval;
}
int _g_gnulib_snprintf(char *string, size_t n, char const *format, ...) {
  va_list args;
  int retval;
  va_start(args, format);
  retval = _g_gnulib_vsnprintf(string, n, format, args);
  va_end(args);
  return retval;
}
int _g_gnulib_vprintf(char const *format, va_list args) {
  return _g_gnulib_vfprintf(stdout, format, args);
}
int _g_gnulib_vfprintf(FILE *file, char const *format, va_list args) {
  char *result;
  size_t length;
  result = vasnprintf(NULL, &length, format, args);
  if (result == NULL) return -1;
  fwrite(result, 1, length, file);
  free(result);
  return length;
}
int _g_gnulib_vsprintf(char *string, char const *format, va_list args) {
  char *result;
  size_t length;
  result = vasnprintf(NULL, &length, format, args);
  if (result == NULL) return -1;
  memcpy(string, result, length + 1);
  free(result);
  return length;  
}
int _g_gnulib_vsnprintf(char *string, size_t n, char const *format, va_list args) {
  char *result;
  size_t length;
  result = vasnprintf(NULL, &length, format, args);
  if (result == NULL) return -1;
  if (n > 0) {
      memcpy(string, result, MIN(length + 1, n));
      string[n - 1] = 0;
  }
  free(result);
  return length;  
}
int _g_gnulib_vasprintf(char **result, char const *format, va_list args) {
  size_t length;
  *result = vasnprintf(NULL, &length, format, args);
  if (*result == NULL) return -1;
  return length;  
}






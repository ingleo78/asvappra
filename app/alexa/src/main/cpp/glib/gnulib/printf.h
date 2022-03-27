#ifndef __GNULIB_PRINTF_H__
#define __GNULIB_PRINTF_H__

#include <stdarg.h>
#include <stdio.h>

typedef unsigned int size_t;
int _g_gnulib_printf(char const *format,  ...);
int _g_gnulib_fprintf(FILE *file, char const *format, ...);
int _g_gnulib_sprintf(char *string, char const *format, ...);
int _g_gnulib_snprintf(char *string, size_t n, char const *format, ...);
int _g_gnulib_vprintf(char const *format, va_list args);
int _g_gnulib_vfprintf(FILE *file, char const *format, va_list args);
int _g_gnulib_vsprintf(char *string, char const *format, va_list args);
int _g_gnulib_vsnprintf(char *string, size_t n, char const *format, va_list args);
int _g_gnulib_vasprintf (char **result, char const *format, va_list args);

#endif
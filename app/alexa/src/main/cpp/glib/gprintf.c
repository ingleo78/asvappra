#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "gprintf.h"
#include "gprintfint.h"

gint g_printf(gchar const *format, ...) {
  va_list args;
  gint retval;
  va_start(args, format);
  retval = g_vprintf(format, args);
  va_end(args);
  return retval;
}
gint g_fprintf(FILE *file, gchar const *format, ...) {
  va_list args;
  gint retval;
  va_start(args, format);
  retval = g_vfprintf(file, format, args);
  va_end(args);
  return retval;
}
gint g_sprintf(gchar *string, gchar const *format, ...) {
  va_list args;
  gint retval;
  va_start(args, format);
  retval = g_vsprintf(string, format, args);
  va_end(args);
  return retval;
}
gint g_snprintf(gchar *string, gulong n, gchar const *format, ...) {
  va_list args;
  gint retval;
  va_start(args, format);
  retval = g_vsnprintf(string, n, format, args);
  va_end(args);
  return retval;
}
gint g_vprintf(gchar const *format, va_list args) {
  g_return_val_if_fail(format != NULL, -1);
  return _g_vprintf(format, args);
}
gint g_vfprintf(FILE *file, gchar const *format, va_list args) {
  g_return_val_if_fail(format != NULL, -1);
  return _g_vfprintf(file, format, args);
}
gint g_vsprintf(gchar *string, gchar const *format, va_list args) {
  g_return_val_if_fail(string != NULL, -1);
  g_return_val_if_fail(format != NULL, -1);
  return _g_vsprintf(string, format, args);
}
gint
g_vsnprintf (gchar *string, gulong n, gchar const *format, va_list args) {
  g_return_val_if_fail(n == 0 || string != NULL, -1);
  g_return_val_if_fail(format != NULL, -1);
  return _g_vsnprintf(string, n, format, args);
}
gint g_vasprintf(gchar **string, gchar const *format, va_list args) {
  gint len;
  g_return_val_if_fail(string != NULL, -1);
#if !defined(HAVE_GOOD_PRINTF)
  len = _g_gnulib_vasprintf(string, format, args);
  if (len < 0) *string = NULL;
#elif defined(HAVE_VASPRINTF)
  len = vasprintf (string, format, args);
  if (len < 0) *string = NULL;
  else if (!g_mem_is_system_malloc ()) {
      gchar *string1 = g_strndup(*string, len);
      free(*string);
      *string = string1;
  }
#else
  {
      va_list args2;
      G_VA_COPY(args2, args);
      *string = g_new(gchar, g_printf_string_upper_bound (format, args));
      len = _g_vsprintf(*string, format, args2);
      va_end(args2);
  }
#endif
  return len;
}
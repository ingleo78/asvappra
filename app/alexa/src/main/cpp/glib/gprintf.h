#ifndef __G_PRINTF_H__
#define __G_PRINTF_H__

#include <stdio.h>
#include <stdarg.h>
#include "gmacros.h"
#include "gtypes.h"
#include "glib.h"

G_BEGIN_DECLS
gint g_printf(gchar const *format, ...) G_GNUC_PRINTF (1, 2);
gint g_fprintf(FILE *file, gchar const *format, ...) G_GNUC_PRINTF (2, 3);
gint g_sprintf(gchar *string, gchar const *format,...) G_GNUC_PRINTF (2, 3);
gint g_vprintf(gchar const *format, va_list args);
gint g_vfprintf(FILE *file, gchar const *format, va_list args);
gint g_vsprintf(gchar *string, gchar const *format, va_list args);
gint g_vasprintf (gchar **string, gchar const *format, va_list args);
G_END_DECLS

#endif
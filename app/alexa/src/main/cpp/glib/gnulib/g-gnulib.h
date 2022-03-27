#ifndef __G_GNULIB_H__

#include <stdlib.h>
#include "../glib.h"

#define asnprintf        _g_gnulib_asnprintf
#define vasnprintf       _g_gnulib_vasnprintf
#define printf_parse     _g_gnulib_printf_parse
#define printf_fetchargs _g_gnulib_printf_fetchargs
#undef malloc
#undef realloc
#undef free
#define malloc  g_malloc
#define realloc g_realloc
#define free    g_free
#undef HAVE_SNPRINTF
#ifdef HAVE_C99_SNPRINTF
#define HAVE_SNPRINTF 1
#else
#define HAVE_SNPRINTF 0
#endif

#endif  /* __G_GNULIB_H__ */


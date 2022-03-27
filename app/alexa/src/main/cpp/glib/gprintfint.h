#ifndef __G_PRINTFINT_H__
#define __G_PRINTFINT_H__

#ifdef HAVE_GOOD_PRINTF
#define _g_printf    printf
#define _g_fprintf   fprintf
#define _g_sprintf   sprintf
#define _g_snprintf  snprintf
#define _g_vprintf   vprintf
#define _g_vfprintf  vfprintf
#define _g_vsprintf  vsprintf
#define _g_vsnprintf vsnprintf
#else
#include "gnulib/printf.h"
#define _g_printf    _g_gnulib_printf
#define _g_fprintf   _g_gnulib_fprintf
#define _g_sprintf   _g_gnulib_sprintf
#define _g_snprintf  _g_gnulib_snprintf
#define _g_vprintf   _g_gnulib_vprintf
#define _g_vfprintf  _g_gnulib_vfprintf
#define _g_vsprintf  _g_gnulib_vsprintf
#define _g_vsnprintf _g_gnulib_vsnprintf
#endif

#endif
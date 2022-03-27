#ifndef __GST_MACROS_H__
#define __GST_MACROS_H__

#include <glib/glib.h>

G_BEGIN_DECLS
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
# define GST_GNUC_CONSTRUCTOR \
  __attribute__ ((constructor))
#else
# define GST_GNUC_CONSTRUCTOR
#endif
#if defined (__GNUC__) && !defined (GST_IMPLEMENT_INLINES)
# define GST_INLINE_FUNC extern __inline__
# define GST_CAN_INLINE 1
#elif defined(_MSC_VER)
# define GST_INLINE_FUNC extern __inline
# define GST_CAN_INLINE 1
#else
# define GST_INLINE_FUNC extern
# undef GST_CAN_INLINE
#endif
#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L) && !defined(restrict)
#  if defined(__GNUC__) && __GNUC__ >= 4
#    define restrict __restrict__
#  else
#    define restrict
#  endif
#endif
G_END_DECLS

#endif
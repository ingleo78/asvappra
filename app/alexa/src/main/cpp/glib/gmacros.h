#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_MACROS_H__
#define __G_MACROS_H__

#include <stddef.h>
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define G_GNUC_EXTENSION __extension__
#else
#define G_GNUC_EXTENSION
#endif
#if    __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define G_GNUC_PURE __attribute__((__pure__))
#define G_GNUC_MALLOC __attribute__((__malloc__))
#else
#define G_GNUC_PURE
#define G_GNUC_MALLOC
#endif
#if     __GNUC__ >= 4
#define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define G_GNUC_NULL_TERMINATED
#endif
#if     (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#define G_GNUC_ALLOC_SIZE(x) __attribute__((__alloc_size__(x)))
#define G_GNUC_ALLOC_SIZE2(x,y) __attribute__((__alloc_size__(x,y)))
#else
#define G_GNUC_ALLOC_SIZE(x)
#define G_GNUC_ALLOC_SIZE2(x,y)
#endif
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define G_GNUC_PRINTF( format_idx, arg_idx ) __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define G_GNUC_SCANF( format_idx, arg_idx ) __attribute__((__format__ (__scanf__, format_idx, arg_idx)))
#define G_GNUC_FORMAT( arg_idx ) __attribute__((__format_arg__ (arg_idx)))
#define G_GNUC_NORETURN __attribute__((__noreturn__))
#define G_GNUC_CONST __attribute__((__const__))
#define G_GNUC_UNUSED __attribute__((__unused__))
#define G_GNUC_NO_INSTRUMENT __attribute__((__no_instrument_function__))
#define G_GNUC_INTERNAL __attribute__((visibility("hidden")))
#else
#define G_GNUC_PRINTF( format_idx, arg_idx )
#define G_GNUC_SCANF( format_idx, arg_idx )
#define G_GNUC_FORMAT( arg_idx )
#define G_GNUC_NORETURN
#define G_GNUC_CONST
#define G_GNUC_UNUSED
#define G_GNUC_NO_INSTRUMENT
#define G_GNUC_INTERNAL
#endif
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define G_GNUC_DEPRECATED  __attribute__((__deprecated__))
#else
#define G_GNUC_DEPRECATED
#endif
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define G_GNUC_DEPRECATED_FOR(f) __attribute__((deprecated("Use " #f " instead")))
#else
#define G_GNUC_DEPRECATED_FOR(f)  G_GNUC_DEPRECATED
#endif
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)
#define G_GNUC_MAY_ALIAS  __attribute__((may_alias))
#else
#define G_GNUC_MAY_ALIAS
#endif
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#define G_GNUC_WARN_UNUSED_RESULT  __attribute__((warn_unused_result))
#else
#define G_GNUC_WARN_UNUSED_RESULT
#endif
#ifndef G_DISABLE_DEPRECATED
#if defined (__GNUC__) && (__GNUC__ < 3)
#define G_GNUC_FUNCTION  __FUNCTION__
#define G_GNUC_PRETTY_FUNCTION  __PRETTY_FUNCTION__
#else
#define G_GNUC_FUNCTION  ""
#define G_GNUC_PRETTY_FUNCTION  ""
#endif
#endif
#define	G_STRINGIFY_ARG(contents)  #contents
#define G_STRINGIFY(macro_or_string)  G_STRINGIFY_ARG (macro_or_string)
#define G_PASTE_ARGS(identifier1,identifier2) identifier1 ## identifier2
#define G_PASTE(identifier1,identifier2)  G_PASTE_ARGS (identifier1, identifier2)
#define G_STATIC_ASSERT(expr) typedef struct { char Compile_Time_Assertion[(expr) ? 1 : -1]; } G_PASTE (_GStaticAssert_, __LINE__)
#if defined(__GNUC__) && (__GNUC__ < 3) && !defined(__cplusplus)
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__) ":" __PRETTY_FUNCTION__ "()"
#else
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)
#endif
#if defined (__GNUC__)
#  define G_STRFUNC     ((const char*) (__PRETTY_FUNCTION__))
#elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 19901L
#  define G_STRFUNC     ((const char*) (__func__))
#else
#  define G_STRFUNC     ((const char*) ("???"))
#endif
#ifdef  __cplusplus
# define G_BEGIN_DECLS  extern "C" {
# define G_END_DECLS    }
#else
# define G_BEGIN_DECLS
# define G_END_DECLS
#endif
#ifndef NULL
#  ifdef __cplusplus
#    define NULL        (0L)
#  else /* !__cplusplus */
#    define NULL        ((void*) 0)
#  endif /* !__cplusplus */
#endif
#ifndef	FALSE
#define	FALSE	(0)
#endif
#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif
#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))
#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define G_N_ELEMENTS(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define GPOINTER_TO_SIZE(p)	((gsize) (p))
#define GSIZE_TO_POINTER(s)	((gpointer) (gsize) (s))
#if defined(__GNUC__)  && __GNUC__ >= 4
#  define G_STRUCT_OFFSET(struct_type, member) ((glong)offsetof(struct_type, member))
#else
#  define G_STRUCT_OFFSET(struct_type, member)	((glong) ((guint8*) &((struct_type*) 0)->member))
#endif
#define G_STRUCT_MEMBER_P(struct_p, struct_offset) ((gpointer)((guint8*)(struct_p) + (glong)(struct_offset)))
#define G_STRUCT_MEMBER(member_type, struct_p, struct_offset) (*(member_type*)G_STRUCT_MEMBER_P((struct_p), (struct_offset)))
#if !(defined (G_STMT_START) && defined (G_STMT_END))
#define G_STMT_START  do
#define G_STMT_END  while(0)
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__cplusplus)
#define G_ALIGNOF(type) _Alignof(type)
#else
#define G_ALIGNOF(type) (G_STRUCT_OFFSET(struct { char a; type b; }, b))
#endif
#ifdef G_DISABLE_CONST_RETURNS
#define G_CONST_RETURN
#else
#define G_CONST_RETURN const
#endif
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _G_BOOLEAN_EXPR(expr)                   \
 __extension__ ({                               \
   int _g_boolean_var_;                         \
   if (expr)_g_boolean_var_ = 1;                \
   else _g_boolean_var_ = 0;                    \
   _g_boolean_var_;                             \
})
#define G_LIKELY(expr) (__builtin_expect (_G_BOOLEAN_EXPR(expr), 1))
#define G_UNLIKELY(expr) (__builtin_expect (_G_BOOLEAN_EXPR(expr), 0))
#else
#define G_LIKELY(expr) (expr)
#define G_UNLIKELY(expr) (expr)
#endif

#endif
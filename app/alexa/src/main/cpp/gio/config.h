#define BROKEN_POLL 1
#define ENABLE_NLS 1
#define ENABLE_REGEX /**/
#define GETTEXT_PACKAGE "@GETTEXT_PACKAGE@"
#define GLIB_BINARY_AGE @GLIB_BINARY_AGE@
#define GLIB_INTERFACE_AGE @GLIB_INTERFACE_AGE@
#define GLIB_LOCALE_DIR "NONE/share/locale"
#define GLIB_MAJOR_VERSION @GLIB_MAJOR_VERSION@
#define GLIB_MICRO_VERSION @GLIB_MICRO_VERSION@
#define GLIB_MINOR_VERSION @GLIB_MINOR_VERSION@
#define GLIB_SIZEOF_SYSTEM_THREAD 4
#define GIO_MODULE_DIR "/dev/null"
#ifndef _MSC_VER
#define G_ATOMIC_I486 1
#endif
#ifndef _MSC_VER
#define G_HAVE_INLINE 1
#else
/* #undef G_HAVE_INLINE */
#endif
#define G_HAVE___INLINE 1
#if !defined(_MSC_VER) && !defined(__DMC__)
#define G_HAVE___INLINE__ 1
#else
/* #undef G_HAVE___INLINE__ */
#endif
#ifndef _MSC_VER
#define G_VA_COPY va_copy
#else
/* #undef G_VA_COPY */
#endif
#define HAVE_ALLOCA 1
#define HAVE_ATEXIT 1
#define HAVE_BIND_TEXTDOMAIN_CODESET 1
#ifndef _MSC_VER
#define HAVE_DIRENT_H 1
#else
/* #undef HAVE_DIRENT_H */
#endif
#define HAVE_GETCWD 1
#define HAVE_INT64_AND_I64 1
#ifndef _MSC_VER
#define HAVE_INTMAX_T 1
#else
/* #undef HAVE_INTMAX_T */
#endif
#ifndef _MSC_VER
#define HAVE_INTTYPES_H 1
#else
/* #undef HAVE_INTTYPES_H */
#endif
#ifndef _MSC_VER
#define HAVE_INTTYPES_H_WITH_UINTMAX 1
#else
/* #undef HAVE_INTTYPES_H_WITH_UINTMAX */
#endif
#define HAVE_LOCALE_H 1
#define HAVE_LONG_DOUBLE 1
#ifndef _MSC_VER
#define HAVE_LONG_LONG 1
#else
/* #undef HAVE_LONG_LONG */
#endif
#define HAVE_LONG_LONG_FORMAT 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMORY_H 1
#define HAVE_SETLOCALE 1
#ifndef _MSC_VER
#define HAVE_SNPRINTF 1
#ifdef __DMC__
#define snprintf _snprintf
#endif
#else
/* #undef HAVE_SNPRINTF */
#endif
#ifndef _MSC_VER
#define HAVE_STDINT_H 1
#else
/* #undef HAVE_STDINT_H */
#endif
#ifndef _MSC_VER
#define HAVE_STDINT_H_WITH_UINTMAX 1
#else
/* #undef HAVE_STDINT_H_WITH_UINTMAX */
#endif
#define HAVE_STDLIB_H 1
#if !defined(_MSC_VER) && !defined(__DMC__)
#define HAVE_STRCASECMP 1
#else
/* #undef HAVE_STRCASECMP */
#endif
#define HAVE_STRERROR 1
#if !defined(_MSC_VER) && !defined(__DMC__)
#define HAVE_STRINGS_H 1
#else
/* #undef HAVE_STRINGS_H */
#endif
#define HAVE_STRING_H 1
#if !defined(_MSC_VER) && !defined(__DMC__)
#define HAVE_STRNCASECMP 1
#else
/* #undef HAVE_STRNCASECMP */
#endif
#if !defined(_MSC_VER) && !defined(__DMC__)
#define HAVE_SYS_PARAM_H 1
#else
/* #undef HAVE_SYS_PARAM_H */
#endif
#define HAVE_SYS_STAT_H 1
#ifndef _MSC_VER
#define HAVE_SYS_TIME_H 1
#else
/* #undef HAVE_SYS_TIME_H */
#endif
#define HAVE_SYS_TYPES_H 1
#ifndef _MSC_VER
#define HAVE_UNISTD_H 1
#else
/* #undef HAVE_UNISTD_H */
#endif
#if !defined(_MSC_VER) && !defined(__DMC__)
#define HAVE_VALUES_H 1
#else
/* #undef HAVE_VALUES_H */
#endif
#define HAVE_VASPRINTF 1
#define HAVE_VPRINTF 1
#ifndef _MSC_VER
#define HAVE_VSNPRINTF 1
#ifdef __DMC__
#define vsnprintf _vsnprintf
#endif
#else
/* #undef HAVE_VSNPRINTF */
#endif
#define HAVE_WCHAR_T 1
#define HAVE_WCSLEN 1
#define HAVE_WINT_T 1
#ifndef _MSC_VER
#else
#define HAVE_WSPIAPI_H 1
#endif
#define LT_OBJDIR ".libs/"
#define NEED_ICONV_CACHE 1
#define NO_FD_SET 1
#define NO_SYS_ERRLIST 1
#define NO_SYS_SIGLIST 1
#define NO_SYS_SIGLIST_DECL 1
#define PACKAGE_BUGREPORT "http://bugzilla.gnome.org/enter_bug.cgi?product=glib"
#define PACKAGE_NAME "glib"
#define PACKAGE_STRING "glib @GLIB_MAJOR_VERSION@.@GLIB_MINOR_VERSION@.@GLIB_MICRO_VERSION@"
#define PACKAGE_TARNAME "glib"
#define PACKAGE_VERSION "@GLIB_MAJOR_VERSION@.@GLIB_MINOR_VERSION@.@GLIB_MICRO_VERSION@"
#define REALLOC_0_WORKS 1
#ifndef _MSC_VER
#define SANE_MALLOC_PROTOS 1
#else
/* #undef SANE_MALLOC_PROTOS */
#endif
#define SIZEOF_CHAR 1
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#ifndef _MSC_VER
#define SIZEOF_LONG_LONG 8
#else
#define SIZEOF_LONG_LONG 0
#endif
#define SIZEOF_SHORT 2
#define SIZEOF_SIZE_T 4
#define SIZEOF_VOID_P 4
#define SIZEOF___INT64 8
#define STDC_HEADERS 1
#define USE_LIBICONV_NATIVE 1
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif
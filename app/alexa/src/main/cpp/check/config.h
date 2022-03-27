#if defined(__osf__)
# define _OSF_SOURCE
#endif

#include <wchar.h>

#define HAVE_INT16_T 1
#define HAVE_INT32_T 1
#define HAVE_INT64_T 1
#define HAVE_INTMAX_T 1
#define HAVE_UINT8_T 1
#define HAVE_UINT16_T 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1
#define HAVE_UINTMAX_T 1
#define HAVE___INT64 1
#define HAVE_U_INT64_T 1
#define HAVE_UNSIGNED___INT64 1
#if !defined(HAVE_INT64_T) && defined(HAVE___INT64)
typedef __int64 int64_t;
#define HAVE_INT64_T
#endif
#if !defined(HAVE_INT64_T) && SIZE_OF_INT == 8
typedef int int64_t;
#define HAVE_INT64_T
#endif
#if !defined(HAVE_INT64_T) && SIZE_OF_LONG == 8
typedef long int64_t;
#define HAVE_INT64_T
#endif
#if !defined(HAVE_INT64_T) && SIZE_OF_LONG_LONG == 8
typedef long long int64_t;
#define HAVE_INT64_T
#endif
#if !defined(HAVE_INT64_T)
#error No 64-bit integer type was found.
#endif
#if !defined(HAVE_INT32_T) && SIZE_OF_INT == 4
typedef long int32_t;
#define HAVE_INT32_T
#endif
#if !defined(HAVE_INT32_T) && SIZE_OF_LONG == 4
typedef long int32_t;
#define HAVE_INT32_T
#endif
#if !defined(HAVE_INT32_T)
#error No 32-bit integer type was found.
#endif
#if !defined(HAVE_INT16_T) && SIZE_OF_INT == 2
typedef int int16_t;
#define HAVE_INT16_T
#endif
#if !defined(HAVE_INT16_T) && SIZE_OF_SHORT == 2
typedef short int16_t;
#define HAVE_INT16_T
#endif
#if !defined(HAVE_INT16_T)
#error No 16-bit integer type was found.
#endif
#if !defined(HAVE_UINT64_T) && defined(HAVE_UNSIGNED___INT64)
typedef unsigned __int64 uint64_t;
#define HAVE_UINT64_T
#endif
#if !defined(HAVE_UINT64_T) && SIZE_OF_UNSIGNED == 8
typedef unsigned uint64_t;
#define HAVE_UINT64_T
#endif
#if !defined(HAVE_UINT64_T) && SIZE_OF_UNSIGNED_LONG == 8
typedef unsigned long uint64_t;
#define HAVE_UINT64_T
#endif
#if !defined(HAVE_UINT64_T) && SIZE_OF_UNSIGNED_LONG_LONG == 8
typedef unsigned long long uint64_t;
#define HAVE_UINT64_T
#endif
#if !defined(HAVE_UINT64_T)
#error No 64-bit unsigned integer type was found.
#endif
#if !defined(HAVE_UINT32_T) && SIZE_OF_UNSIGNED == 4
typedef unsigned uint32_t;
#define HAVE_UINT32_T
#endif
#if !defined(HAVE_UINT32_T) && SIZE_OF_UNSIGNED_LONG == 4
typedef unsigned long uint32_t;
#define HAVE_UINT32_T
#endif
#if !defined(HAVE_UINT32_T)
#error No 32-bit unsigned integer type was found.
#endif
#if !defined(HAVE_UINT16_T) && SIZE_OF_UNSIGNED == 2
typedef unsigned uint16_t;
#define HAVE_UINT16_T
#endif
#if !defined(HAVE_UINT16_T) && SIZE_OF_UNSIGNED_SHORT == 2
typedef unsigned short uint16_t;
#define HAVE_UINT16_T
#endif
#if !defined(HAVE_UINT16_T)
#error No 16-bit unsigned integer type was found.
#endif
#if !defined(HAVE_UINT8_T)
typedef unsigned char uint8_t;
#define HAVE_UINT8_T
#endif
#if !defined(HAVE_UINT16_T)
#error No 8-bit unsigned integer type was found.
#endif
#if !defined(HAVE_INTMAX_T)
typedef int64_t intmax_t;
#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX
#endif
#if !defined(HAVE_UINTMAX_T)
typedef uint64_t uintmax_t;
#endif
#define HAVE_DECL_INT64_MAX 1
#define HAVE_DECL_INT64_MIN 1
#define HAVE_DECL_SIZE_MAX 1
#define HAVE_DECL_SSIZE_MAX 1
#define HAVE_DECL_UINT32_MAX 1
#define HAVE_DECL_UINT64_MAX 1
#define HAVE_ERRNO_H 1
#define HAVE_FORK 1
#define HAVE_GETPID 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_DECL_LOCALTIME_R 1
#define HAVE_LOCALTIME_S 1
#define HAVE_LONG_LONG_INT 1
#define HAVE_MALLOC 1
#define HAVE_REALLOC 1
#define HAVE_DECL_SETENV 1
#define HAVE_SIGNAL_H 1
#define HAVE_SIGACTION 1
#define HAVE_STDARG_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1¿
#define HAVE_DECL_STRDUP 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_DECL_STRSIGNAL 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_WINDOWS_H 1
#define HAVE_SYNCHAPI_H 1
#define HAVE_INIT_ONCE_BEGIN_INITIALIZE 1
#define HAVE_INIT_ONCE_COMPLETE 1
#define HAVE_UNSIGNED_LONG_LONG 1
#define HAVE_UNSIGNED_LONG_LONG_INT 1
#define HAVE_WCHAR_T 1
#define HAVE__GETPID 1
#define HAVE__LOCALTIME64_S 1
#define HAVE__STRDUP 1
#define HAVE_PTHREAD 1
#define CHECK_VERSION "${CHECK_MAJOR_VERSION}.${CHECK_MINOR_VERSION}.${CHECK_MICRO_VERSION}"
#define SIZEOF_WCHAR_T (sizeof(wchar_t))
#define STRERROR_R_CHAR_P 1
#define TIME_WITH_SYS_TIME 1
#define SAFE_TO_DEFINE_EXTENSIONS 1
#ifdef SAFE_TO_DEFINE_EXTENSIONS
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS 1
#endif
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif
#endif
#define _FILE_OFFSET_BITS
#define _LARGEFILE_SOURCE 1
#define _LARGE_FILES
#define _WIN32_WINNT
#define WINVER
#define clockid_t
#define gid_t
#define id_t
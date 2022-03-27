#ifndef LIBCOMPAT_H
#define LIBCOMPAT_H

#if HAVE_CONFIG_H
#include "config.h"
#endif
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#define GCC_VERSION_AT_LEAST(major, minor, patch) \
((__GNUC__ > (major)) || \
 (__GNUC__ == (major) && __GNUC_MINOR__ > (minor)) || \
 (__GNUC__ == (major) && __GNUC_MINOR__ == (minor) && __GNUC_PATCHLEVEL__ >= (patch)) )
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define GCC_VERSION_AT_LEAST(major, minor, patch) \
((__GNUC__ > (major)) || \
 (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#else
#define GCC_VERSION_AT_LEAST(major, minor, patch) 0
#endif
#if GCC_VERSION_AT_LEAST(2,95,3)
#define CK_ATTRIBUTE_UNUSED __attribute__ ((unused))
#define CK_ATTRIBUTE_FORMAT(a, b, c) __attribute__ ((format (a, b, c)))
#else
#define CK_ATTRIBUTE_UNUSED
#define CK_ATTRIBUTE_FORMAT(a, b, c)
#endif
#if GCC_VERSION_AT_LEAST(2,5,0)
#define CK_ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#define CK_ATTRIBUTE_NORETURN
#endif
#if GCC_VERSION_AT_LEAST(4,7,4) && (__STDC_VERSION__ >= 199901L)
#define CK_DIAGNOSTIC_STRINGIFY(x) #x
#define CK_DIAGNOSTIC_HELPER1(y) CK_DIAGNOSTIC_STRINGIFY(GCC diagnostic ignored y)
#define CK_DIAGNOSTIC_HELPER2(z) CK_DIAGNOSTIC_HELPER1(#z)
#define CK_DIAGNOSTIC_PUSH_IGNORE(w) \
    _Pragma("GCC diagnostic push") \
    _Pragma(CK_DIAGNOSTIC_HELPER2(w))
#define CK_DIAGNOSTIC_POP(w) _Pragma ("GCC diagnostic pop")
#else
#define CK_DIAGNOSTIC_PUSH_IGNORE(w)
#define CK_DIAGNOSTIC_POP(w)
#endif
#ifndef CK_DLL_EXP
#define CK_DLL_EXP
#endif
#if defined(_MSC_VER)
#include <WinSock2.h>
#include <io.h>
#include <process.h>
#endif
#ifdef _WIN32
#define CK_FMT_ZU "%Iu"
#define CK_FMT_ZD "%Id"
#define CK_FMT_TD "%Id"
#else
#define CK_FMT_ZU "%zu"
#define CK_FMT_ZD "%zd"
#define CK_FMT_TD "%td"
#endif
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if !defined(INFINITY) || !defined(NAN)
extern double DOUBLE_ZERO;
#define INFINITY (1.0/DOUBLE_ZERO)
#define NAN (DOUBLE_ZERO/DOUBLE_ZERO)
#endif
#if !defined(isnan) || !defined(isinf) || !defined(isfinite)
#define NEED_fpclassify
extern int fpclassify(double d);
#define FP_INFINITE (1)
#define FP_NAN (2)
#define FP_ZERO (4)
#define FP_NORMAL (8)
#define FP_SUBNORMAL (16)
#define isnan(x) ((fpclassify((double)(x)) & FP_NAN) == FP_NAN)
#define isinf(x) ((fpclassify((double)(x)) & FP_INFINITE) == FP_INFINITE)
#define isfinite(x) ((fpclassify((double)(x)) & (FP_NAN|FP_INFINITE)) == 0)
#endif
#ifndef HAVE_SYS_TIME_H
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#if defined(HAVE_INIT_ONCE_BEGIN_INITIALIZE) && defined(HAVE_INIT_ONCE_COMPLETE)
#define HAVE_WIN32_INIT_ONCE 1
#endif
#if defined HAVE_PTHREAD
#include <pthread.h>
#elif defined HAVE_WIN32_INIT_ONCE
typedef void pthread_mutexattr_t;
typedef struct {
    INIT_ONCE init;
    HANDLE mutex;
} pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER { \
    INIT_ONCE_STATIC_INIT, \
    NULL, \
}
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
#endif
#include <stdint.h>
#if !HAVE_DECL_ALARM
CK_DLL_EXP unsigned int alarm(unsigned int seconds);
#endif
#if !HAVE_MALLOC
CK_DLL_EXP void *rpl_malloc(size_t n);
#endif
#if !HAVE_REALLOC
CK_DLL_EXP void *rpl_realloc(void *p, size_t n);
#endif
#if !HAVE_GETPID && HAVE__GETPID
#define getpid _getpid
#endif
#if !HAVE_GETTIMEOFDAY
CK_DLL_EXP int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
#if !HAVE_DECL_LOCALTIME_R
#if !defined(localtime_r)
CK_DLL_EXP struct tm *localtime_r(const time_t * clock, struct tm *result);
#endif
#endif
#if !HAVE_DECL_STRDUP && !HAVE__STRDUP
CK_DLL_EXP char *strdup(const char *str);
#elif !HAVE_DECL_STRDUP && HAVE__STRDUP
#define strdup _strdup
#endif
#if !HAVE_DECL_STRSIGNAL
CK_DLL_EXP char *strsignal(int sig);
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC -1
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME -1
#endif
#ifndef HAVE_LIBRT
#ifdef STRUCT_TIMESPEC_DEFINITION_MISSING
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
#endif
#ifdef STRUCT_ITIMERSPEC_DEFINITION_MISSING
struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};
#endif
struct sigevent;
CK_DLL_EXP int clock_gettime(clockid_t clk_id, struct timespec *ts);
CK_DLL_EXP int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t * timerid);
CK_DLL_EXP int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
CK_DLL_EXP int timer_delete(timer_t timerid);
#endif
#if !HAVE_STDARG_H
#include <stdarg.h>
#if !HAVE_VSNPRINTF
CK_DLL_EXP int rpl_vsnprintf(char *, size_t, const char *, va_list);
#define vsnprintf rpl_vsnprintf
#endif
#if !HAVE_SNPRINTF
CK_DLL_EXP int rpl_snprintf(char *, size_t, const char *, ...);
#define snprintf rpl_snprintf
#endif
#endif
#if !HAVE_GETLINE
CK_DLL_EXP ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif
CK_DLL_EXP void ck_do_nothing(void) CK_ATTRIBUTE_NORETURN;
#endif
#endif
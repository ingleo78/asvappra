#ifndef _DBUS_CONFIG_H
#define _DBUS_CONFIG_H

#define DBUS_CMAKE 1
#define HAVE_GNUC_VARARGS 1
#define DBUS_CONSOLE_AUTH_DIR "@DBUS_CONSOLE_AUTH_DIR@"
#define DBUS_DATADIR  "@DBUS_DATADIR@"
#define DBUS_BINDIR   "@DBUS_BINDIR@"
#define DBUS_PREFIX "@DBUS_PREFIX@"
#define DBUS_SYSTEM_CONFIG_FILE  "@DBUS_SYSTEM_CONFIG_FILE@"
#define DBUS_SESSION_CONFIG_FILE "@DBUS_SESSION_CONFIG_FILE@"
#define DBUS_DAEMON_NAME "@DBUS_DAEMON_NAME@"
#define DBUS_SYSTEM_BUS_DEFAULT_ADDRESS  "@DBUS_SYSTEM_BUS_DEFAULT_ADDRESS@"
#define DBUS_SESSION_BUS_CONNECT_ADDRESS  "@DBUS_SESSION_BUS_CONNECT_ADDRESS@"
#define DBUS_MACHINE_UUID_FILE "@DBUS_MACHINE_UUID_FILE@"
#define DBUS_DAEMONDIR "@DBUS_DAEMONDIR@"
#define DBUS_RUNSTATEDIR "@DBUS_RUNSTATEDIR@"
#define DBUS_ENABLE_STATS
#define DBUS_ENABLE_CONTAINERS
#define TEST_LISTEN       "@TEST_LISTEN@"
#define DBUS_EXEEXT "@EXEEXT@"
#define DBUS_ENABLE_ANSI 1
#define DBUS_ENABLE_VERBOSE_MODE 1
#define DBUS_DISABLE_ASSERT 1
#ifndef DBUS_DISABLE_ASSERT
#define DBUS_ENABLE_ASSERT 1
#endif
#define DBUS_DISABLE_CHECKS 1
#ifndef DBUS_DISABLE_CHECKS
#define DBUS_ENABLE_CHECKS 1
#endif
#define DBUS_GCOV_ENABLED 1
#define HAVE_CONSOLE_OWNER_FILE 1
#define DBUS_CONSOLE_OWNER_FILE "@DBUS_CONSOLE_OWNER_FILE@"
#define DBUS_BUILD_X11 1
#ifdef DBUS_BUILD_X11
#define DBUS_ENABLE_X11_AUTOLAUNCH 1
#endif
#define _DBUS_VA_COPY_ASSIGN(a1,a2) { a1 = a2; }
#define DBUS_VA_COPY_FUNC
#if (defined DBUS_VA_COPY_FUNC)
#define DBUS_VA_COPY @DBUS_VA_COPY_FUNC@
#endif
#ifdef DBUS_VA_COPY_FUNC
#undef DBUS_VA_COPY_FUNC
#endif
#define DBUS_VA_COPY_AS_ARRAY @DBUS_VA_COPY_AS_ARRAY@
#define DBUS_WITH_GLIB 1
#define GLIB_VERSION_MIN_REQUIRED @GLIB_VERSION_MIN_REQUIRED@
#define GLIB_VERSION_MAX_ALLOWED  @GLIB_VERSION_MAX_ALLOWED@
#define HAVE_ALLOCA_H
#define HAVE_BYTESWAP_H
#define HAVE_CRT_EXTERNS_H
#define HAVE_DIRENT_H 1
#define HAVE_DLFCN_H
#define HAVE_ERRNO_H 1
#define HAVE_EXECINFO_H
#define HAVE_EXPAT_H
#define HAVE_GRP_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_IO_H 1
#define HAVE_LOCALE_H 1
#define HAVE_MEMORY_H
#define HAVE_POLL 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H
#define HAVE_STDIO_H 1
#define HAVE_STRINGS_H
#define HAVE_STRING_H
#define HAVE_SYSLOG_H
#define HAVE_SYS_EVENTS_H
#define HAVE_SYS_INOTIFY_H
#define HAVE_SYS_PRCTL_H
#define HAVE_SYS_RESOURCE_H
#define HAVE_SYS_STAT_H
#define HAVE_SYS_SYSLIMITS_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H
#define HAVE_SYS_UIO_H
#define HAVE_SYS_WAIT_H 1
#define HAVE_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_WS2TCPIP_H
#define HAVE_BACKTRACE 1
#define HAVE_GETGROUPLIST 1
#define HAVE_GETPEERUCRED 1
#define HAVE_NANOSLEEP 1
#define HAVE_POSIX_GETPWNAM_R 1
#define HAVE_SOCKETPAIR 1
#define HAVE_SETENV 1
#define HAVE_UNSETENV 1
#define HAVE_CLEARENV 1
#define HAVE_WRITEV 1
#define HAVE_SOCKLEN_T 1
#define HAVE_SETLOCALE 1
#define HAVE_LOCALECONV 1
#define HAVE_STRTOLL 1
#define HAVE_STRTOULL 1
#define HAVE_PIPE2
#define HAVE_ACCEPT4 1
#define HAVE_DIRFD 1
#define HAVE_INOTIFY_INIT1 1
#define HAVE_UNIX_FD_PASSING 1
#define HAVE_CMSGCRED 1
#define FD_SETSIZE @FD_SETSIZE@
#define DBUS_USER "@DBUS_USER@"
#define DBUS_TEST_USER "@DBUS_TEST_USER@"
#if defined(_WIN32) || defined(_WIN64) || defined (_WIN32_WCE)
#define DBUS_WIN
#define DBUS_WIN_FIXME 1
#ifdef _WIN32_WCE
#define DBUS_WINCE
#else
#define DBUS_WIN32
#endif
#else
#define DBUS_UNIX
#endif
#if defined(_WIN32) || defined(_WIN64)
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifndef _MSC_VER
#define uid_t int
#define gid_t int
#else
#define snprintf _snprintf
typedef int mode_t;
#if !defined(_WIN32_WCE)
#define strtoll _strtoi64
#define strtoull _strtoui64
#define HAVE_STRTOLL 1
#define HAVE_STRTOULL 1
#endif
#endif
#endif
#ifdef interface
#undef interface
#endif
#ifndef SIGHUP
#define SIGHUP	1
#endif
#define DBUS_VERBOSE_C_S 1
#ifdef DBUS_VERBOSE_C_S
#define _dbus_verbose_C_S printf
#else
#define _dbus_verbose_C_S _dbus_verbose
#endif
# if defined(_MSC_VER) && !defined(inline)
#define inline __inline
#endif
#ifdef DBUS_WIN
#define FD_SETSIZE @FD_SETSIZE@
#endif

#endif
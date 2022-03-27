#ifndef HEADER_CURL_CONFIG_WIN32_H
#define HEADER_CURL_CONFIG_WIN32_H

#define HAVE_ASSERT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#if defined(__MINGW32__) || defined(__POCC__)
#define HAVE_GETOPT_H 1
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#define HAVE_INTTYPES_H 1
#endif
#define HAVE_IO_H 1
#define HAVE_LOCALE_H 1
#if !defined(__SALFORDC__) && !defined(__POCC__)
#define NEED_MALLOC_H 1
#endif
#ifndef __SALFORDC__
#define HAVE_PROCESS_H 1
#endif
#define HAVE_SIGNAL_H 1
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#define HAVE_STDBOOL_H 1
#endif
#define HAVE_STDLIB_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#ifndef __BORLANDC__
#define HAVE_SYS_UTIME_H 1
#endif
#define HAVE_TIME_H 1
#if defined(__MINGW32__) || defined(__WATCOMC__) || defined(__LCC__) || defined(__POCC__)
#define HAVE_UNISTD_H 1
#endif
#define HAVE_WINDOWS_H 1
#define HAVE_WINSOCK_H 1
#ifndef __SALFORDC__
#define HAVE_WINSOCK2_H 1
#endif
#ifndef __SALFORDC__
#define HAVE_WS2TCPIP_H 1
#endif
#define HAVE_SIG_ATOMIC_T 1
#define STDC_HEADERS 1
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#define HAVE_BOOL_T 1
#endif
#define HAVE_CLOSESOCKET 1
#define HAVE_GETPEERNAME 1
#define HAVE_GETSOCKNAME 1
#define HAVE_GETHOSTBYADDR 1
#define HAVE_GETHOSTNAME 1
#define HAVE_GETSERVBYNAME 1
#define HAVE_INET_ADDR 1
#define HAVE_IOCTLSOCKET 1
#define HAVE_IOCTLSOCKET_FIONBIO 1
#define HAVE_PERROR 1
#define HAVE_RAND_SCREEN 1
#define HAVE_RAND_STATUS 1
#define HAVE_CRYPTO_CLEANUP_ALL_EX_DATA 1
#define HAVE_SELECT 1
#define HAVE_SETLOCALE 1
#define HAVE_SETMODE 1
#define HAVE_SETVBUF 1
#define HAVE_SOCKET 1
#define HAVE_STRDUP 1
#define HAVE_STRFTIME 1
#define HAVE_STRICMP 1
#define HAVE_STRNICMP 1
#define HAVE_STRSTR 1
#if defined(__MINGW32__) || defined(__WATCOMC__) || defined(__POCC__) || (defined(_MSC_VER) && (_MSC_VER >= 1800))
#define HAVE_STRTOLL 1
#endif
#ifndef __BORLANDC__
#define HAVE_UTIME 1
#endif
#define GETNAMEINFO_QUAL_ARG1 const
#define GETNAMEINFO_TYPE_ARG1 struct sockaddr *
#define GETNAMEINFO_TYPE_ARG2 socklen_t
#define GETNAMEINFO_TYPE_ARG46 DWORD
#define GETNAMEINFO_TYPE_ARG7 int
#define HAVE_RECV 1
#define RECV_TYPE_ARG1 SOCKET
#define RECV_TYPE_ARG2 char *
#define RECV_TYPE_ARG3 int
#define RECV_TYPE_ARG4 int
#define RECV_TYPE_RETV int
#define HAVE_RECVFROM 1
#define RECVFROM_TYPE_ARG1 SOCKET
#define RECVFROM_TYPE_ARG2 char
#define RECVFROM_TYPE_ARG3 int
#define RECVFROM_TYPE_ARG4 int
#define RECVFROM_TYPE_ARG5 struct sockaddr
#define RECVFROM_TYPE_ARG6 int
#define RECVFROM_TYPE_RETV int
#define HAVE_SEND 1
#define SEND_TYPE_ARG1 SOCKET
#define SEND_QUAL_ARG2 const
#define SEND_TYPE_ARG2 char *
#define SEND_TYPE_ARG3 int
#define SEND_TYPE_ARG4 int
#define SEND_TYPE_RETV int
#define in_addr_t unsigned long
#define RETSIGTYPE void
#ifndef _SSIZE_T_DEFINED
#if (defined(__WATCOMC__) && (__WATCOMC__ >= 1240)) || defined(__POCC__) || defined(__MINGW32__)
//nothing here
#elif defined(_WIN64)
#define _SSIZE_T_DEFINED
#define ssize_t __int64
#else
#define _SSIZE_T_DEFINED
typedef signed int ssize_t;
#endif
#endif
#define SIZEOF_INT 4
#define SIZEOF_LONG_DOUBLE 16
#define SIZEOF_SHORT 2
#define SIZEOF_LONG 4
#if defined(_WIN64)
#define SIZEOF_SIZE_T 8
#else
#define SIZEOF_SIZE_T 4
#endif
#define SIZEOF_CURL_OFF_T 8
#ifdef USE_LWIPSOCK
#undef USE_WINSOCK
#undef HAVE_WINSOCK_H
#undef HAVE_WINSOCK2_H
#undef HAVE_WS2TCPIP_H
#undef HAVE_ERRNO_H
#undef HAVE_GETHOSTNAME
#undef HAVE_GETNAMEINFO
#undef LWIP_POSIX_SOCKETS_IO_NAMES
#undef RECV_TYPE_ARG1
#undef RECV_TYPE_ARG3
#undef SEND_TYPE_ARG1
#undef SEND_TYPE_ARG3
#define HAVE_FREEADDRINFO
#define HAVE_GETADDRINFO
#define HAVE_GETHOSTBYNAME
#define HAVE_GETHOSTBYNAME_R
#define HAVE_GETHOSTBYNAME_R_6
#define LWIP_POSIX_SOCKETS_IO_NAMES 0
#define RECV_TYPE_ARG1 int
#define RECV_TYPE_ARG3 size_t
#define SEND_TYPE_ARG1 int
#define SEND_TYPE_ARG3 size_t
#endif
#ifdef USE_WATT32
#include <netinet/tcp.h>
#undef byte
#undef word
#undef USE_WINSOCK
#undef HAVE_WINSOCK_H
#undef HAVE_WINSOCK2_H
#undef HAVE_WS2TCPIP_H
#define HAVE_GETADDRINFO
#define HAVE_GETNAMEINFO
#define HAVE_SYS_IOCTL_H
#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#define HAVE_NETDB_H
#define HAVE_ARPA_INET_H
#define HAVE_FREEADDRINFO
#define SOCKET int
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define HAVE_VARIADIC_MACROS_C99 1
#endif
#if defined(__MINGW32__) || defined(__WATCOMC__) || (defined(_MSC_VER)     && (_MSC_VER     >= 1310)) || (defined(__BORLANDC__) && (__BORLANDC__ >= 0x561))
#define HAVE_LONGLONG 1
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#ifndef _USE_32BIT_TIME_T
#define SIZEOF_TIME_T 8
#else
#define SIZEOF_TIME_T 4
#endif
#endif
#if defined(_MSC_VER)
#define VS2008_MIN_TARGET 0x0500
#if defined(_USING_V110_SDK71_)
#define VS2012_MIN_TARGET 0x0501
#else
#define VS2012_MIN_TARGET 0x0600
#endif
#define VS2008_DEF_TARGET 0x0501
#if defined(_USING_V110_SDK71_)
#define VS2012_DEF_TARGET 0x0501
#else
#define VS2012_DEF_TARGET 0x0600
#endif
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1500) && (_MSC_VER <= 1600)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT VS2008_DEF_TARGET
#endif
#ifndef WINVER
#define WINVER VS2008_DEF_TARGET
#endif
#if (_WIN32_WINNT < VS2008_MIN_TARGET) || (WINVER < VS2008_MIN_TARGET)
#error VS2008 does not support Windows build targets prior to Windows 2000
#endif
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1700)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT VS2012_DEF_TARGET
#endif
#ifndef WINVER
#define WINVER VS2012_DEF_TARGET
#endif
#if (_WIN32_WINNT < VS2012_MIN_TARGET) || (WINVER < VS2012_MIN_TARGET)
#if defined(_USING_V110_SDK71_)
#error VS2012 does not support Windows build targets prior to Windows XP
#else
#error VS2012 does not support Windows build targets prior to Windows Vista
#endif
#endif
#endif
#if defined(__POCC__) && (__POCC__ >= 500)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
#define WINVER 0x0500
#endif
#endif
#if defined(HAVE_WS2TCPIP_H)
#if defined(__POCC__)
#define HAVE_FREEADDRINFO           1
#define HAVE_GETADDRINFO            1
#define HAVE_GETADDRINFO_THREADSAFE 1
#define HAVE_GETNAMEINFO            1
#elif defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0501)
#define HAVE_FREEADDRINFO           1
#define HAVE_GETADDRINFO            1
#define HAVE_GETADDRINFO_THREADSAFE 1
#define HAVE_GETNAMEINFO            1
#elif defined(_MSC_VER) && (_MSC_VER >= 1200)
#define HAVE_FREEADDRINFO           1
#define HAVE_GETADDRINFO            1
#define HAVE_GETADDRINFO_THREADSAFE 1
#define HAVE_GETNAMEINFO            1
#endif
#endif
#if defined(__POCC__)
#ifndef _MSC_VER
#error Microsoft extensions /Ze compiler option is required
#endif
#ifndef __POCC__OLDNAMES
#error Compatibility names /Go compiler option is required
#endif
#endif
#if !defined(__SALFORDC__) && !defined(__BORLANDC__)
#define HAVE_STRUCT_SOCKADDR_STORAGE 1
#endif
#define HAVE_STRUCT_TIMEVAL 1
#define HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID 1
#if defined(HAVE_WINSOCK2_H) && defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0600)
#define HAVE_STRUCT_POLLFD 1
#endif
#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#if (_MSC_VER >= 900) && (_INTEGRAL_MAX_BITS >= 64)
#define USE_WIN32_LARGE_FILES
#else
#define USE_WIN32_SMALL_FILES
endif
#endif
#if defined(__MINGW32__) && !defined(USE_WIN32_LARGE_FILES)
#define USE_WIN32_LARGE_FILES
#endif
#if defined(__WATCOMC__) && !defined(USE_WIN32_LARGE_FILES)
#define USE_WIN32_LARGE_FILES
#endif
#if defined(__POCC__)
#undef USE_WIN32_LARGE_FILES
#endif
#if !defined(USE_WIN32_LARGE_FILES) && !defined(USE_WIN32_SMALL_FILES)
#define USE_WIN32_SMALL_FILES
#endif
#if !defined(USE_SYNC_DNS) && !defined(USE_ARES) && !defined(USE_THREADS_WIN32)
#define USE_THREADS_WIN32 1
#endif
#if defined(USE_ARES) && defined(USE_THREADS_WIN32)
#error "Only one DNS lookup specialty may be defined at most"
#endif
#if defined(CURL_HAS_NOVELL_LDAPSDK) || defined(CURL_HAS_MOZILLA_LDAPSDK)
#undef USE_WIN32_LDAP
#define HAVE_LDAP_SSL_H 1
#define HAVE_LDAP_URL_PARSE 1
#elif defined(CURL_HAS_OPENLDAP_LDAPSDK)
#undef USE_WIN32_LDAP
#define HAVE_LDAP_URL_PARSE 1
#else
#undef HAVE_LDAP_URL_PARSE
#define HAVE_LDAP_SSL 1
#define USE_WIN32_LDAP 1
#endif
#if defined(__WATCOMC__) && defined(USE_WIN32_LDAP)
#if __WATCOMC__ < 1280
#define WINBERAPI  __declspec(cdecl)
#define WINLDAPAPI __declspec(cdecl)
#endif
#endif
#if defined(__POCC__) && defined(USE_WIN32_LDAP)
#define CURL_DISABLE_LDAP 1
#endif
#if !defined(CURL_WINDOWS_APP)
#define USE_WIN32_CRYPTO
#endif
#define USE_UNIX_SOCKETS
#if !defined(UNIX_PATH_MAX)
#define UNIX_PATH_MAX 108
#include <ws2tcpip.h>
typedef struct sockaddr_un {
    ADDRESS_FAMILY sun_family;
    char sun_path[UNIX_PATH_MAX];
} SOCKADDR_UN, *PSOCKADDR_UN;
#endif
#undef OS
#if defined(_M_IX86) || defined(__i386__)
#define OS "i386-pc-win32"
#elif defined(_M_X64) || defined(__x86_64__)
#define OS "x86_64-pc-win32"
#elif defined(_M_IA64) || defined(__ia64__)
#define OS "ia64-pc-win32"
#elif defined(_M_ARM_NT) || defined(__arm__)
#define OS "thumbv7a-pc-win32"
#elif defined(_M_ARM64) || defined(__aarch64__)
#define OS "aarch64-pc-win32"
#else
#define OS "unknown-pc-win32"
#endif
#define PACKAGE "curl"
#define USE_MANUAL 1
#if defined(__POCC__) || defined(USE_IPV6)
#define ENABLE_IPV6 1
#endif
#endif
#endif
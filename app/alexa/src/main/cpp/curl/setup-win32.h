#ifndef HEADER_CURL_SETUP_WIN32_H
#define HEADER_CURL_SETUP_WIN32_H

#ifdef HAVE_WINDOWS_H
#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif
#if defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif
#include <winerror.h>
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#include <tchar.h>
#ifdef UNICODE
typedef wchar_t *(*curl_wcsdup_callback)(const wchar_t *str);
#endif
#endif
#undef USE_WINSOCK
#ifdef HAVE_WINSOCK2_H
#define USE_WINSOCK 2
#else
#ifdef HAVE_WINSOCK_H
#define USE_WINSOCK 1
#endif
#endif
#ifndef _WIN32_WINNT_NT4
#define _WIN32_WINNT_NT4            0x0400   /* Windows NT 4.0 */
#endif
#ifndef _WIN32_WINNT_WIN2K
#define _WIN32_WINNT_WIN2K          0x0500   /* Windows 2000 */
#endif
#ifndef _WIN32_WINNT_WINXP
#define _WIN32_WINNT_WINXP          0x0501   /* Windows XP */
#endif
#ifndef _WIN32_WINNT_WS03
#define _WIN32_WINNT_WS03           0x0502   /* Windows Server 2003 */
#endif
#ifndef _WIN32_WINNT_WIN6
#define _WIN32_WINNT_WIN6           0x0600   /* Windows Vista */
#endif
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA          0x0600   /* Windows Vista */
#endif
#ifndef _WIN32_WINNT_WS08
#define _WIN32_WINNT_WS08           0x0600   /* Windows Server 2008 */
#endif
#ifndef _WIN32_WINNT_LONGHORN
#define _WIN32_WINNT_LONGHORN       0x0600   /* Windows Vista */
#endif
#ifndef _WIN32_WINNT_WIN7
#define _WIN32_WINNT_WIN7           0x0601   /* Windows 7 */
#endif
#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8           0x0602   /* Windows 8 */
#endif
#ifndef _WIN32_WINNT_WINBLUE
#define _WIN32_WINNT_WINBLUE        0x0603   /* Windows 8.1 */
#endif
#ifndef _WIN32_WINNT_WINTHRESHOLD
#define _WIN32_WINNT_WINTHRESHOLD   0x0A00   /* Windows 10 */
#endif
#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10          0x0A00   /* Windows 10 */
#endif

#endif
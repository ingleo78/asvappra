#ifndef DBUS_SOCKETS_H
#define DBUS_SOCKETS_H

#if defined(DBUS_WIN) || defined(DBUS_WINCE)
#ifndef STRICT
#define STRICT
#include <winsock2.h>
#undef STRICT
#endif
#include <winsock2.h>
#undef interface
#if HAVE_ERRNO_H
#include <errno.h>
#endif

#define DBUS_SOCKET_API_RETURNS_ERROR(n) ((n) == SOCKET_ERROR)
#define DBUS_SOCKET_SET_ERRNO() (_dbus_win_set_errno (WSAGetLastError()))
#else
//#error "dbus-sockets-win.h should not be included on non-Windows"
#endif
#endif
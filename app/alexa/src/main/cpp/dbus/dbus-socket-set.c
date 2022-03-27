#include "config.h"
#include "dbus-socket-set.h"

DBusSocketSet *_dbus_socket_set_new(int size_hint) {
  DBusSocketSet *ret;
#ifdef DBUS_HAVE_LINUX_EPOLL
  ret = _dbus_socket_set_epoll_new ();
  if (ret != NULL) return ret;
#endif
  ret = _dbus_socket_set_poll_new (size_hint);
  if (ret != NULL) return ret;
  return NULL;
}
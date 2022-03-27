#ifndef DBUS_SOCKET_SET_H
#define DBUS_SOCKET_SET_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "dbus.h"
#include "dbus-sysdeps.h"

typedef struct {
  DBusPollable fd;
  unsigned int flags;
} DBusSocketEvent;
typedef struct DBusSocketSet DBusSocketSet;
typedef struct DBusSocketSetClass DBusSocketSetClass;
struct DBusSocketSetClass {
  void (*free)(DBusSocketSet *self);
  dbus_bool_t (*add)(DBusSocketSet *self, DBusPollable fd, unsigned int flags, dbus_bool_t enabled);
  void (*remove)(DBusSocketSet *self, DBusPollable fd);
  void (*enable)(DBusSocketSet *self, DBusPollable fd, unsigned int flags);
  void (*disable)(DBusSocketSet *self, DBusPollable fd);
  int (*poll)(DBusSocketSet *self, DBusSocketEvent *revents, int max_events, int timeout_ms);
};
struct DBusSocketSet {
  DBusSocketSetClass *cls;
};
DBusSocketSet *_dbus_socket_set_new(int size_hint);
static inline void _dbus_socket_set_free(DBusSocketSet *self) {
  (self->cls->free)(self);
}
static inline dbus_bool_t _dbus_socket_set_add(DBusSocketSet *self, DBusPollable fd, unsigned int flags, dbus_bool_t enabled) {
  return (self->cls->add)(self, fd, flags, enabled);
}
static inline void _dbus_socket_set_remove(DBusSocketSet *self, DBusPollable fd) {
  (self->cls->remove)(self, fd);
}
static inline void _dbus_socket_set_enable(DBusSocketSet *self, DBusPollable fd, unsigned int flags) {
  (self->cls->enable)(self, fd, flags);
}
static inline void _dbus_socket_set_disable(DBusSocketSet *self, DBusPollable fd) {
  (self->cls->disable)(self, fd);
}
static inline int _dbus_socket_set_poll(DBusSocketSet *self, DBusSocketEvent *revents, int max_events, int timeout_ms) {
  return (self->cls->poll)(self, revents, max_events, timeout_ms);
}
extern DBusSocketSetClass _dbus_socket_set_poll_class;
extern DBusSocketSetClass _dbus_socket_set_epoll_class;
DBusSocketSet *_dbus_socket_set_poll_new(int size_hint);
DBusSocketSet *_dbus_socket_set_epoll_new(void);

#endif
#endif
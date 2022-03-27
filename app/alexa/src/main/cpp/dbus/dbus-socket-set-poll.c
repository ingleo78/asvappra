#include "config.h"
#include "dbus-socket-set.h"
#include "dbus-internals.h"
#include "dbus-list.h"
#include "dbus-sysdeps.h"
#include "dbus-watch.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct {
  DBusSocketSet parent;
  DBusPollFD *fds;
  int n_fds;
  int n_reserved;
  int n_allocated;
} DBusSocketSetPoll;
#define REALLOC_INCREMENT 8
#define MINIMUM_SIZE 8
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#define DEFAULT_SIZE_HINT 1
#else
#define DEFAULT_SIZE_HINT MINIMUM_SIZE
#endif
static inline DBusSocketSetPoll *socket_set_poll_cast(DBusSocketSet *set) {
  _dbus_assert(set->cls == &_dbus_socket_set_poll_class);
  return (DBusSocketSetPoll*)set;
}
static void socket_set_poll_free(DBusSocketSet *set) {
  DBusSocketSetPoll *self = socket_set_poll_cast(set);
  dbus_free(self->fds);
  dbus_free(self);
  _dbus_verbose("freed socket set %p\n", self);
}
DBusSocketSet *_dbus_socket_set_poll_new(int size_hint) {
  DBusSocketSetPoll *ret;
  if (size_hint <= 0) size_hint = DEFAULT_SIZE_HINT;
  ret = dbus_new0(DBusSocketSetPoll, 1);
  if (ret == NULL) return NULL;
  ret->parent.cls = &_dbus_socket_set_poll_class;
  ret->n_fds = 0;
  ret->n_allocated = size_hint;
  ret->fds = dbus_new0(DBusPollFD, size_hint);
  if (ret->fds == NULL) {
      socket_set_poll_free((DBusSocketSet*)ret);
      return NULL;
  }
  _dbus_verbose("new socket set at %p\n", ret);
  return (DBusSocketSet*)ret;
}
static short watch_flags_to_poll_events(unsigned int flags) {
  short events = 0;
  if (flags & DBUS_WATCH_READABLE) events |= _DBUS_POLLIN;
  if (flags & DBUS_WATCH_WRITABLE) events |= _DBUS_POLLOUT;
  return events;
}
static dbus_bool_t socket_set_poll_add(DBusSocketSet *set, DBusPollable fd, unsigned int flags, dbus_bool_t enabled) {
  DBusSocketSetPoll *self = socket_set_poll_cast(set);
#ifndef DBUS_DISABLE_ASSERT
  int i;
  for (i = 0; i < self->n_fds; i++) _dbus_assert(!_dbus_pollable_equals(self->fds[i].fd, fd));
#endif
  if (self->n_reserved >= self->n_allocated) {
      DBusPollFD *new_fds = dbus_realloc(self->fds,sizeof(DBusPollFD) * (self->n_allocated + REALLOC_INCREMENT));
      _dbus_verbose("inflating set %p from %d en/%d res/%d alloc to %d\n", self, self->n_fds, self->n_reserved, self->n_allocated,
                    self->n_allocated + REALLOC_INCREMENT);
      if (new_fds == NULL) return FALSE;
      self->fds = new_fds;
      self->n_allocated += REALLOC_INCREMENT;
  }
  _dbus_verbose("before adding fd %" DBUS_POLLABLE_FORMAT " to %p, %d en/%d res/%d alloc\n", _dbus_pollable_printable (fd), self, self->n_fds, self->n_reserved,
                 self->n_allocated);
  _dbus_assert(self->n_reserved >= self->n_fds);
  _dbus_assert(self->n_allocated > self->n_reserved);
  self->n_reserved++;
  if (enabled) {
      self->fds[self->n_fds].fd = fd;
      self->fds[self->n_fds].events = watch_flags_to_poll_events (flags);
      self->n_fds++;
  }
  return TRUE;
}
static void socket_set_poll_enable(DBusSocketSet *set, DBusPollable fd, unsigned int flags) {
  DBusSocketSetPoll *self = socket_set_poll_cast(set);
  int i;
  for (i = 0; i < self->n_fds; i++) {
      if (_dbus_pollable_equals(self->fds[i].fd, fd)) {
          self->fds[i].events = watch_flags_to_poll_events(flags);
          return;
      }
  }
  _dbus_assert(self->n_fds < self->n_reserved);
  _dbus_assert(self->n_reserved <= self->n_allocated);
  self->fds[self->n_fds].fd = fd;
  self->fds[self->n_fds].events = watch_flags_to_poll_events(flags);
  self->n_fds++;
}
static void socket_set_poll_disable(DBusSocketSet *set, DBusPollable fd) {
  DBusSocketSetPoll *self = socket_set_poll_cast(set);
  int i;
  for (i = 0; i < self->n_fds; i++) {
      if (_dbus_pollable_equals(self->fds[i].fd, fd)) {
          if (i != self->n_fds - 1) {
              self->fds[i].fd = self->fds[self->n_fds - 1].fd;
              self->fds[i].events = self->fds[self->n_fds - 1].events;
          }
          self->n_fds--;
          return;
      }
  }
}
static void socket_set_poll_remove(DBusSocketSet *set, DBusPollable fd) {
  DBusSocketSetPoll *self = socket_set_poll_cast(set);
  socket_set_poll_disable(set, fd);
  self->n_reserved--;
  _dbus_verbose("after removing fd %" DBUS_POLLABLE_FORMAT " from %p, %d en/%d res/%d alloc\n", _dbus_pollable_printable(fd), self, self->n_fds, self->n_reserved,
                self->n_allocated);
  _dbus_assert(self->n_fds <= self->n_reserved);
  _dbus_assert(self->n_reserved <= self->n_allocated);
  if (self->n_reserved + MINIMUM_SIZE < self->n_allocated / 2) {
      DBusPollFD *new_fds = dbus_realloc(self->fds,sizeof(DBusPollFD) * (self->n_reserved + MINIMUM_SIZE));
      _dbus_verbose("before deflating %p, %d en/%d res/%d alloc\n", self, self->n_fds, self->n_reserved, self->n_allocated);
      if (_DBUS_UNLIKELY(new_fds == NULL)) return;
      self->fds = new_fds;
      self->n_allocated = self->n_reserved;
  }
}
static unsigned int watch_flags_from_poll_revents(short revents) {
  unsigned int condition = 0;
  if (revents & _DBUS_POLLIN) condition |= DBUS_WATCH_READABLE;
  if (revents & _DBUS_POLLOUT) condition |= DBUS_WATCH_WRITABLE;
  if (revents & _DBUS_POLLHUP) condition |= DBUS_WATCH_HANGUP;
  if (revents & _DBUS_POLLERR) condition |= DBUS_WATCH_ERROR;
  if (_DBUS_UNLIKELY(revents & _DBUS_POLLNVAL)) condition |= _DBUS_WATCH_NVAL;
  return condition;
}
static int socket_set_poll_poll(DBusSocketSet *set, DBusSocketEvent *revents, int max_events, int timeout_ms) {
  DBusSocketSetPoll *self = socket_set_poll_cast(set);
  int i;
  int n_events;
  int n_ready;
  _dbus_assert(max_events > 0);
  for (i = 0; i < self->n_fds; i++) self->fds[i].revents = 0;
  n_ready = _dbus_poll(self->fds, self->n_fds, timeout_ms);
  if (n_ready <= 0) return n_ready;
  n_events = 0;
  for (i = 0; i < self->n_fds; i++) {
      if (self->fds[i].revents != 0) {
          revents[n_events].fd = self->fds[i].fd;
          revents[n_events].flags = watch_flags_from_poll_revents(self->fds[i].revents);
          n_events += 1;
          if (n_events == max_events) return n_events;
      }
  }
  return n_events;
}
DBusSocketSetClass _dbus_socket_set_poll_class = {
  socket_set_poll_free,
  socket_set_poll_add,
  socket_set_poll_remove,
  socket_set_poll_enable,
  socket_set_poll_disable,
  socket_set_poll_poll
};
#endif
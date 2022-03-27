#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "config.h"
#include "dbus-socket-set.h"
#include "dbus-internals.h"
#include "dbus-sysdeps.h"

#ifndef __linux__
# error This file is for Linux epoll(4)
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct {
  DBusSocketSet parent;
  int epfd;
} DBusSocketSetEpoll;
static inline DBusSocketSetEpoll *socket_set_epoll_cast(DBusSocketSet *set) {
  _dbus_assert(set->cls == &_dbus_socket_set_epoll_class);
  return (DBusSocketSetEpoll*)set;
}
static void socket_set_epoll_free(DBusSocketSet *set) {
  DBusSocketSetEpoll *self = socket_set_epoll_cast(set);
  if (self == NULL) return;
  if (self->epfd != -1) close(self->epfd);
  dbus_free(self);
}
DBusSocketSet *_dbus_socket_set_epoll_new(void) {
  DBusSocketSetEpoll *self;
  self = dbus_new0(DBusSocketSetEpoll, 1);
  if (self == NULL) return NULL;
  self->parent.cls = &_dbus_socket_set_epoll_class;
  self->epfd = epoll_create1(EPOLL_CLOEXEC);
  if (self->epfd == -1) {
      int flags;
      self->epfd = epoll_create(42);
      flags = fcntl(self->epfd, F_GETFD, 0);
      if (flags != -1) fcntl(self->epfd, F_SETFD, flags | FD_CLOEXEC);
  }
  if (self->epfd == -1) {
      socket_set_epoll_free((DBusSocketSet*)self);
      return NULL;
  }
  return (DBusSocketSet*)self;
}
static uint32_t watch_flags_to_epoll_events(unsigned int flags) {
  uint32_t events = 0;
  if (flags & DBUS_WATCH_READABLE) events |= EPOLLIN;
  if (flags & DBUS_WATCH_WRITABLE) events |= EPOLLOUT;
  return events;
}
static unsigned int epoll_events_to_watch_flags(uint32_t events) {
  short flags = 0;
  if (events & EPOLLIN) flags |= DBUS_WATCH_READABLE;
  if (events & EPOLLOUT) flags |= DBUS_WATCH_WRITABLE;
  if (events & EPOLLHUP) flags |= DBUS_WATCH_HANGUP;
  if (events & EPOLLERR) flags |= DBUS_WATCH_ERROR;
  return flags;
}
static dbus_bool_t socket_set_epoll_add(DBusSocketSet *set, DBusPollable fd, unsigned int flags, dbus_bool_t enabled) {
  DBusSocketSetEpoll *self = socket_set_epoll_cast(set);
  struct epoll_event event;
  int err;
  _DBUS_ZERO(event);
  event.data.fd = fd;
  if (enabled) event.events = watch_flags_to_epoll_events(flags);
  else event.events = EPOLLET;
  if (epoll_ctl(self->epfd, EPOLL_CTL_ADD, fd, &event) == 0) return TRUE;
  err = errno;
  switch(err) {
      case ENOMEM: case ENOSPC: break;
      case EBADF: _dbus_warn("Bad fd %d", fd); break;
      case EEXIST: _dbus_warn("fd %d added and then added again", fd); break;
      default: _dbus_warn("Misc error when trying to watch fd %d: %s", fd, strerror(err));
  }
  return FALSE;
}
static void socket_set_epoll_enable(DBusSocketSet *set, DBusPollable fd, unsigned int flags) {
  DBusSocketSetEpoll *self = socket_set_epoll_cast(set);
  struct epoll_event event;
  int err;
  _DBUS_ZERO(event);
  event.data.fd = fd;
  event.events = watch_flags_to_epoll_events(flags);
  if (epoll_ctl(self->epfd, EPOLL_CTL_MOD, fd, &event) == 0) return;
  err = errno;
  switch(err) {
      case EBADF: _dbus_warn("Bad fd %d", fd); break;
      case ENOENT: _dbus_warn("fd %d enabled before it was added", fd); break;
      case ENOMEM: _dbus_warn("Insufficient memory to change watch for fd %d", fd); break;
      default: _dbus_warn("Misc error when trying to watch fd %d: %s", fd, strerror(err));
  }
}
static void socket_set_epoll_disable(DBusSocketSet *set, DBusPollable fd) {
  DBusSocketSetEpoll *self = socket_set_epoll_cast(set);
  struct epoll_event event;
  int err;
  _DBUS_ZERO(event);
  event.data.fd = fd;
  event.events = EPOLLET;
  if (epoll_ctl(self->epfd, EPOLL_CTL_MOD, fd, &event) == 0) return;
  err = errno;
  _dbus_warn("Error when trying to watch fd %d: %s", fd, strerror(err));
}
static void socket_set_epoll_remove(DBusSocketSet *set, DBusPollable fd) {
  DBusSocketSetEpoll *self = socket_set_epoll_cast(set);
  int err;
  struct epoll_event dummy;
  _DBUS_ZERO(dummy);
  if (epoll_ctl(self->epfd, EPOLL_CTL_DEL, fd, &dummy) == 0) return;
  err = errno;
  _dbus_warn("Error when trying to remove fd %d: %s", fd, strerror(err));
}
#define N_STACK_DESCRIPTORS 64
static int socket_set_epoll_poll(DBusSocketSet *set, DBusSocketEvent *revents, int max_events, int timeout_ms) {
  DBusSocketSetEpoll *self = socket_set_epoll_cast(set);
  struct epoll_event events[N_STACK_DESCRIPTORS];
  int n_ready;
  int i;
  _dbus_assert(max_events > 0);
  n_ready = epoll_wait(self->epfd, events,MIN(_DBUS_N_ELEMENTS(events), max_events), timeout_ms);
  if (n_ready <= 0) return n_ready;
  for (i = 0; i < n_ready; i++) {
      revents[i].fd = events[i].data.fd;
      revents[i].flags = epoll_events_to_watch_flags(events[i].events);
  }
  return n_ready;
}
DBusSocketSetClass _dbus_socket_set_epoll_class = {
  socket_set_epoll_free,
  socket_set_epoll_add,
  socket_set_epoll_remove,
  socket_set_epoll_enable,
  socket_set_epoll_disable,
  socket_set_epoll_poll
};
#ifndef TEST_BEHAVIOUR_OF_EPOLLET
#include <stdio.h>
#include <sys/epoll.h>

int main(void) {
  struct epoll_event input;
  struct epoll_event output;
  int epfd = epoll_create1(EPOLL_CLOEXEC);
  int fd = 0;
  int ret;
  _DBUS_ZERO (input);
  input.events = EPOLLHUP | EPOLLET;
  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &input);
  printf("ctl ADD: %d\n", ret);
  ret = epoll_wait(epfd, &output, 1, -1);
  printf("wait for HUP, edge-triggered: %d\n", ret);
  ret = epoll_wait(epfd, &output, 1, 1);
  printf("wait for HUP again: %d\n", ret);
  input.events = EPOLLHUP;
  ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &input);
  printf("ctl MOD: %d\n", ret);
  ret = epoll_wait(epfd, &output, 1, -1);
  printf("wait for HUP: %d\n", ret);
  return 0;
}
#endif
#endif
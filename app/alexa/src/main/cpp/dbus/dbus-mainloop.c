#include "config.h"
#include "dbus-mainloop.h"
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "dbus-hash.h"
#include "dbus-list.h"
#include "dbus-socket-set.h"
#include "dbus-timeout.h"
#include "dbus-watch.h"

#define MAINLOOP_SPEW 0
struct DBusLoop {
  int refcount;
  DBusHashTable *watches;
  DBusSocketSet *socket_set;
  DBusList *timeouts;
  int callback_list_serial;
  int watch_count;
  int timeout_count;
  int depth;
  DBusList *need_dispatch;
  unsigned oom_watch_pending : 1;
};
typedef struct {
  DBusTimeout *timeout;
  long last_tv_sec;
  long last_tv_usec;
} TimeoutCallback;
#define TIMEOUT_CALLBACK(callback)  ((TimeoutCallback*)callback)
static TimeoutCallback* timeout_callback_new(DBusTimeout *timeout) {
  TimeoutCallback *cb;
  cb = dbus_new (TimeoutCallback, 1);
  if (cb == NULL) return NULL;
  cb->timeout = timeout;
  _dbus_get_monotonic_time(&cb->last_tv_sec, &cb->last_tv_usec);
  return cb;
}
static void timeout_callback_free(TimeoutCallback *cb) {
  dbus_free(cb);
}
static void free_watch_table_entry(void *data) {
  DBusList **watches = data;
  DBusWatch *watch;
  if (watches == NULL) return;
  for (watch = _dbus_list_pop_first(watches); watch != NULL; watch = _dbus_list_pop_first(watches)) _dbus_watch_unref(watch);
  _dbus_assert(*watches == NULL);
  dbus_free(watches);
}
DBusLoop* _dbus_loop_new(void) {
  DBusLoop *loop;
  loop = dbus_new0(DBusLoop, 1);
  if (loop == NULL) return NULL;
  loop->watches = _dbus_hash_table_new(DBUS_HASH_POLLABLE, NULL, free_watch_table_entry);
  loop->socket_set = _dbus_socket_set_new(0);
  if (loop->watches == NULL || loop->socket_set == NULL) {
      if (loop->watches != NULL) _dbus_hash_table_unref(loop->watches);
      if (loop->socket_set != NULL) _dbus_socket_set_free(loop->socket_set);
      dbus_free(loop);
      return NULL;
  }
  loop->refcount = 1;
  return loop;
}
DBusLoop *_dbus_loop_ref(DBusLoop *loop) {
  _dbus_assert(loop != NULL);
  _dbus_assert(loop->refcount > 0);
  loop->refcount += 1;
  return loop;
}
void _dbus_loop_unref(DBusLoop *loop) {
  _dbus_assert(loop != NULL);
  _dbus_assert(loop->refcount > 0);
  loop->refcount -= 1;
  if (loop->refcount == 0) {
      while(loop->need_dispatch) {
          DBusConnection *connection = _dbus_list_pop_first(&loop->need_dispatch);
          dbus_connection_unref(connection);
      }
      _dbus_hash_table_unref(loop->watches);
      _dbus_socket_set_free(loop->socket_set);
      dbus_free(loop);
  }
}
static DBusList **ensure_watch_table_entry(DBusLoop *loop, DBusPollable fd) {
  DBusList **watches;
  watches = _dbus_hash_table_lookup_pollable(loop->watches, fd);
  if (watches == NULL) {
      watches = dbus_new0(DBusList *, 1);
      if (watches == NULL) return watches;
      if (!_dbus_hash_table_insert_pollable(loop->watches, fd, watches)) {
          dbus_free(watches);
          watches = NULL;
      }
  }
  return watches;
}
static void cull_watches_for_invalid_fd(DBusLoop *loop, DBusPollable fd) {
  DBusList *link;
  DBusList **watches;
  _dbus_warn("invalid request, socket fd %" DBUS_POLLABLE_FORMAT " not open", _dbus_pollable_printable(fd));
  watches = _dbus_hash_table_lookup_pollable(loop->watches, fd);
  if (watches != NULL) {
      for (link = _dbus_list_get_first_link(watches); link != NULL; link = _dbus_list_get_next_link(watches, link)) _dbus_watch_invalidate(link->data);
  }
  _dbus_hash_table_remove_pollable(loop->watches, fd);
}
static dbus_bool_t gc_watch_table_entry(DBusLoop *loop, DBusList **watches, DBusPollable fd) {
  if (watches == NULL) return FALSE;
  if (*watches != NULL) return FALSE;
  _dbus_hash_table_remove_pollable(loop->watches, fd);
  return TRUE;
}
static void refresh_watches_for_fd(DBusLoop *loop, DBusList **watches, DBusPollable fd) {
  DBusList *link;
  unsigned int flags = 0;
  dbus_bool_t interested = FALSE;
  _dbus_assert(_dbus_pollable_is_valid(fd));
  if (watches == NULL) watches = _dbus_hash_table_lookup_pollable(loop->watches, fd);
  _dbus_assert(watches != NULL);
  for (link = _dbus_list_get_first_link(watches); link != NULL; link = _dbus_list_get_next_link(watches, link)) {
      if (dbus_watch_get_enabled(link->data) && !_dbus_watch_get_oom_last_time(link->data)) {
          flags |= dbus_watch_get_flags(link->data);
          interested = TRUE;
      }
  }
  if (interested) _dbus_socket_set_enable(loop->socket_set, fd, flags);
  else _dbus_socket_set_disable(loop->socket_set, fd);
}
dbus_bool_t _dbus_loop_add_watch(DBusLoop *loop, DBusWatch *watch) {
  DBusPollable fd;
  DBusList **watches;
  fd = _dbus_watch_get_pollable(watch);
  _dbus_assert(_dbus_pollable_is_valid(fd));
  watches = ensure_watch_table_entry(loop, fd);
  if (watches == NULL) return FALSE;
  if (!_dbus_list_append(watches, _dbus_watch_ref(watch))) {
      _dbus_watch_unref(watch);
      gc_watch_table_entry(loop, watches, fd);
      return FALSE;
  }
  if (_dbus_list_length_is_one(watches)) {
      if (!_dbus_socket_set_add(loop->socket_set, fd, dbus_watch_get_flags(watch), dbus_watch_get_enabled(watch))) {
          _dbus_hash_table_remove_pollable(loop->watches, fd);
          return FALSE;
      }
  } else refresh_watches_for_fd(loop, watches, fd);
  loop->callback_list_serial += 1;
  loop->watch_count += 1;
  return TRUE;
}
void _dbus_loop_toggle_watch(DBusLoop *loop, DBusWatch *watch) {
  refresh_watches_for_fd(loop, NULL, _dbus_watch_get_pollable(watch));
}
void _dbus_loop_remove_watch(DBusLoop *loop, DBusWatch *watch) {
  DBusList **watches;
  DBusList *link;
  DBusPollable fd;
  fd = _dbus_watch_get_pollable(watch);
  _dbus_assert(_dbus_pollable_is_valid(fd));
  watches = _dbus_hash_table_lookup_pollable(loop->watches, fd);
  if (watches != NULL) {
      link = _dbus_list_get_first_link(watches);
      while(link != NULL) {
          DBusList *next = _dbus_list_get_next_link(watches, link);
          DBusWatch *this = link->data;
          if (this == watch) {
              _dbus_list_remove_link(watches, link);
              loop->callback_list_serial += 1;
              loop->watch_count -= 1;
              _dbus_watch_unref(this);
              if (gc_watch_table_entry(loop, watches, fd)) _dbus_socket_set_remove(loop->socket_set, fd);
              return;
          }
          link = next;
      }
  }
  _dbus_warn("could not find watch %p to remove", watch);
}
dbus_bool_t _dbus_loop_add_timeout(DBusLoop *loop, DBusTimeout *timeout) {
  TimeoutCallback *tcb;
  tcb = timeout_callback_new(timeout);
  if (tcb == NULL) return FALSE;
  if (_dbus_list_append(&loop->timeouts, tcb)) {
      loop->callback_list_serial += 1;
      loop->timeout_count += 1;
  } else {
      timeout_callback_free(tcb);
      return FALSE;
  }
  return TRUE;
}
void _dbus_loop_remove_timeout(DBusLoop *loop, DBusTimeout *timeout) {
  DBusList *link;
  link = _dbus_list_get_first_link(&loop->timeouts);
  while(link != NULL) {
      DBusList *next = _dbus_list_get_next_link(&loop->timeouts, link);
      TimeoutCallback *this = link->data;
      if (this->timeout == timeout) {
          _dbus_list_remove_link(&loop->timeouts, link);
          loop->callback_list_serial += 1;
          loop->timeout_count -= 1;
          timeout_callback_free(this);
          return;
      }
      link = next;
  }
  _dbus_warn("could not find timeout %p to remove", timeout);
}
static dbus_bool_t check_timeout(long tv_sec, long tv_usec, TimeoutCallback *tcb, int *timeout) {
  long sec_remaining;
  long msec_remaining;
  long expiration_tv_sec;
  long expiration_tv_usec;
  long interval_seconds;
  long interval_milliseconds;
  int interval;
  interval = dbus_timeout_get_interval(tcb->timeout);
  interval_seconds = interval / 1000L;
  interval_milliseconds = interval % 1000L;
  expiration_tv_sec = tcb->last_tv_sec + interval_seconds;
  expiration_tv_usec = tcb->last_tv_usec + interval_milliseconds * 1000;
  if (expiration_tv_usec >= 1000000) {
      expiration_tv_usec -= 1000000;
      expiration_tv_sec += 1;
  }
  sec_remaining = expiration_tv_sec - tv_sec;
  msec_remaining = (expiration_tv_usec - tv_usec) / 1000L;
#if MAINLOOP_SPEW
  _dbus_verbose("Interval is %ld seconds %ld msecs\n", interval_seconds, interval_milliseconds);
  _dbus_verbose("Now is  %lu seconds %lu usecs\n", tv_sec, tv_usec);
  _dbus_verbose("Last is %lu seconds %lu usecs\n", tcb->last_tv_sec, tcb->last_tv_usec);
  _dbus_verbose("Exp is  %lu seconds %lu usecs\n", expiration_tv_sec, expiration_tv_usec);
  _dbus_verbose("Pre-correction, sec_remaining %ld msec_remaining %ld\n", sec_remaining, msec_remaining);
#endif
  if (sec_remaining < 0 || (sec_remaining == 0 && msec_remaining < 0)) *timeout = 0;
  else {
      if (msec_remaining < 0) {
          msec_remaining += 1000;
          sec_remaining -= 1;
	  }
      if (sec_remaining > (_DBUS_INT_MAX / 1000) || msec_remaining > _DBUS_INT_MAX) *timeout = _DBUS_INT_MAX;
      else *timeout = sec_remaining * 1000 + msec_remaining;
  }
  if (*timeout > interval) {
      _dbus_verbose("System clock set backward! Resetting timeout.\n");
      tcb->last_tv_sec = tv_sec;
      tcb->last_tv_usec = tv_usec;
      *timeout = interval;
  }
#if MAINLOOP_SPEW
  _dbus_verbose("  timeout expires in %d milliseconds\n", *timeout);
#endif
  return *timeout == 0;
}
dbus_bool_t _dbus_loop_dispatch(DBusLoop *loop) {
#if MAINLOOP_SPEW
  _dbus_verbose("  %d connections to dispatch\n", _dbus_list_get_length (&loop->need_dispatch));
#endif
  if (loop->need_dispatch == NULL) return FALSE;
next:
  while(loop->need_dispatch != NULL) {
      DBusConnection *connection = _dbus_list_pop_first(&loop->need_dispatch);
      while(TRUE) {
          DBusDispatchStatus status;
          status = dbus_connection_dispatch(connection);
          if (status == DBUS_DISPATCH_COMPLETE) {
              dbus_connection_unref(connection);
              goto next;
          } else {
              if (status == DBUS_DISPATCH_NEED_MEMORY) _dbus_wait_for_memory();
          }
      }
  }
  return TRUE;
}
dbus_bool_t _dbus_loop_queue_dispatch(DBusLoop *loop, DBusConnection *connection) {
  if (_dbus_list_append(&loop->need_dispatch, connection)) {
      dbus_connection_ref(connection);
      return TRUE;
  } else return FALSE;
}
dbus_bool_t _dbus_loop_iterate(DBusLoop *loop, dbus_bool_t block) {
#define N_STACK_DESCRIPTORS 64
  dbus_bool_t retval;
  DBusSocketEvent ready_fds[N_STACK_DESCRIPTORS];
  int i;
  DBusList *link;
  int n_ready;
  int initial_serial;
  long timeout;
  int orig_depth;
  retval = FALSE;
  orig_depth = loop->depth;
#if MAINLOOP_SPEW
  _dbus_verbose("Iteration block=%d depth=%d timeout_count=%d watch_count=%d\n", block, loop->depth, loop->timeout_count, loop->watch_count);
#endif
  if (_dbus_hash_table_get_n_entries(loop->watches) == 0 && loop->timeouts == NULL) goto next_iteration;
  timeout = -1;
  if (loop->timeout_count > 0) {
      long tv_sec;
      long tv_usec;
      _dbus_get_monotonic_time(&tv_sec, &tv_usec);
      link = _dbus_list_get_first_link(&loop->timeouts);
      while(link != NULL) {
          DBusList *next = _dbus_list_get_next_link(&loop->timeouts, link);
          TimeoutCallback *tcb = link->data;
          if (dbus_timeout_get_enabled(tcb->timeout)) {
              int msecs_remaining;
              if (_dbus_timeout_needs_restart(tcb->timeout)) {
                  tcb->last_tv_sec = tv_sec;
                  tcb->last_tv_usec = tv_usec;
                  _dbus_timeout_restarted(tcb->timeout);
              }
              check_timeout(tv_sec, tv_usec, tcb, &msecs_remaining);
              if (timeout < 0) timeout = msecs_remaining;
              else timeout = MIN(msecs_remaining, timeout);
          #if MAINLOOP_SPEW
              _dbus_verbose("  timeout added, %d remaining, aggregate timeout %ld\n", msecs_remaining, timeout);
          #endif
              _dbus_assert(timeout >= 0);
          }
      #if MAINLOOP_SPEW
          else _dbus_verbose("  skipping disabled timeout\n");
      #endif
          link = next;
      }
  }
  if (!block || loop->need_dispatch != NULL) {
      timeout = 0;
  #if MAINLOOP_SPEW
      _dbus_verbose("  timeout is 0 as we aren't blocking\n");
  #endif
  }
  if (loop->oom_watch_pending) timeout = MIN(timeout, _dbus_get_oom_wait());
#if MAINLOOP_SPEW
  _dbus_verbose("  polling on %d descriptors timeout %ld\n", _DBUS_N_ELEMENTS(ready_fds), timeout);
#endif
  n_ready = _dbus_socket_set_poll(loop->socket_set, ready_fds,_DBUS_N_ELEMENTS(ready_fds), timeout);
  if (loop->oom_watch_pending) {
      DBusHashIter hash_iter;
      loop->oom_watch_pending = FALSE;
      _dbus_hash_iter_init(loop->watches, &hash_iter);
      while(_dbus_hash_iter_next(&hash_iter)) {
          DBusList **watches;
          DBusPollable fd;
          dbus_bool_t changed;
          changed = FALSE;
          fd = _dbus_hash_iter_get_pollable_key(&hash_iter);
          watches = _dbus_hash_iter_get_value(&hash_iter);
          for (link = _dbus_list_get_first_link(watches); link != NULL; link = _dbus_list_get_next_link(watches, link)) {
              DBusWatch *watch = link->data;
              if (_dbus_watch_get_oom_last_time(watch)) {
                  _dbus_watch_set_oom_last_time(watch, FALSE);
                  changed = TRUE;
              }
          }
          if (changed) refresh_watches_for_fd(loop, watches, fd);
      }
      retval = TRUE;
  }
  initial_serial = loop->callback_list_serial;
  if (loop->timeout_count > 0) {
      long tv_sec;
      long tv_usec;
      _dbus_get_monotonic_time(&tv_sec, &tv_usec);
      link = _dbus_list_get_first_link(&loop->timeouts);
      while (link != NULL) {
          DBusList *next = _dbus_list_get_next_link(&loop->timeouts, link);
          TimeoutCallback *tcb = link->data;
          if (initial_serial != loop->callback_list_serial) goto next_iteration;
          if (loop->depth != orig_depth) goto next_iteration;
          if (dbus_timeout_get_enabled(tcb->timeout)) {
              int msecs_remaining;
              if (check_timeout(tv_sec, tv_usec, tcb, &msecs_remaining)) {
                  tcb->last_tv_sec = tv_sec;
                  tcb->last_tv_usec = tv_usec;
              #if MAINLOOP_SPEW
                  _dbus_verbose("  invoking timeout\n");
              #endif
                  dbus_timeout_handle(tcb->timeout);
                  retval = TRUE;
              } else {
              #if MAINLOOP_SPEW
                  _dbus_verbose("  timeout has not expired\n");
              #endif
              }
          }
      #if MAINLOOP_SPEW
          else _dbus_verbose("  skipping invocation of disabled timeout\n");
      #endif
          link = next;
      }
  }
  if (n_ready > 0) {
      for (i = 0; i < n_ready; i++) {
          DBusList **watches;
          DBusList *next;
          unsigned int condition;
          dbus_bool_t any_oom;
          if (initial_serial != loop->callback_list_serial) goto next_iteration;
          if (loop->depth != orig_depth) goto next_iteration;
          _dbus_assert(ready_fds[i].flags != 0);
          if (_DBUS_UNLIKELY(ready_fds[i].flags & _DBUS_WATCH_NVAL)) {
              cull_watches_for_invalid_fd(loop, ready_fds[i].fd);
              goto next_iteration;
          }
          condition = ready_fds[i].flags;
          _dbus_assert((condition & _DBUS_WATCH_NVAL) == 0);
          if (condition == 0) continue;
          watches = _dbus_hash_table_lookup_pollable(loop->watches, ready_fds[i].fd);
          if (watches == NULL) continue;
          any_oom = FALSE;
          for (link = _dbus_list_get_first_link(watches); link != NULL; link = next) {
              DBusWatch *watch = link->data;
              next = _dbus_list_get_next_link(watches, link);
              if (dbus_watch_get_enabled(watch)) {
                  dbus_bool_t oom;
                  oom = !dbus_watch_handle(watch, condition);
                  if (oom) {
                      _dbus_watch_set_oom_last_time(watch, TRUE);
                      loop->oom_watch_pending = TRUE;
                      any_oom = TRUE;
                  }
              #if MAINLOOP_SPEW
                  _dbus_verbose("  Invoked watch, oom = %d\n", oom);
              #endif
                  retval = TRUE;
                  if (initial_serial != loop->callback_list_serial || loop->depth != orig_depth) {
                      if (any_oom) refresh_watches_for_fd(loop, NULL, ready_fds[i].fd);
                      goto next_iteration;
                  }
              }
          }
          if (any_oom) refresh_watches_for_fd(loop, watches, ready_fds[i].fd);
      }
  }
next_iteration:
#if MAINLOOP_SPEW
  _dbus_verbose("  moving to next iteration\n");
#endif
  if (_dbus_loop_dispatch(loop)) retval = TRUE;
#if MAINLOOP_SPEW
  _dbus_verbose("Returning %d\n", retval);
#endif
  return retval;
}
void _dbus_loop_run(DBusLoop *loop) {
  int our_exit_depth;
  _dbus_assert(loop->depth >= 0);
  _dbus_loop_ref(loop);
  our_exit_depth = loop->depth;
  loop->depth += 1;
  _dbus_verbose("Running main loop, depth %d -> %d\n", loop->depth - 1, loop->depth);
  while(loop->depth != our_exit_depth) _dbus_loop_iterate(loop, TRUE);
  _dbus_loop_unref(loop);
}
void _dbus_loop_quit(DBusLoop *loop) {
  _dbus_assert(loop->depth > 0);
  loop->depth -= 1;
  _dbus_verbose("Quit main loop, depth %d -> %d\n", loop->depth + 1, loop->depth);
}
int _dbus_get_oom_wait(void) {
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  return 0;
#else
  return 500;
#endif
}
void _dbus_wait_for_memory(void) {
  _dbus_verbose("Waiting for more memory\n");
  _dbus_sleep_milliseconds(_dbus_get_oom_wait());
}
#endif
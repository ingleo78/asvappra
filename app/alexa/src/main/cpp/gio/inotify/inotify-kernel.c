#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>
#include "../../glib/glib.h"
#include "../config.h"
#include "inotify-kernel.h"

#define PROCESS_EVENTS_TIME 1000
#define DEFAULT_HOLD_UNTIL_TIME 0
#define MOVE_HOLD_UNTIL_TIME 500
static int inotify_instance_fd = -1;
static GQueue *events_to_process = NULL;
static GQueue *event_queue = NULL;
static GHashTable * cookie_hash = NULL;
static GIOChannel *inotify_read_ioc;
static GPollFD ik_poll_fd;
static gboolean ik_poll_fd_enabled = TRUE;
static void (*user_cb)(ik_event_t *event);
static gboolean ik_read_callback(gpointer user_data);
static gboolean ik_process_eq_callback(gpointer user_data);
static guint32 ik_move_matches = 0;
static guint32 ik_move_misses = 0;
static gboolean process_eq_running = FALSE;
G_LOCK_EXTERN (inotify_lock);
typedef struct ik_event_internal {
  ik_event_t *event;
  gboolean seen;
  gboolean sent;
  GTimeVal hold_until;
  struct ik_event_internal *pair;
} ik_event_internal_t;
static gboolean ik_source_prepare(GSource *source, gint *timeout) {
  return FALSE;
}
static gboolean ik_source_timeout(gpointer data) {
  GSource *source = (GSource*)data;
  g_source_add_poll(source, &ik_poll_fd);
  g_source_unref (source);
  ik_poll_fd_enabled = TRUE;
  return FALSE;
}
#define MAX_PENDING_COUNT 2
#define PENDING_THRESHOLD(qsize) ((qsize) >> 1)
#define PENDING_MARGINAL_COST(p) ((unsigned int)(1 << (p)))
#define MAX_QUEUED_EVENTS 2048
#define AVERAGE_EVENT_SIZE sizeof(struct inotify_event) + 16
#define TIMEOUT_MILLISECONDS 10
static gboolean ik_source_check(GSource *source) {
  static int prev_pending = 0, pending_count = 0;
  if (!ik_poll_fd_enabled || !(ik_poll_fd.revents & G_IO_IN)) return FALSE;
  if (pending_count < MAX_PENDING_COUNT) {
      unsigned int pending;
      if (ioctl (inotify_instance_fd, FIONREAD, &pending) == -1) goto do_read;
      pending /= AVERAGE_EVENT_SIZE;
      if (pending > PENDING_THRESHOLD (MAX_QUEUED_EVENTS)) goto do_read;
      if (pending-prev_pending < PENDING_MARGINAL_COST (pending_count)) goto do_read;
      prev_pending = pending;
      pending_count++;
      g_source_remove_poll (source, &ik_poll_fd);
      ik_poll_fd_enabled = FALSE;
      g_source_ref (source);
      g_timeout_add (TIMEOUT_MILLISECONDS, ik_source_timeout, source);
      return FALSE;
  }
  do_read:
  prev_pending = 0;
  pending_count = 0;
  return TRUE;
}
static gboolean ik_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
  if (callback) return callback(user_data);
  return TRUE;
}
static GSourceFuncs ik_source_funcs = {
  ik_source_prepare,
  ik_source_check,
  ik_source_dispatch,
  NULL
};
gboolean _ik_startup (void (*cb)(ik_event_t *event)) {
  static gboolean initialized = FALSE;
  GSource *source;
  user_cb = cb;
  if (initialized) return inotify_instance_fd >= 0;
  initialized = TRUE;
#ifdef HAVE_INOTIFY_INIT1
  inotify_instance_fd = inotify_init1(IN_CLOEXEC);
#else
  inotify_instance_fd = -1;
#endif
  if (inotify_instance_fd < 0) inotify_instance_fd = inotify_init();
  if (inotify_instance_fd < 0) return FALSE;
  inotify_read_ioc = g_io_channel_unix_new(inotify_instance_fd);
  ik_poll_fd.fd = inotify_instance_fd;
  ik_poll_fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
  g_io_channel_set_encoding(inotify_read_ioc, NULL, NULL);
  g_io_channel_set_flags(inotify_read_ioc, G_IO_FLAG_NONBLOCK, NULL);
  source = g_source_new(&ik_source_funcs, sizeof(GSource));
  g_source_set_name(source, "GIO Inotify");
  g_source_add_poll(source, &ik_poll_fd);
  g_source_set_callback(source, ik_read_callback, NULL, NULL);
  g_source_attach(source, NULL);
  g_source_unref(source);
  cookie_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
  event_queue = g_queue_new();
  events_to_process = g_queue_new();
  return TRUE;
}
static ik_event_internal_t *ik_event_internal_new(ik_event_t *event) {
  ik_event_internal_t *internal_event = g_new0(ik_event_internal_t, 1);
  GTimeVal tv;
  g_assert(event);
  g_get_current_time(&tv);
  g_time_val_add (&tv, DEFAULT_HOLD_UNTIL_TIME);
  internal_event->event = event;
  internal_event->hold_until = tv;
  return internal_event;
}
static ik_event_t *ik_event_new(char *buffer) {
  struct inotify_event *kevent = (struct inotify_event*)buffer;
  ik_event_t *event = g_new0(ik_event_t, 1);
  g_assert(buffer);
  event->wd = kevent->wd;
  event->mask = kevent->mask;
  event->cookie = kevent->cookie;
  event->len = kevent->len;
  if (event->len) event->name = g_strdup(kevent->name);
  else event->name = g_strdup("");
  return event;
}
ik_event_t * ik_event_new_dummy(const char *name, gint32 wd, guint32 mask) {
  ik_event_t *event = g_new0 (ik_event_t, 1);
  event->wd = wd;
  event->mask = mask;
  event->cookie = 0;
  if (name) event->name = g_strdup (name);
  else event->name = g_strdup("");
  event->len = strlen(event->name);
  return event;
}
void _ik_event_free(ik_event_t *event) {
  if (event->pair) _ik_event_free(event->pair);
  g_free(event->name);
  g_free(event);
}
gint32 _ik_watch(const char *path, guint32 mask, int *err) {
  gint32 wd = -1;
  g_assert(path != NULL);
  g_assert(inotify_instance_fd >= 0);
  wd = inotify_add_watch(inotify_instance_fd, path, mask);
  if (wd < 0) {
      int e = errno;
      if (err) *err = e;
      return wd;
  }
  g_assert(wd >= 0);
  return wd;
}
int _ik_ignore(const char *path, gint32 wd) {
  g_assert(wd >= 0);
  g_assert(inotify_instance_fd >= 0);
  if (inotify_rm_watch(inotify_instance_fd, wd) < 0) return -1;
  return 0;
}
void _ik_move_stats(guint32 *matches, guint32 *misses) {
  if (matches) *matches = ik_move_matches;
  if (misses) *misses = ik_move_misses;
}
const char *_ik_mask_to_string(guint32 mask) {
  gboolean is_dir = mask & IN_ISDIR;
  mask &= ~IN_ISDIR;
  if (is_dir) {
      switch(mask) {
          case IN_ACCESS: return "ACCESS (dir)";
          case IN_MODIFY: return "MODIFY (dir)";
          case IN_ATTRIB: return "ATTRIB (dir)";
          case IN_CLOSE_WRITE: return "CLOSE_WRITE (dir)";
          case IN_CLOSE_NOWRITE: return "CLOSE_NOWRITE (dir)";
          case IN_OPEN: return "OPEN (dir)";
          case IN_MOVED_FROM: return "MOVED_FROM (dir)";
          case IN_MOVED_TO: return "MOVED_TO (dir)";
          case IN_DELETE: return "DELETE (dir)";
          case IN_CREATE: return "CREATE (dir)";
          case IN_DELETE_SELF: return "DELETE_SELF (dir)";
          case IN_UNMOUNT: return "UNMOUNT (dir)";
          case IN_Q_OVERFLOW: return "Q_OVERFLOW (dir)";
          case IN_IGNORED: return "IGNORED (dir)";
          default: return "UNKNOWN_EVENT (dir)";
	  }
  } else {
      switch(mask) {
          case IN_ACCESS: return "ACCESS";
          case IN_MODIFY: return "MODIFY";
          case IN_ATTRIB: return "ATTRIB";
          case IN_CLOSE_WRITE: return "CLOSE_WRITE";
          case IN_CLOSE_NOWRITE: return "CLOSE_NOWRITE";
          case IN_OPEN: return "OPEN";
          case IN_MOVED_FROM: return "MOVED_FROM";
          case IN_MOVED_TO: return "MOVED_TO";
          case IN_DELETE: return "DELETE";
          case IN_CREATE: return "CREATE";
          case IN_DELETE_SELF: return "DELETE_SELF";
          case IN_UNMOUNT: return "UNMOUNT";
          case IN_Q_OVERFLOW: return "Q_OVERFLOW";
          case IN_IGNORED: return "IGNORED";
          default: return "UNKNOWN_EVENT";
	  }
  }
}
static void ik_read_events(gsize  *buffer_size_out, gchar **buffer_out) {
  static gchar *buffer = NULL;
  static gsize buffer_size;
  if (buffer == NULL) {
      buffer_size = AVERAGE_EVENT_SIZE;
      buffer_size *= MAX_QUEUED_EVENTS;
      buffer = g_malloc(buffer_size);
  }
  *buffer_size_out = 0;
  *buffer_out = NULL;
  memset(buffer, 0, buffer_size);
  if (g_io_channel_read_chars (inotify_read_ioc, (char*)buffer, buffer_size, buffer_size_out, NULL) != G_IO_STATUS_NORMAL);
  *buffer_out = buffer;
}
static gboolean ik_read_callback(gpointer user_data) {
  gchar *buffer;
  gsize buffer_size, buffer_i, events;
  G_LOCK(inotify_lock);
  ik_read_events(&buffer_size, &buffer);
  buffer_i = 0;
  events = 0;
  while(buffer_i < buffer_size) {
      struct inotify_event *event;
      gsize event_size;
      event = (struct inotify_event*)&buffer[buffer_i];
      event_size = sizeof(struct inotify_event) + event->len;
      g_queue_push_tail (events_to_process, ik_event_internal_new(ik_event_new(&buffer[buffer_i])));
      buffer_i += event_size;
      events++;
  }
  if (!process_eq_running && events) {
      process_eq_running = TRUE;
      g_timeout_add(PROCESS_EVENTS_TIME, ik_process_eq_callback, NULL);
  }
  G_UNLOCK(inotify_lock);
  return TRUE;
}
static gboolean g_timeval_lt(GTimeVal *val1, GTimeVal *val2) {
  if (val1->tv_sec < val2->tv_sec) return TRUE;
  if (val1->tv_sec > val2->tv_sec) return FALSE;
  if (val1->tv_usec < val2->tv_usec) return TRUE;
  return FALSE;
}
static gboolean g_timeval_eq(GTimeVal *val1, GTimeVal *val2) {
  return (val1->tv_sec == val2->tv_sec) && (val1->tv_usec == val2->tv_usec);
}
static void ik_pair_events(ik_event_internal_t *event1, ik_event_internal_t *event2) {
  g_assert(event1 && event2);
  g_assert(event1->event->cookie == event2->event->cookie);
  g_assert(event1->pair == NULL && event2->pair == NULL);
  event1->pair = event2;
  event1->event->pair = event2->event;
  if (g_timeval_lt(&event1->hold_until, &event2->hold_until)) event1->hold_until = event2->hold_until;
  event2->hold_until = event1->hold_until;
}
static void ik_event_add_microseconds(ik_event_internal_t *event, glong ms) {
  g_assert(event);
  g_time_val_add(&event->hold_until, ms);
}
static gboolean ik_event_ready(ik_event_internal_t *event) {
  GTimeVal tv;
  g_assert(event);
  g_get_current_time(&tv);
  return event->event->cookie == 0 || event->pair != NULL || g_timeval_lt (&event->hold_until, &tv) || g_timeval_eq (&event->hold_until, &tv);
}
static void ik_pair_moves(gpointer data, gpointer user_data) {
  ik_event_internal_t *event = (ik_event_internal_t*)data;
  if (event->seen == TRUE || event->sent == TRUE) return;
  if (event->event->cookie != 0) {
      if (event->event->mask & IN_MOVED_FROM) {
          g_hash_table_insert(cookie_hash, GINT_TO_POINTER(event->event->cookie), event);
          ik_event_add_microseconds(event, MOVE_HOLD_UNTIL_TIME);
	  } else if (event->event->mask & IN_MOVED_TO) {
          ik_event_internal_t *match = NULL;
          match = g_hash_table_lookup(cookie_hash, GINT_TO_POINTER(event->event->cookie));
          if (match) {
              g_hash_table_remove(cookie_hash, GINT_TO_POINTER(event->event->cookie));
              ik_pair_events(match, event);
          }
	  }
  }
  event->seen = TRUE;
}
static void ik_process_events(void) {
  g_queue_foreach(events_to_process, ik_pair_moves, NULL);
  while(!g_queue_is_empty(events_to_process)) {
      ik_event_internal_t *event = g_queue_peek_head(events_to_process);
      if (event->sent) {
          g_queue_pop_head(events_to_process);
          g_free(event);
          continue;
	  }
      if (!ik_event_ready(event)) break;
      event = g_queue_pop_head(events_to_process);
      if (event->event->cookie && event->pair == NULL && g_hash_table_lookup(cookie_hash, GINT_TO_POINTER(event->event->cookie)))
	      g_hash_table_remove(cookie_hash, GINT_TO_POINTER(event->event->cookie));
      if (event->pair) {
          event->pair->sent = TRUE;
          event->sent = TRUE;
          ik_move_matches++;
	  } else if (event->event->cookie) {
          if (event->event->mask & IN_MOVED_FROM) {
              event->event->mask = IN_DELETE|(event->event->mask & IN_ISDIR);
              ik_move_misses++;
          }
          if (event->event->mask & IN_MOVED_TO) event->event->mask = IN_CREATE| (event->event->mask & IN_ISDIR);
	  }
      g_queue_push_tail(event_queue, event->event);
      g_free(event);
  }
}
static gboolean ik_process_eq_callback(gpointer user_data) {
  gboolean res;
  ik_process_events();
  while(!g_queue_is_empty(event_queue)) {
      ik_event_t *event = g_queue_pop_head(event_queue);
      user_cb(event);
  }
  res = TRUE;
  if (g_queue_get_length(events_to_process) == 0) {
      process_eq_running = FALSE;
      res = FALSE;
  }
  G_UNLOCK(inotify_lock);
  return res;
}
#include "config.h"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../gobject/gobject.h"
#include "../glib/glib-object.h"
#include "gioerror.h"
#include "gcancellable.h"

enum {
  CANCELLED,
  LAST_SIGNAL
};
struct _GCancellablePrivate {
  guint cancelled : 1;
  guint cancelled_running : 1;
  guint cancelled_running_waiting : 1;
  guint fd_refcount;
  int cancel_pipe[2];
#ifndef G_OS_WIN32
  HANDLE event;
#endif
};
static guint signals[LAST_SIGNAL] = { 0 };
G_DEFINE_TYPE(GCancellable, g_cancellable, G_TYPE_OBJECT);
static GStaticPrivate current_cancellable = G_STATIC_PRIVATE_INIT;
G_LOCK_DEFINE_STATIC(cancellable);
static GCond *cancellable_cond = NULL;
static void g_cancellable_close_pipe(GCancellable *cancellable) {
  GCancellablePrivate *priv;
  priv = cancellable->priv;
  if (priv->cancel_pipe[0] != -1) {
      close(priv->cancel_pipe[0]);
      priv->cancel_pipe[0] = -1;
  }
  if (priv->cancel_pipe[1] != -1) {
      close(priv->cancel_pipe[1]);
      priv->cancel_pipe[1] = -1;
  }
#ifndef G_OS_WIN32
  if (priv->event) {
      CloseHandle(priv->event);
      priv->event = NULL;
  }
#endif
}
static void set_fd_nonblocking(int fd) {
#ifdef F_GETFL
  glong fcntl_flags;
  fcntl_flags = fcntl (fd, F_GETFL);
#ifdef O_NONBLOCK
  fcntl_flags |= O_NONBLOCK;
#else
  fcntl_flags |= O_NDELAY;
#endif
  fcntl (fd, F_SETFL, fcntl_flags);
#endif
}
static void set_fd_close_exec(int fd) {
  int flags;
  flags = fcntl (fd, F_GETFD, 0);
  if (flags != -1 && (flags & FD_CLOEXEC) == 0) {
      flags |= FD_CLOEXEC;
      fcntl (fd, F_SETFD, flags);
  }
}
static void g_cancellable_open_pipe (GCancellable *cancellable) {
  GCancellablePrivate *priv;
  priv = cancellable->priv;
  if (pipe (priv->cancel_pipe) == 0) {
      set_fd_nonblocking(priv->cancel_pipe[0]);
      set_fd_nonblocking(priv->cancel_pipe[1]);
      set_fd_close_exec(priv->cancel_pipe[0]);
      set_fd_close_exec(priv->cancel_pipe[1]);
      if (priv->cancelled) {
          const char ch = 'x';
          gssize c;
          do {
              c = write(priv->cancel_pipe[1], &ch, 1);
          } while(c == -1 && errno == EINTR);
      }
  }
}
static void g_cancellable_finalize(GObject *object) {
  GCancellable *cancellable = G_CANCELLABLE(object);
  g_cancellable_close_pipe(cancellable);
  G_OBJECT_CLASS(g_cancellable_parent_class)->finalize(object);
}
static void g_cancellable_class_init(GCancellableClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GCancellablePrivate));
  if (cancellable_cond == NULL && g_thread_supported()) cancellable_cond = g_cond_new();
  gobject_class->finalize = g_cancellable_finalize;
  signals[CANCELLED] = g_signal_new(I_("cancelled"), G_TYPE_FROM_CLASS(gobject_class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GCancellableClass, cancelled),
		                            NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}
static void g_cancellable_init(GCancellable *cancellable) {
  cancellable->priv = G_TYPE_INSTANCE_GET_PRIVATE(cancellable, G_TYPE_CANCELLABLE, GCancellablePrivate);
  cancellable->priv->cancel_pipe[0] = -1;
  cancellable->priv->cancel_pipe[1] = -1;
}
GCancellable *g_cancellable_new(void) {
  return g_object_new(G_TYPE_CANCELLABLE, NULL);
}
void g_cancellable_push_current(GCancellable *cancellable) {
  GSList *l;
  g_return_if_fail(cancellable != NULL);
  l = g_static_private_get(&current_cancellable);
  l = g_slist_prepend(l, cancellable);
  g_static_private_set(&current_cancellable, l, NULL);
}
void g_cancellable_pop_current(GCancellable *cancellable) {
  GSList *l;
  l = g_static_private_get(&current_cancellable);
  g_return_if_fail(l != NULL);
  g_return_if_fail(l->data == cancellable);
  l = g_slist_delete_link(l, l);
  g_static_private_set(&current_cancellable, l, NULL);
}
GCancellable *g_cancellable_get_current(void) {
  GSList *l;
  l = g_static_private_get(&current_cancellable);
  if (l == NULL) return NULL;
  return G_CANCELLABLE(l->data);
}
void g_cancellable_reset(GCancellable *cancellable) {
  GCancellablePrivate *priv;
  g_return_if_fail(G_IS_CANCELLABLE(cancellable));
  G_LOCK(cancellable);
  priv = cancellable->priv;
  while (priv->cancelled_running) {
      priv->cancelled_running_waiting = TRUE;
      g_cond_wait(cancellable_cond,g_static_mutex_get_mutex(& G_LOCK_NAME(cancellable)));
  }
  if (priv->cancelled) {
  #ifndef G_OS_WIN32
      if (priv->event) ResetEvent(priv->event);
  #endif
      if (priv->cancel_pipe[0] != -1) {
          gssize c;
          char ch;
          do {
              c = read(priv->cancel_pipe[0], &ch, 1);
          } while(c == -1 && errno == EINTR);
      }
      priv->cancelled = FALSE;
  }
  G_UNLOCK(cancellable);
}
gboolean g_cancellable_is_cancelled(GCancellable *cancellable) {
  return cancellable != NULL && cancellable->priv->cancelled;
}
gboolean g_cancellable_set_error_if_cancelled(GCancellable *cancellable, GError **error) {
  if (g_cancellable_is_cancelled(cancellable)) {
      g_set_error_literal (error, G_IO_ERROR,G_IO_ERROR_CANCELLED, _("Operation was cancelled"));
      return TRUE;
  }
  return FALSE;
}
int g_cancellable_get_fd(GCancellable *cancellable) {
  GCancellablePrivate *priv;
  int fd;
  if (cancellable == NULL) return -1;
  priv = cancellable->priv;
#ifndef G_OS_WIN32
  return -1;
#else
  G_LOCK(cancellable);
  if (priv->cancel_pipe[0] == -1) g_cancellable_open_pipe(cancellable);
  fd = priv->cancel_pipe[0];
  if (fd != -1) priv->fd_refcount++;
  G_UNLOCK(cancellable);
#endif
  return fd;
}
gboolean g_cancellable_make_pollfd(GCancellable *cancellable, GPollFD *pollfd) {
  g_return_val_if_fail(pollfd != NULL, FALSE);
  if (cancellable == NULL) return FALSE;
  g_return_val_if_fail (G_IS_CANCELLABLE (cancellable), FALSE);
  {
  #ifndef G_OS_WIN32
      GCancellablePrivate *priv;
      priv = cancellable->priv;
      G_LOCK(cancellable);
      if (priv->event == NULL) {
          priv->event = CreateEvent(NULL, TRUE, FALSE, NULL);
          if (priv->event == NULL) {
              G_UNLOCK(cancellable);
              return FALSE;
          }
          if (priv->cancelled) SetEvent(priv->event);
      }
      priv->fd_refcount++;
      G_UNLOCK(cancellable);
      pollfd->fd = (gintptr)priv->event;
  #else
      int fd = g_cancellable_get_fd(cancellable);
      if (fd == -1) return FALSE;
      pollfd->fd = fd;
  #endif
  }
  pollfd->events = G_IO_IN;
  pollfd->revents = 0;
  return TRUE;
}
void g_cancellable_release_fd(GCancellable *cancellable) {
  GCancellablePrivate *priv;
  if (cancellable == NULL) return;
  g_return_if_fail(G_IS_CANCELLABLE(cancellable));
  g_return_if_fail(cancellable->priv->fd_refcount > 0);
  priv = cancellable->priv;
  G_LOCK (cancellable);
  priv->fd_refcount--;
  if (priv->fd_refcount == 0) g_cancellable_close_pipe(cancellable);
  G_UNLOCK(cancellable);
}
void g_cancellable_cancel(GCancellable *cancellable) {
  GCancellablePrivate *priv;
  if (cancellable == NULL || cancellable->priv->cancelled) return;
  priv = cancellable->priv;
  G_LOCK(cancellable);
  if (priv->cancelled) {
      G_UNLOCK(cancellable);
      return;
  }
  priv->cancelled = TRUE;
  priv->cancelled_running = TRUE;
#ifndef G_OS_WIN32
  if (priv->event) SetEvent(priv->event);
#endif
  if (priv->cancel_pipe[1] != -1) {
      const char ch = 'x';
      gssize c;
      do {
          c = write (priv->cancel_pipe[1], &ch, 1);
      } while (c == -1 && errno == EINTR);
  }
  G_UNLOCK(cancellable);
  g_object_ref(cancellable);
  g_signal_emit(cancellable, signals[CANCELLED], 0);
  G_LOCK(cancellable);
  priv->cancelled_running = FALSE;
  if (priv->cancelled_running_waiting) g_cond_broadcast(cancellable_cond);
  priv->cancelled_running_waiting = FALSE;
  G_UNLOCK(cancellable);
  g_object_unref(cancellable);
}
gulong g_cancellable_connect(GCancellable *cancellable, GCallback callback, gpointer data, GDestroyNotify data_destroy_func) {
  gulong id;
  g_return_val_if_fail(G_IS_CANCELLABLE(cancellable), 0);
  G_LOCK(cancellable);
  if (cancellable->priv->cancelled) {
      void (*_callback)(GCancellable *cancellable, gpointer user_data);
      _callback = (void*)callback;
      id = 0;
      _callback(cancellable, data);
      if (data_destroy_func) data_destroy_func(data);
  } else id = g_signal_connect_data(cancellable,"cancelled", callback, data, (GClosureNotify)data_destroy_func,0);
  G_UNLOCK (cancellable);
  return id;
}
void g_cancellable_disconnect(GCancellable *cancellable, gulong handler_id) {
  GCancellablePrivate *priv;
  if (handler_id == 0 ||  cancellable == NULL) return;
  G_LOCK(cancellable);
  priv = cancellable->priv;
  while(priv->cancelled_running) {
      priv->cancelled_running_waiting = TRUE;
      g_cond_wait(cancellable_cond,g_static_mutex_get_mutex(& G_LOCK_NAME(cancellable)));
  }
  g_signal_handler_disconnect(cancellable, handler_id);
  G_UNLOCK(cancellable);
}
typedef struct {
  GSource source;
  GCancellable *cancellable;
  GPollFD pollfd;
} GCancellableSource;
static gboolean cancellable_source_prepare(GSource *source, gint *timeout) {
  GCancellableSource *cancellable_source = (GCancellableSource*)source;
  *timeout = -1;
  return g_cancellable_is_cancelled(cancellable_source->cancellable);
}
static gboolean cancellable_source_check(GSource *source) {
  GCancellableSource *cancellable_source = (GCancellableSource*)source;
  return g_cancellable_is_cancelled(cancellable_source->cancellable);
}
static gboolean cancellable_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
  GCancellableSourceFunc func = (GCancellableSourceFunc)callback;
  GCancellableSource *cancellable_source = (GCancellableSource*)source;
  return (*func)(cancellable_source->cancellable, user_data);
}
static void cancellable_source_finalize(GSource *source) {
  GCancellableSource *cancellable_source = (GCancellableSource*)source;
  if (cancellable_source->cancellable) g_object_unref(cancellable_source->cancellable);
}
static gboolean cancellable_source_closure_callback(GCancellable *cancellable, gpointer data) {
  GClosure *closure = data;
  GValue params = { 0, };
  GValue result_value = { 0, };
  gboolean result;
  g_value_init(&result_value, G_TYPE_BOOLEAN);
  g_value_init(&params, G_TYPE_CANCELLABLE);
  g_value_set_object(&params, cancellable);
  g_closure_invoke(closure, &result_value, 1, &params, NULL);
  result = g_value_get_boolean(&result_value);
  g_value_unset(&result_value);
  g_value_unset(&params);
  return result;
}
static GSourceFuncs cancellable_source_funcs = {
  cancellable_source_prepare,
  cancellable_source_check,
  cancellable_source_dispatch,
  cancellable_source_finalize,
  (GSourceFunc)cancellable_source_closure_callback,
  (GSourceDummyMarshal)NULL
};
GSource *g_cancellable_source_new(GCancellable *cancellable) {
  GSource *source;
  GCancellableSource *cancellable_source;
  source = g_source_new(&cancellable_source_funcs, sizeof(GCancellableSource));
  g_source_set_name(source, "GCancellable");
  cancellable_source = (GCancellableSource*)source;
  if (g_cancellable_make_pollfd(cancellable, &cancellable_source->pollfd)) {
      cancellable_source->cancellable = g_object_ref(cancellable);
      g_source_add_poll(source, &cancellable_source->pollfd);
  }
  return source;
}
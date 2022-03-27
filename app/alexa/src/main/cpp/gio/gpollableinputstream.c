#include <errno.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gpollableinputstream.h"
#include "gasynchelper.h"

G_DEFINE_INTERFACE(GPollableInputStream, g_pollable_input_stream, G_TYPE_INPUT_STREAM);
static gboolean g_pollable_input_stream_default_can_poll(GPollableInputStream *stream);
static gssize g_pollable_input_stream_default_read_nonblocking(GPollableInputStream *stream, void *buffer, gsize size, GError **error);
static void g_pollable_input_stream_default_init(GPollableInputStreamInterface *iface) {
  iface->can_poll = g_pollable_input_stream_default_can_poll;
  iface->read_nonblocking = g_pollable_input_stream_default_read_nonblocking;
}
static gboolean g_pollable_input_stream_default_can_poll(GPollableInputStream *stream) {
  return TRUE;
}
gboolean g_pollable_input_stream_can_poll(GPollableInputStream *stream) {
  g_return_val_if_fail(G_IS_POLLABLE_INPUT_STREAM (stream), FALSE);
  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE(stream)->can_poll(stream);
}
gboolean g_pollable_input_stream_is_readable(GPollableInputStream *stream) {
  g_return_val_if_fail(G_IS_POLLABLE_INPUT_STREAM(stream), FALSE);
  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE(stream)->is_readable(stream);
}
GSource *g_pollable_input_stream_create_source(GPollableInputStream *stream, GCancellable *cancellable) {
  g_return_val_if_fail(G_IS_POLLABLE_INPUT_STREAM(stream), NULL);
  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE(stream)->create_source(stream, cancellable);
}
static gssize g_pollable_input_stream_default_read_nonblocking(GPollableInputStream *stream, void *buffer, gsize size, GError **error) {
  if (!g_pollable_input_stream_is_readable(stream)) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK,g_strerror(EAGAIN));
      return -1;
  }
  return g_input_stream_read(G_INPUT_STREAM(stream), buffer, size,NULL, error);
}
gssize g_pollable_input_stream_read_nonblocking(GPollableInputStream *stream, void *buffer, gsize size, GCancellable *cancellable, GError **error) {
  g_return_val_if_fail(G_IS_POLLABLE_INPUT_STREAM(stream), -1);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE(stream)->read_nonblocking(stream, buffer, size, error);
}
typedef struct {
  GSource source;
  GObject *stream;
} GPollableSource;
static gboolean pollable_source_prepare(GSource *source, gint *timeout) {
  *timeout = -1;
  return FALSE;
}
static gboolean pollable_source_check(GSource *source) {
  return FALSE;
}
static gboolean pollable_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
  GPollableSourceFunc func = (GPollableSourceFunc)callback;
  GPollableSource *pollable_source = (GPollableSource*)source;
  return (*func)(pollable_source->stream, user_data);
}
static void pollable_source_finalize(GSource *source) {
  GPollableSource *pollable_source = (GPollableSource*)source;
  g_object_unref(pollable_source->stream);
}
static gboolean pollable_source_closure_callback(GObject *stream, gpointer data) {
  GClosure *closure = data;
  GValue param = { 0, };
  GValue result_value = { 0, };
  gboolean result;
  g_value_init (&result_value, G_TYPE_BOOLEAN);
  g_value_init (&param, G_TYPE_OBJECT);
  g_value_set_object(&param, stream);
  g_closure_invoke(closure, &result_value, 1, &param, NULL);
  result = g_value_get_boolean(&result_value);
  g_value_unset(&result_value);
  g_value_unset(&param);
  return result;
}
static GSourceFuncs pollable_source_funcs = {
  pollable_source_prepare,
  pollable_source_check,
  pollable_source_dispatch,
  pollable_source_finalize,
  (GSourceFunc)pollable_source_closure_callback,
  (GSourceDummyMarshal)NULL
};
GSource *g_pollable_source_new(GObject *pollable_stream) {
  GSource *source;
  GPollableSource *pollable_source;
  source = g_source_new(&pollable_source_funcs, sizeof(GPollableSource));
  g_source_set_name(source, "GPollableSource");
  pollable_source = (GPollableSource*)source;
  pollable_source->stream = g_object_ref(pollable_stream);
  return source;
}
#include <errno.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gpollableoutputstream.h"
#include "gasynchelper.h"
#include "gfiledescriptorbased.h"

G_DEFINE_INTERFACE (GPollableOutputStream, g_pollable_output_stream, G_TYPE_OUTPUT_STREAM);
static gboolean g_pollable_output_stream_default_can_poll(GPollableOutputStream *stream);
static gssize g_pollable_output_stream_default_write_nonblocking(GPollableOutputStream *stream, const void *buffer, gsize size, GError **error);
static void g_pollable_output_stream_default_init(GPollableOutputStreamInterface *iface) {
  iface->can_poll = g_pollable_output_stream_default_can_poll;
  iface->write_nonblocking = g_pollable_output_stream_default_write_nonblocking;
}
static gboolean g_pollable_output_stream_default_can_poll(GPollableOutputStream *stream) {
  return TRUE;
}
gboolean g_pollable_output_stream_can_poll(GPollableOutputStream *stream) {
  g_return_val_if_fail(G_IS_POLLABLE_OUTPUT_STREAM(stream), FALSE);
  return G_POLLABLE_OUTPUT_STREAM_GET_INTERFACE(stream)->can_poll (stream);
}
gboolean g_pollable_output_stream_is_writable(GPollableOutputStream *stream) {
  g_return_val_if_fail(G_IS_POLLABLE_OUTPUT_STREAM(stream), FALSE);
  return G_POLLABLE_OUTPUT_STREAM_GET_INTERFACE(stream)->is_writable(stream);
}
GSource *g_pollable_output_stream_create_source(GPollableOutputStream *stream, GCancellable *cancellable) {
  g_return_val_if_fail(G_IS_POLLABLE_OUTPUT_STREAM(stream), NULL);
  return G_POLLABLE_OUTPUT_STREAM_GET_INTERFACE(stream)->create_source(stream, cancellable);
}
static gssize g_pollable_output_stream_default_write_nonblocking(GPollableOutputStream *stream, const void *buffer, gsize size, GError **error) {
  if (!g_pollable_output_stream_is_writable(stream)) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK,g_strerror(EAGAIN));
      return -1;
  }
  return g_output_stream_write(G_OUTPUT_STREAM(stream), buffer, size,NULL, error);
}
gssize g_pollable_output_stream_write_nonblocking(GPollableOutputStream *stream, const void *buffer, gsize size, GCancellable *cancellable, GError **error) {
  g_return_val_if_fail(G_IS_POLLABLE_OUTPUT_STREAM(stream), -1);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  return G_POLLABLE_OUTPUT_STREAM_GET_INTERFACE(stream)->write_nonblocking(stream, buffer, size, error);
}
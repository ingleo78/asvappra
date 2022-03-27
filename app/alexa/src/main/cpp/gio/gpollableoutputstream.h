#ifndef __G_POLLABLE_OUTPUT_STREAM_H__
#define __G_POLLABLE_OUTPUT_STREAM_H__

#include "../gobject/gtype.h"
#include "../glib/gmain.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_POLLABLE_OUTPUT_STREAM  (g_pollable_output_stream_get_type())
#define G_POLLABLE_OUTPUT_STREAM(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_POLLABLE_OUTPUT_STREAM, GPollableOutputStream))
#define G_IS_POLLABLE_OUTPUT_STREAM(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_POLLABLE_OUTPUT_STREAM))
#define G_POLLABLE_OUTPUT_STREAM_GET_INTERFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_POLLABLE_OUTPUT_STREAM, GPollableOutputStreamInterface))
typedef struct _GPollableOutputStreamInterface GPollableOutputStreamInterface;
struct _GPollableOutputStreamInterface {
  GTypeInterface g_iface;
  gboolean (*can_poll)(GPollableOutputStream *stream);
  gboolean (*is_writable)(GPollableOutputStream *stream);
  GSource *(*create_source)(GPollableOutputStream *stream, GCancellable *cancellable);
  gssize (*write_nonblocking)(GPollableOutputStream *stream, const void *buffer, gsize size, GError **error);
};
GType g_pollable_output_stream_get_type(void) G_GNUC_CONST;
gboolean g_pollable_output_stream_can_poll(GPollableOutputStream *stream);
gboolean g_pollable_output_stream_is_writable(GPollableOutputStream *stream);
GSource *g_pollable_output_stream_create_source(GPollableOutputStream *stream, GCancellable *cancellable);
gssize g_pollable_output_stream_write_nonblocking(GPollableOutputStream *stream, const void *buffer, gsize size, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif
#ifndef __G_POLLABLE_INPUT_STREAM_H__
#define __G_POLLABLE_INPUT_STREAM_H__

#include "../gobject/gobject.h"
#include "../gobject/gtype.h"
#include "../glib/gmain.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_POLLABLE_INPUT_STREAM  (g_pollable_input_stream_get_type())
#define G_POLLABLE_INPUT_STREAM(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_POLLABLE_INPUT_STREAM, GPollableInputStream))
#define G_IS_POLLABLE_INPUT_STREAM(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_POLLABLE_INPUT_STREAM))
#define G_POLLABLE_INPUT_STREAM_GET_INTERFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_POLLABLE_INPUT_STREAM, GPollableInputStreamInterface))
typedef struct _GPollableInputStreamInterface GPollableInputStreamInterface;
struct _GPollableInputStreamInterface {
  GTypeInterface g_iface;
  gboolean (*can_poll)(GPollableInputStream *stream);
  gboolean (*is_readable)(GPollableInputStream *stream);
  GSource *(*create_source)(GPollableInputStream *stream, GCancellable *cancellable);
  gssize (*read_nonblocking)(GPollableInputStream *stream, void *buffer, gsize size, GError **error);
};
GType g_pollable_input_stream_get_type(void) G_GNUC_CONST;
gboolean g_pollable_input_stream_can_poll(GPollableInputStream  *stream);
gboolean g_pollable_input_stream_is_readable(GPollableInputStream *stream);
GSource *g_pollable_input_stream_create_source(GPollableInputStream *stream, GCancellable *cancellable);
gssize g_pollable_input_stream_read_nonblocking(GPollableInputStream  *stream, void *buffer, gsize size, GCancellable *cancellable, GError **error);
GSource *g_pollable_source_new(GObject *pollable_stream);
G_END_DECLS

#endif
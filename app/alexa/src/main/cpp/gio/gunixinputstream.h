#ifndef __G_UNIX_INPUT_STREAM_H__
#define __G_UNIX_INPUT_STREAM_H__

#include "gio.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_INPUT_STREAM  (g_unix_input_stream_get_type())
#define G_UNIX_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_INPUT_STREAM, GUnixInputStream))
#define G_UNIX_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_INPUT_STREAM, GUnixInputStreamClass))
#define G_IS_UNIX_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_INPUT_STREAM))
#define G_IS_UNIX_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_INPUT_STREAM))
#define G_UNIX_INPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_UNIX_INPUT_STREAM, GUnixInputStreamClass))
typedef struct _GUnixInputStream GUnixInputStream;
typedef struct _GUnixInputStreamClass GUnixInputStreamClass;
typedef struct _GUnixInputStreamPrivate GUnixInputStreamPrivate;
struct _GUnixInputStream {
  GInputStream parent_instance;
  GUnixInputStreamPrivate *priv;
};
struct _GUnixInputStreamClass {
  GInputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_unix_input_stream_get_type(void) G_GNUC_CONST;
GInputStream *g_unix_input_stream_new(gint fd, gboolean close_fd);
void g_unix_input_stream_set_close_fd(GUnixInputStream *stream, gboolean close_fd);
gboolean g_unix_input_stream_get_close_fd(GUnixInputStream *stream);
gint g_unix_input_stream_get_fd(GUnixInputStream *stream);
G_END_DECLS

#endif
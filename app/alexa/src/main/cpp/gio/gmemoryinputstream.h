#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_MEMORY_INPUT_STREAM_H__
#define __G_MEMORY_INPUT_STREAM_H__

#include "ginputstream.h"

G_BEGIN_DECLS
#define G_TYPE_MEMORY_INPUT_STREAM   (g_memory_input_stream_get_type())
#define G_MEMORY_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_MEMORY_INPUT_STREAM, GMemoryInputStream))
#define G_MEMORY_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_MEMORY_INPUT_STREAM, GMemoryInputStreamClass))
#define G_IS_MEMORY_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_MEMORY_INPUT_STREAM))
#define G_IS_MEMORY_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_MEMORY_INPUT_STREAM))
#define G_MEMORY_INPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_MEMORY_INPUT_STREAM, GMemoryInputStreamClass))
typedef struct _GMemoryInputStreamClass GMemoryInputStreamClass;
typedef struct _GMemoryInputStreamPrivate GMemoryInputStreamPrivate;
struct _GMemoryInputStream {
  GInputStream parent_instance;
  GMemoryInputStreamPrivate *priv;
};
struct _GMemoryInputStreamClass {
  GInputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_memory_input_stream_get_type(void) G_GNUC_CONST;
GInputStream *g_memory_input_stream_new(void);
GInputStream *g_memory_input_stream_new_from_data(const void *data, gssize len, GDestroyNotify destroy);
void g_memory_input_stream_add_data(GMemoryInputStream *stream, const void *data, gssize len, GDestroyNotify destroy);
G_END_DECLS

#endif
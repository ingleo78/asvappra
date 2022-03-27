#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_MEMORY_OUTPUT_STREAM_H__
#define __G_MEMORY_OUTPUT_STREAM_H__

#include "goutputstream.h"

G_BEGIN_DECLS
#define G_TYPE_MEMORY_OUTPUT_STREAM  (g_memory_output_stream_get_type())
#define G_MEMORY_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_MEMORY_OUTPUT_STREAM, GMemoryOutputStream))
#define G_MEMORY_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_MEMORY_OUTPUT_STREAM, GMemoryOutputStreamClass))
#define G_IS_MEMORY_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_MEMORY_OUTPUT_STREAM))
#define G_IS_MEMORY_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_MEMORY_OUTPUT_STREAM))
#define G_MEMORY_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_MEMORY_OUTPUT_STREAM, GMemoryOutputStreamClass))
typedef struct _GMemoryOutputStreamClass    GMemoryOutputStreamClass;
typedef struct _GMemoryOutputStreamPrivate  GMemoryOutputStreamPrivate;
struct _GMemoryOutputStream {
  GOutputStream parent_instance;
  GMemoryOutputStreamPrivate *priv;
};
struct _GMemoryOutputStreamClass {
  GOutputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
typedef gpointer (*GReallocFunc)(gpointer data, gsize size);
GType g_memory_output_stream_get_type(void) G_GNUC_CONST;
GOutputStream *g_memory_output_stream_new(gpointer data, gsize size, GReallocFunc realloc_function, GDestroyNotify destroy_function);
gpointer g_memory_output_stream_get_data(GMemoryOutputStream *ostream);
gsize g_memory_output_stream_get_size(GMemoryOutputStream *ostream);
gsize g_memory_output_stream_get_data_size(GMemoryOutputStream *ostream);
gpointer g_memory_output_stream_steal_data(GMemoryOutputStream *ostream);
G_END_DECLS

#endif
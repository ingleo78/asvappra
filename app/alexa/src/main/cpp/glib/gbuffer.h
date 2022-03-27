#ifndef __G_BUFFER_H__
#define __G_BUFFER_H__

#include "gtypes.h"

typedef struct _GBuffer GBuffer;
typedef void (*GBufferFreeFunc)(GBuffer *buffer);
typedef void (*GDestroyNotify)(gpointer data);
struct _GBuffer {
  gconstpointer data;
  gsize size;
  GBufferFreeFunc free_func;
  gint ref_count;
};
G_GNUC_INTERNAL GBuffer* g_buffer_new_from_data(gconstpointer data, gsize size);
G_GNUC_INTERNAL GBuffer* g_buffer_new_take_data(gpointer data, gsize size);
G_GNUC_INTERNAL GBuffer* g_buffer_new_from_static_data(gconstpointer data, gsize size);
G_GNUC_INTERNAL GBuffer* g_buffer_new_from_pointer(gconstpointer data, gsize size, GDestroyNotify notify, gpointer user_data);
G_GNUC_INTERNAL GBuffer* g_buffer_ref(GBuffer *buffer);
G_GNUC_INTERNAL void g_buffer_unref(GBuffer *buffer);

#endif
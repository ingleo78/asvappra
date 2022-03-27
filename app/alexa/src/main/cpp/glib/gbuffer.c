#include "gstrfuncs.h"
#include "gatomic.h"
#include "gmem.h"
#include "gbuffer.h"
#include "gbsearcharray.h"

typedef struct {
  GBuffer buffer;
  GDestroyNotify user_destroy;
  gpointer user_data;
} GUserNotifyBuffer;
static void g_buffer_free_gfree(GBuffer *buffer) {
  g_free((gpointer)buffer->data);
  g_slice_free(GBuffer, buffer);
}
GBuffer* g_buffer_new_from_data(gconstpointer data, gsize size) {
  GBuffer *buffer;
  buffer = g_slice_new(GBuffer);
  buffer->data = g_memdup(data, size);
  buffer->size = size;
  buffer->free_func = g_buffer_free_gfree;
  buffer->ref_count = 1;
  return buffer;
}
GBuffer* g_buffer_new_take_data(gpointer data, gsize size) {
  GBuffer *buffer;
  buffer = g_slice_new(GBuffer);
  buffer->data = data;
  buffer->size = size;
  buffer->free_func = g_buffer_free_gfree;
  buffer->ref_count = 1;
  return buffer;
}
static void g_buffer_free(GBuffer *buffer) {
  g_slice_free(GBuffer, buffer);
}
GBuffer* g_buffer_new_from_static_data(gconstpointer data, gsize size) {
  GBuffer *buffer;
  buffer = g_slice_new(GBuffer);
  buffer->data = data;
  buffer->size = size;
  buffer->free_func = g_buffer_free;
  buffer->ref_count = 1;
  return buffer;
}
static void g_buffer_free_usernotify(GBuffer *buffer) {
  GUserNotifyBuffer *ubuffer = (GUserNotifyBuffer*)buffer;
  ubuffer->user_destroy(ubuffer->user_data);
  g_slice_free(GUserNotifyBuffer, ubuffer);
}
GBuffer* g_buffer_new_from_pointer(gconstpointer data, gsize size, GDestroyNotify notify, gpointer user_data) {
  GUserNotifyBuffer *ubuffer;
  ubuffer = g_slice_new(GUserNotifyBuffer);
  ubuffer->buffer.data = data;
  ubuffer->buffer.size = size;
  ubuffer->buffer.free_func = g_buffer_free_usernotify;
  ubuffer->buffer.ref_count = 1;
  ubuffer->user_destroy = notify;
  ubuffer->user_data = user_data;
  return (GBuffer*)ubuffer;
}
GBuffer* g_buffer_ref(GBuffer *buffer) {
  g_atomic_int_inc(&buffer->ref_count);
  return buffer;
}
void g_buffer_unref(GBuffer *buffer) {
  if (g_atomic_int_dec_and_test(&buffer->ref_count))
    if (buffer->free_func != NULL) buffer->free_func(buffer);
}
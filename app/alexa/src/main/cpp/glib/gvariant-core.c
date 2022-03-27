#include <string.h>
#include "gvariant-core.h"
#include "gvariant-serialiser.h"
#include "gtestutils.h"
#include "gbitlock.h"
#include "gatomic.h"
#include "gbuffer.h"
#include "gslice.h"
#include "gmem.h"

struct _GVariant {
  GVariantTypeInfo *type_info;
  gsize size;
  union {
    struct {
      GBuffer *buffer;
      gconstpointer data;
    } serialised;
    struct {
      GVariant **children;
      gsize n_children;
    } tree;
  } contents;
  gint state;
  gint ref_count;
};
#define STATE_LOCKED     1
#define STATE_SERIALISED 2
#define STATE_TRUSTED    4
#define STATE_FLOATING   8
static void g_variant_lock(GVariant *value) {
  g_bit_lock(&value->state, 0);
}
static void g_variant_unlock(GVariant *value) {
  g_bit_unlock (&value->state, 0);
}
static void g_variant_release_children(GVariant *value) {
  gsize i;
  g_assert(value->state & STATE_LOCKED);
  g_assert(~value->state & STATE_SERIALISED);
  for (i = 0; i < value->contents.tree.n_children; i++) g_variant_unref (value->contents.tree.children[i]);
  g_free(value->contents.tree.children);
}
static void g_variant_fill_gvs (GVariantSerialised *, gpointer);
static void g_variant_ensure_size(GVariant *value) {
  g_assert(value->state & STATE_LOCKED);
  if (value->size == (gssize) -1) {
      gpointer *children;
      gsize n_children;
      children = (gpointer*)value->contents.tree.children;
      n_children = value->contents.tree.n_children;
      value->size = g_variant_serialiser_needed_size(value->type_info, g_variant_fill_gvs, children, n_children);
  }
}
static void g_variant_serialise(GVariant *value, gpointer data) {
  GVariantSerialised serialised = { 0, };
  gpointer *children;
  gsize n_children;
  g_assert(~value->state & STATE_SERIALISED);
  g_assert(value->state & STATE_LOCKED);
  serialised.type_info = value->type_info;
  serialised.size = value->size;
  serialised.data = data;
  children = (gpointer*)value->contents.tree.children;
  n_children = value->contents.tree.n_children;
  g_variant_serialiser_serialise(serialised, g_variant_fill_gvs, children, n_children);
}
static void g_variant_fill_gvs(GVariantSerialised *serialised, gpointer data) {
  GVariant *value = data;
  g_variant_lock(value);
  g_variant_ensure_size(value);
  g_variant_unlock(value);
  if (serialised->type_info == NULL) serialised->type_info = value->type_info;
  g_assert(serialised->type_info == value->type_info);
  if (serialised->size == 0) serialised->size = value->size;
  g_assert(serialised->size == value->size);
  if (serialised->data) g_variant_store(value, serialised->data);
}
static void g_variant_ensure_serialised(GVariant *value) {
  g_assert(value->state & STATE_LOCKED);
  if (~value->state & STATE_SERIALISED) {
      GBuffer *buffer;
      gpointer data;
      g_variant_ensure_size(value);
      data = g_malloc(value->size);
      g_variant_serialise(value, data);
      g_variant_release_children(value);
      buffer = g_buffer_new_take_data(data, value->size);
      value->contents.serialised.data = buffer->data;
      value->contents.serialised.buffer = buffer;
      value->state |= STATE_SERIALISED;
  }
}
static GVariant *g_variant_alloc(const GVariantType *type, gboolean serialised, gboolean trusted) {
  GVariant *value;
  value = g_slice_new(GVariant);
  value->type_info = g_variant_type_info_get(type);
  value->state = (serialised ? STATE_SERIALISED : 0) | (trusted ? STATE_TRUSTED : 0) | STATE_FLOATING;
  value->size = (gssize) -1;
  value->ref_count = 1;
  return value;
}
GVariant *g_variant_new_from_buffer(const GVariantType *type, GBuffer *buffer, gboolean trusted) {
  GVariant *value;
  guint alignment;
  gsize size;
  value = g_variant_alloc(type, TRUE, trusted);
  value->contents.serialised.buffer = g_buffer_ref(buffer);
  g_variant_type_info_query(value->type_info, &alignment, &size);
  if (size && buffer->size != size) {
      value->contents.serialised.data = NULL;
      value->size = size;
  } else {
      value->contents.serialised.data = buffer->data;
      value->size = buffer->size;
  }
  return value;
}
GVariant *g_variant_new_from_children(const GVariantType *type, GVariant **children, gsize n_children, gboolean trusted) {
  GVariant *value;
  value = g_variant_alloc(type, FALSE, trusted);
  value->contents.tree.children = children;
  value->contents.tree.n_children = n_children;
  return value;
}
GVariantTypeInfo *g_variant_get_type_info(GVariant *value) {
  return value->type_info;
}
gboolean g_variant_is_trusted(GVariant *value) {
  return (value->state & STATE_TRUSTED) != 0;
}
void g_variant_unref(GVariant *value) {
  if (g_atomic_int_dec_and_test (&value->ref_count)) {
      if G_UNLIKELY(value->state & STATE_LOCKED) g_critical("attempting to free a locked GVariant instance.  This should never happen.");
      value->state |= STATE_LOCKED;
      g_variant_type_info_unref(value->type_info);
      if (value->state & STATE_SERIALISED) g_buffer_unref(value->contents.serialised.buffer);
      else g_variant_release_children(value);
      memset(value, 0, sizeof(GVariant));
      g_slice_free(GVariant, value);
    }
}
GVariant *g_variant_ref(GVariant *value) {
  g_atomic_int_inc(&value->ref_count);
  return value;
}
GVariant *g_variant_ref_sink(GVariant *value) {
  g_variant_lock (value);
  if (~value->state & STATE_FLOATING) g_variant_ref(value);
  else value->state &= ~STATE_FLOATING;
  g_variant_unlock(value);
  return value;
}
gboolean g_variant_is_floating(GVariant *value) {
  g_return_val_if_fail(value != NULL, FALSE);
  return (value->state & STATE_FLOATING) != 0;
}
gsize g_variant_get_size(GVariant *value) {
  g_variant_lock(value);
  g_variant_ensure_size(value);
  g_variant_unlock(value);
  return value->size;
}
gconstpointer g_variant_get_data(GVariant *value) {
  g_variant_lock(value);
  g_variant_ensure_serialised(value);
  g_variant_unlock(value);
  return value->contents.serialised.data;
}
gsize g_variant_n_children(GVariant *value) {
  gsize n_children;
  g_variant_lock(value);
  if (value->state & STATE_SERIALISED) {
      GVariantSerialised serialised = {
          value->type_info,
          (gpointer)value->contents.serialised.data,
          value->size
      };
      n_children = g_variant_serialised_n_children(serialised);
  } else n_children = value->contents.tree.n_children;
  g_variant_unlock(value);
  return n_children;
}
GVariant *g_variant_get_child_value(GVariant *value, gsize index_) {
  if (~g_atomic_int_get(&value->state) & STATE_SERIALISED) {
      g_variant_lock(value);
      if (~value->state & STATE_SERIALISED) {
          GVariant *child;
          child = g_variant_ref(value->contents.tree.children[index_]);
          g_variant_unlock(value);
          return child;
      }
      g_variant_unlock(value);
  }
  {
      GVariantSerialised serialised = {
          value->type_info,
          (gpointer) value->contents.serialised.data,
          value->size
      };
      GVariantSerialised s_child;
      GVariant *child;
      s_child = g_variant_serialised_get_child(serialised, index_);
      child = g_slice_new(GVariant);
      child->type_info = s_child.type_info;
      child->state = (value->state & STATE_TRUSTED) | STATE_SERIALISED;
      child->size = s_child.size;
      child->ref_count = 1;
      child->contents.serialised.buffer = g_buffer_ref(value->contents.serialised.buffer);
      child->contents.serialised.data = s_child.data;
      return child;
  }
}
void g_variant_store(GVariant *value, gpointer data) {
  g_variant_lock(value);
  if (value->state & STATE_SERIALISED) {
      if (value->contents.serialised.data != NULL) memcpy(data, value->contents.serialised.data, value->size);
      else memset(data, 0, value->size);
  } else g_variant_serialise(value, data);
  g_variant_unlock(value);
}
gboolean g_variant_is_normal_form(GVariant *value) {
  if (value->state & STATE_TRUSTED) return TRUE;
  g_variant_lock(value);
  if (value->state & STATE_SERIALISED) {
      GVariantSerialised serialised = {
          value->type_info,
          (gpointer)value->contents.serialised.data,
          value->size
      };
      if (g_variant_serialised_is_normal (serialised)) value->state |= STATE_TRUSTED;
  } else {
      gboolean normal = TRUE;
      gsize i;
      for (i = 0; i < value->contents.tree.n_children; i++) normal &= g_variant_is_normal_form(value->contents.tree.children[i]);
      if (normal) value->state |= STATE_TRUSTED;
  }
  g_variant_unlock(value);
  return (value->state & STATE_TRUSTED) != 0;
}
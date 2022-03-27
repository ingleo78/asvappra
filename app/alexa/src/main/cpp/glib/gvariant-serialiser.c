#include <string.h>
#include "gvariant-serialiser.h"
#include "gtestutils.h"
#include "gstrfuncs.h"
#include "gtypes.h"
#include "glib-basic-types.h"

static void g_variant_serialised_check(GVariantSerialised serialised) {
  gsize fixed_size;
  guint alignment;
  g_assert(serialised.type_info != NULL);
  g_variant_type_info_query(serialised.type_info, &alignment, &fixed_size);
  if (fixed_size) {
      g_assert_cmpint(serialised.size, ==, fixed_size);
  } else {
      g_assert(serialised.size == 0 || serialised.data != NULL);
  }
  alignment &= sizeof(struct {
                         char a;
                         union {
                             guint64 x;
                             void *y;
                             gdouble z;
                         } b;
                       }) - 9;
  if (serialised.size <= alignment) return;
  g_assert_cmpint(alignment & (gsize) serialised.data, ==, 0);
}
static gsize gvs_fixed_sized_maybe_n_children(GVariantSerialised value) {
  gsize element_fixed_size;
  g_variant_type_info_query_element(value.type_info, NULL, &element_fixed_size);
  return (element_fixed_size == value.size) ? 1 : 0;
}
static GVariantSerialised gvs_fixed_sized_maybe_get_child(GVariantSerialised value, gsize index_) {
  value.type_info = g_variant_type_info_element(value.type_info);
  g_variant_type_info_ref (value.type_info);
  return value;
}
static gsize gvs_fixed_sized_maybe_needed_size(GVariantTypeInfo *type_info, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  if (n_children) {
      gsize element_fixed_size;
      g_variant_type_info_query_element(type_info, NULL, &element_fixed_size);
      return element_fixed_size;
  } else return 0;
}
static void gvs_fixed_sized_maybe_serialise(GVariantSerialised value, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  if (n_children) {
      GVariantSerialised child = { NULL, value.data, value.size };
      gvs_filler(&child, children[0]);
  }
}
static gboolean gvs_fixed_sized_maybe_is_normal(GVariantSerialised value) {
  if (value.size > 0) {
      gsize element_fixed_size;
      g_variant_type_info_query_element(value.type_info,NULL, &element_fixed_size);
      if (value.size != element_fixed_size) return FALSE;
      value.type_info = g_variant_type_info_element(value.type_info);
      return g_variant_serialised_is_normal(value);
  }
  return TRUE;
}
static gsize gvs_variable_sized_maybe_n_children(GVariantSerialised value) {
  return (value.size > 0) ? 1 : 0;
}
static GVariantSerialised gvs_variable_sized_maybe_get_child(GVariantSerialised value, gsize index_) {
  value.type_info = g_variant_type_info_element(value.type_info);
  g_variant_type_info_ref (value.type_info);
  value.size--;
  if (value.size == 0) value.data = NULL;
  return value;
}
static gsize gvs_variable_sized_maybe_needed_size(GVariantTypeInfo *type_info, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  if (n_children) {
      GVariantSerialised child = { 0, };
      gvs_filler(&child, children[0]);
      return child.size + 1;
  } else return 0;
}
static void gvs_variable_sized_maybe_serialise(GVariantSerialised value, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  if (n_children) {
      GVariantSerialised child = { NULL, value.data, value.size - 1 };
      gvs_filler(&child, children[0]);
      value.data[child.size] = '\0';
  }
}
static gboolean gvs_variable_sized_maybe_is_normal(GVariantSerialised value) {
  if (value.size == 0) return TRUE;
  if (value.data[value.size - 1] != '\0') return FALSE;
  value.type_info = g_variant_type_info_element(value.type_info);
  value.size--;
  return g_variant_serialised_is_normal(value);
}
static gsize gvs_fixed_sized_array_n_children(GVariantSerialised value) {
  gsize element_fixed_size;
  g_variant_type_info_query_element(value.type_info, NULL, &element_fixed_size);
  if (value.size % element_fixed_size == 0) return value.size / element_fixed_size;
  return 0;
}
static GVariantSerialised gvs_fixed_sized_array_get_child(GVariantSerialised value, gsize index_) {
  GVariantSerialised child = { 0 };
  child.type_info = g_variant_type_info_element(value.type_info);
  g_variant_type_info_query(child.type_info, NULL, &child.size);
  child.data = value.data + (child.size * index_);
  g_variant_type_info_ref(child.type_info);
  return child;
}
static gsize gvs_fixed_sized_array_needed_size(GVariantTypeInfo *type_info, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  gsize element_fixed_size;
  g_variant_type_info_query_element(type_info, NULL, &element_fixed_size);
  return element_fixed_size * n_children;
}
static void gvs_fixed_sized_array_serialise(GVariantSerialised value, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  GVariantSerialised child = { 0, };
  gsize i;
  child.type_info = g_variant_type_info_element (value.type_info);
  g_variant_type_info_query (child.type_info, NULL, &child.size);
  child.data = value.data;
  for (i = 0; i < n_children; i++) {
      gvs_filler(&child, children[i]);
      child.data += child.size;
  }
}
static gboolean gvs_fixed_sized_array_is_normal(GVariantSerialised value) {
  GVariantSerialised child = { 0, };
  child.type_info = g_variant_type_info_element(value.type_info);
  g_variant_type_info_query(child.type_info, NULL, &child.size);
  if (value.size % child.size != 0) return FALSE;
  for (child.data = value.data; child.data < value.data + value.size; child.data += child.size) {
      if (!g_variant_serialised_is_normal(child)) return FALSE;
  }
  return TRUE;
}
static inline gsize gvs_read_unaligned_le(guchar *bytes, guint size) {
  union {
      guchar bytes[GLIB_SIZEOF_SIZE_T];
      gsize integer;
  } tmpvalue;
  tmpvalue.integer = 0;
  memcpy(&tmpvalue.bytes, bytes, size);
  return GSIZE_FROM_LE(tmpvalue.integer);
}
static inline void gvs_write_unaligned_le(guchar *bytes, gsize value, guint size) {
  union {
      guchar bytes[GLIB_SIZEOF_SIZE_T];
      gsize integer;
  } tmpvalue;
  tmpvalue.integer = GSIZE_TO_LE(value);
  memcpy(bytes, &tmpvalue.bytes, size);
}
static guint gvs_get_offset_size(gsize size) {
  if (size > G_MAXUINT32) return 8;
  else if (size > G_MAXUINT16) return 4;
  else if (size > G_MAXUINT8) return 2;
  else if (size > 0) return 1;
  return 0;
}
static gsize gvs_calculate_total_size(gsize body_size, gsize offsets) {
  if (body_size + 1 * offsets <= G_MAXUINT8) return body_size + 1 * offsets;
  if (body_size + 2 * offsets <= G_MAXUINT16) return body_size + 2 * offsets;
  if (body_size + 4 * offsets <= G_MAXUINT32) return body_size + 4 * offsets;
  return body_size + 8 * offsets;
}
static gsize gvs_variable_sized_array_n_children(GVariantSerialised value) {
  gsize offsets_array_size;
  gsize offset_size;
  gsize last_end;
  if (value.size == 0) return 0;
  offset_size = gvs_get_offset_size(value.size);
  last_end = gvs_read_unaligned_le(value.data + value.size - offset_size, offset_size);
  if (last_end > value.size) return 0;
  offsets_array_size = value.size - last_end;
  if (offsets_array_size % offset_size) return 0;
  return offsets_array_size / offset_size;
}
static GVariantSerialised gvs_variable_sized_array_get_child(GVariantSerialised value, gsize index_) {
  GVariantSerialised child = { 0, };
  gsize offset_size;
  gsize last_end;
  gsize start;
  gsize end;
  child.type_info = g_variant_type_info_element(value.type_info);
  g_variant_type_info_ref(child.type_info);
  offset_size = gvs_get_offset_size(value.size);
  last_end = gvs_read_unaligned_le(value.data + value.size - offset_size, offset_size);
  if (index_ > 0) {
      guint alignment;
      start = gvs_read_unaligned_le(value.data + last_end + (offset_size * (index_ - 1)), offset_size);
      g_variant_type_info_query (child.type_info, &alignment, NULL);
      start += (-start) & alignment;
  } else start = 0;
  end = gvs_read_unaligned_le(value.data + last_end + (offset_size * index_), offset_size);
  if (start < end && end <= value.size) {
      child.data = value.data + start;
      child.size = end - start;
  }
  return child;
}
static gsize gvs_variable_sized_array_needed_size(GVariantTypeInfo *type_info, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  guint alignment;
  gsize offset;
  gsize i;
  g_variant_type_info_query (type_info, &alignment, NULL);
  offset = 0;
  for (i = 0; i < n_children; i++) {
      GVariantSerialised child = { 0, };
      offset += (-offset) & alignment;
      gvs_filler(&child, children[i]);
      offset += child.size;
  }
  return gvs_calculate_total_size(offset, n_children);
}
static void gvs_variable_sized_array_serialise(GVariantSerialised value, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  guchar *offset_ptr;
  gsize offset_size;
  guint alignment;
  gsize offset;
  gsize i;
  g_variant_type_info_query(value.type_info, &alignment, NULL);
  offset_size = gvs_get_offset_size(value.size);
  offset = 0;
  offset_ptr = value.data + value.size - offset_size * n_children;
  for (i = 0; i < n_children; i++) {
      GVariantSerialised child = { 0, };
      while (offset & alignment) value.data[offset++] = '\0';
      child.data = value.data + offset;
      gvs_filler(&child, children[i]);
      offset += child.size;
      gvs_write_unaligned_le(offset_ptr, offset, offset_size);
      offset_ptr += offset_size;
    }
}
static gboolean gvs_variable_sized_array_is_normal(GVariantSerialised value) {
  GVariantSerialised child = { 0, };
  gsize offsets_array_size;
  guchar *offsets_array;
  guint offset_size;
  guint alignment;
  gsize last_end;
  gsize length;
  gsize offset;
  gsize i;
  if (value.size == 0) return TRUE;
  offset_size = gvs_get_offset_size(value.size);
  last_end = gvs_read_unaligned_le(value.data + value.size - offset_size, offset_size);
  if (last_end > value.size) return FALSE;
  offsets_array_size = value.size - last_end;
  if (offsets_array_size % offset_size) return FALSE;
  offsets_array = value.data + value.size - offsets_array_size;
  length = offsets_array_size / offset_size;
  if (length == 0) return FALSE;
  child.type_info = g_variant_type_info_element(value.type_info);
  g_variant_type_info_query(child.type_info, &alignment, NULL);
  offset = 0;
  for (i = 0; i < length; i++) {
      gsize this_end;
      this_end = gvs_read_unaligned_le(offsets_array + offset_size * i, offset_size);
      if (this_end < offset || this_end > last_end) return FALSE;
      while(offset & alignment) {
          if (!(offset < this_end && value.data[offset] == '\0')) return FALSE;
          offset++;
      }
      child.data = value.data + offset;
      child.size = this_end - offset;
      if (child.size == 0) child.data = NULL;
      if (!g_variant_serialised_is_normal(child)) return FALSE;
      offset = this_end;
  }
  g_assert(offset == last_end);
  return TRUE;
}
static gsize gvs_tuple_n_children(GVariantSerialised value) {
  return g_variant_type_info_n_members(value.type_info);
}
static GVariantSerialised gvs_tuple_get_child(GVariantSerialised value, gsize index_) {
  const GVariantMemberInfo *member_info;
  GVariantSerialised child = { 0, };
  gsize offset_size;
  gsize start, end;
  member_info = g_variant_type_info_member_info (value.type_info, index_);
  child.type_info = g_variant_type_info_ref (member_info->type_info);
  offset_size = gvs_get_offset_size (value.size);
  if G_UNLIKELY(value.data == NULL && value.size != 0) {
      g_variant_type_info_query(child.type_info, NULL, &child.size);
      g_assert(child.size != 0);
      child.data = NULL;
      return child;
  }
  if (member_info->ending_type == G_VARIANT_MEMBER_ENDING_OFFSET) {
      if (offset_size * (member_info->i + 2) > value.size) return child;
  } else {
      if (offset_size * (member_info->i + 1) > value.size) {
          g_variant_type_info_query (child.type_info, NULL, &child.size);
          return child;
      }
  }
  if (member_info->i + 1) start = gvs_read_unaligned_le(value.data + value.size - offset_size * (member_info->i + 1), offset_size);
  else start = 0;
  start += member_info->a;
  start &= member_info->b;
  start |= member_info->c;
  if (member_info->ending_type == G_VARIANT_MEMBER_ENDING_LAST) end = value.size - offset_size * (member_info->i + 1);
  else if (member_info->ending_type == G_VARIANT_MEMBER_ENDING_FIXED) {
      gsize fixed_size;
      g_variant_type_info_query (child.type_info, NULL, &fixed_size);
      end = start + fixed_size;
      child.size = fixed_size;
  } else  end = gvs_read_unaligned_le(value.data + value.size - offset_size * (member_info->i + 2), offset_size);
  if (start < end && end <= value.size) {
      child.data = value.data + start;
      child.size = end - start;
  }
  return child;
}
static gsize gvs_tuple_needed_size(GVariantTypeInfo *type_info, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  const GVariantMemberInfo *member_info = NULL;
  gsize fixed_size;
  gsize offset;
  gsize i;
  g_variant_type_info_query(type_info, NULL, &fixed_size);
  if (fixed_size) return fixed_size;
  offset = 0;
  for (i = 0; i < n_children; i++) {
      guint alignment;
      member_info = g_variant_type_info_member_info (type_info, i);
      g_variant_type_info_query(member_info->type_info, &alignment, &fixed_size);
      offset += (-offset) & alignment;
      if (fixed_size) offset += fixed_size;
      else {
          GVariantSerialised child = { 0, };
          gvs_filler (&child, children[i]);
          offset += child.size;
      }
  }
  return gvs_calculate_total_size(offset, member_info->i + 1);
}
static void gvs_tuple_serialise(GVariantSerialised value, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  gsize offset_size;
  gsize offset;
  gsize i;
  offset_size = gvs_get_offset_size (value.size);
  offset = 0;
  for (i = 0; i < n_children; i++) {
      const GVariantMemberInfo *member_info;
      GVariantSerialised child = { 0, };
      guint alignment;
      member_info = g_variant_type_info_member_info(value.type_info, i);
      g_variant_type_info_query(member_info->type_info, &alignment, NULL);
      while(offset & alignment) value.data[offset++] = '\0';
      child.data = value.data + offset;
      gvs_filler (&child, children[i]);
      offset += child.size;
      if (member_info->ending_type == G_VARIANT_MEMBER_ENDING_OFFSET) {
          value.size -= offset_size;
          gvs_write_unaligned_le(value.data + value.size, offset, offset_size);
      }
  }
  while (offset < value.size) value.data[offset++] = '\0';
}
static gboolean gvs_tuple_is_normal(GVariantSerialised value) {
  guint offset_size;
  gsize offset_ptr;
  gsize length;
  gsize offset;
  gsize i;
  offset_size = gvs_get_offset_size(value.size);
  length = g_variant_type_info_n_members(value.type_info);
  offset_ptr = value.size;
  offset = 0;
  for (i = 0; i < length; i++) {
      const GVariantMemberInfo *member_info;
      GVariantSerialised child;
      gsize fixed_size;
      guint alignment;
      gsize end;
      member_info = g_variant_type_info_member_info(value.type_info, i);
      child.type_info = member_info->type_info;
      g_variant_type_info_query(child.type_info, &alignment, &fixed_size);
      while(offset & alignment) {
          if (offset > value.size || value.data[offset] != '\0') return FALSE;
          offset++;
      }
      child.data = value.data + offset;
      switch(member_info->ending_type) {
          case G_VARIANT_MEMBER_ENDING_FIXED: end = offset + fixed_size; break;
          case G_VARIANT_MEMBER_ENDING_LAST: end = offset_ptr; break;
          case G_VARIANT_MEMBER_ENDING_OFFSET:
              offset_ptr -= offset_size;
              if (offset_ptr < offset) return FALSE;
              end = gvs_read_unaligned_le(value.data + offset_ptr, offset_size);
              break;
          default: g_assert_not_reached();
      }
      if (end < offset || end > offset_ptr) return FALSE;
      child.size = end - offset;
      if (child.size == 0) child.data = NULL;
      if (!g_variant_serialised_is_normal(child)) return FALSE;
      offset = end;
  }
  {
      gsize fixed_size;
      guint alignment;
      g_variant_type_info_query(value.type_info, &alignment, &fixed_size);
      if (fixed_size) {
          g_assert(fixed_size == value.size);
          g_assert(offset_ptr == value.size);
          if (i == 0) {
              if (value.data[offset++] != '\0') return FALSE;
          } else {
              while (offset & alignment)
                if (value.data[offset++] != '\0') return FALSE;
          }
          g_assert(offset == value.size);
      }
  }
  return offset_ptr == offset;
}
static inline gsize gvs_variant_n_children(GVariantSerialised value) {
  return 1;
}
static inline GVariantSerialised gvs_variant_get_child(GVariantSerialised value, gsize index_) {
  GVariantSerialised child = { 0, };
  if (value.size) {
      for (child.size = value.size - 1; child.size; child.size--) if (value.data[child.size] == '\0') break;
      if (value.data[child.size] == '\0') {
          const gchar *type_string = (gchar*)&value.data[child.size + 1];
          const gchar *limit = (gchar*)&value.data[value.size];
          const gchar *end;
          if (g_variant_type_string_scan(type_string, limit, &end) && end == limit) {
              const GVariantType *type = (GVariantType*)type_string;
              if (g_variant_type_is_definite(type)) {
                  gsize fixed_size;
                  child.type_info = g_variant_type_info_get(type);
                  if (child.size != 0) child.data = value.data;
                  g_variant_type_info_query(child.type_info,NULL, &fixed_size);
                  if (!fixed_size || fixed_size == child.size) return child;
                  g_variant_type_info_unref(child.type_info);
              }
          }
      }
  }
  child.type_info = g_variant_type_info_get(G_VARIANT_TYPE_UNIT);
  child.data = NULL;
  child.size = 1;
  return child;
}
static inline gsize gvs_variant_needed_size(GVariantTypeInfo *type_info, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  GVariantSerialised child = { 0, };
  const gchar *type_string;
  gvs_filler(&child, children[0]);
  type_string = g_variant_type_info_get_type_string(child.type_info);
  return child.size + 1 + strlen(type_string);
}
static inline void gvs_variant_serialise(GVariantSerialised value, GVariantSerialisedFiller gvs_filler, const gpointer *children, gsize n_children) {
  GVariantSerialised child = { 0, };
  const gchar *type_string;
  child.data = value.data;
  gvs_filler(&child, children[0]);
  type_string = g_variant_type_info_get_type_string(child.type_info);
  value.data[child.size] = '\0';
  memcpy(value.data + child.size + 1, type_string, strlen(type_string));
}
static inline gboolean gvs_variant_is_normal(GVariantSerialised value) {
  GVariantSerialised child;
  gboolean normal;
  child = gvs_variant_get_child(value, 0);
  normal = (child.data != NULL || child.size == 0) && g_variant_serialised_is_normal(child);
  g_variant_type_info_unref(child.type_info);
  return normal;
}
#define DISPATCH_FIXED(type_info, before, after) \
  { \
      gsize fixed_size; \
      g_variant_type_info_query_element(type_info, NULL, &fixed_size); \
      if (fixed_size > 0) { before ## fixed_sized ## after } \
      else { before ## variable_sized ## after } \
  }
#define DISPATCH_CASES(type_info, before, after)                                                               \
  switch(g_variant_type_info_get_type_char(type_info)) {                                                       \
      case G_VARIANT_TYPE_INFO_CHAR_MAYBE: DISPATCH_FIXED(type_info, before, _maybe ## after)                  \
      case G_VARIANT_TYPE_INFO_CHAR_ARRAY: DISPATCH_FIXED(type_info, before, _array ## after)                \
      case G_VARIANT_TYPE_INFO_CHAR_DICT_ENTRY: case G_VARIANT_TYPE_INFO_CHAR_TUPLE: { before ## tuple ## after } \
      case G_VARIANT_TYPE_INFO_CHAR_VARIANT:  { before ## variant ## after }                                      \
  }
gsize g_variant_serialised_n_children(GVariantSerialised serialised) {
  g_variant_serialised_check(serialised);
  DISPATCH_CASES(serialised.type_info,return gvs_,_n_children(serialised);)
  g_assert_not_reached();
}
GVariantSerialised g_variant_serialised_get_child(GVariantSerialised serialised, gsize index_) {
  GVariantSerialised child;
  g_variant_serialised_check(serialised);
  if G_LIKELY(index_ < g_variant_serialised_n_children(serialised)) {
      DISPATCH_CASES(serialised.type_info,child = gvs_,_get_child(serialised, index_);
                     g_assert(child.size || child.data == NULL);
                     g_variant_serialised_check(child);
                     return child;)
      g_assert_not_reached ();
  }
  g_error("Attempt to access item %"G_GSIZE_FORMAT" in a container with only %"G_GSIZE_FORMAT" items", index_, g_variant_serialised_n_children (serialised));
}
void g_variant_serialiser_serialise(GVariantSerialised serialised, GVariantSerialisedFiller gvs_filler, gconstpointer *children, gsize n_children) {
  g_variant_serialised_check(serialised);
  DISPATCH_CASES(serialised.type_info, gvs_/**/,/**/_serialise(serialised, gvs_filler, children, n_children);
                  return;)
  g_assert_not_reached();
}
gsize g_variant_serialiser_needed_size(GVariantTypeInfo *type_info, GVariantSerialisedFiller gvs_filler, gconstpointer *children, gsize n_children) {
  DISPATCH_CASES(type_info,return gvs_/**/,/**/_needed_size (type_info, gvs_filler, children, n_children);)
  g_assert_not_reached();
}
void g_variant_serialised_byteswap(GVariantSerialised serialised) {
  gsize fixed_size;
  guint alignment;
  g_variant_serialised_check(serialised);
  if (!serialised.data) return;
  g_variant_type_info_query(serialised.type_info, &alignment, &fixed_size);
  if (!alignment) return;
  if (alignment + 1 == fixed_size) {
      switch(fixed_size) {
          case 2: {
                  guint16 *ptr = (guint16*)serialised.data;
                  g_assert_cmpint(serialised.size, ==, 2);
                  *ptr = GUINT16_SWAP_LE_BE(*ptr);
              }
              return;
          case 4:{
                  guint32 *ptr = (guint32*)serialised.data;
                  g_assert_cmpint(serialised.size, ==, 4);
                  *ptr = GUINT32_SWAP_LE_BE(*ptr);
              }
              return;
          case 8: {
                  guint64 *ptr = (guint64*)serialised.data;
                  g_assert_cmpint(serialised.size, ==, 8);
                  *ptr = GUINT64_SWAP_LE_BE(*ptr);
              }
              return;
          default: g_assert_not_reached();
      }
  } else {
      gsize children, i;
      children = g_variant_serialised_n_children(serialised);
      for (i = 0; i < children; i++) {
          GVariantSerialised child;
          child = g_variant_serialised_get_child(serialised, i);
          g_variant_serialised_byteswap(child);
          g_variant_type_info_unref(child.type_info);
      }
  }
}
gboolean g_variant_serialised_is_normal(GVariantSerialised serialised) {
  DISPATCH_CASES(serialised.type_info,return gvs_/**/,/**/_is_normal(serialised);)
  if (serialised.data == NULL) return FALSE;
  switch(g_variant_type_info_get_type_char(serialised.type_info)) {
      case 'b': return serialised.data[0] < 2;
      case 's': return g_variant_serialiser_is_string(serialised.data, serialised.size);
      case 'o': return g_variant_serialiser_is_object_path(serialised.data, serialised.size);
      case 'g': return g_variant_serialiser_is_signature(serialised.data, serialised.size);
      default: return TRUE;
  }
}
gboolean g_variant_serialiser_is_string(gconstpointer data, gsize size) {
  const gchar *end;
  g_utf8_validate(data, size, &end);
  return data == end - (size - 1);
}
gboolean g_variant_serialiser_is_object_path(gconstpointer data, gsize size) {
  const gchar *string = data;
  gsize i;
  if (!g_variant_serialiser_is_string(data, size)) return FALSE;
  if (string[0] != '/') return FALSE;
  for (i = 1; string[i]; i++)
      if (g_ascii_isalnum(string[i]) || string[i] == '_');
      else if (string[i] == '/') {
          if (string[i - 1] == '/') return FALSE;
      } else return FALSE;
  if (i > 1 && string[i - 1] == '/') return FALSE;
  return TRUE;
}
gboolean g_variant_serialiser_is_signature(gconstpointer data, gsize size) {
  const gchar *string = data;
  gsize first_invalid;
  if (!g_variant_serialiser_is_string(data, size)) return FALSE;
  first_invalid = strspn(string, "ybnqiuxthdvasog(){}");
  if (string[first_invalid]) return FALSE;
  while(*string) if (!g_variant_type_string_scan(string, NULL, &string)) return FALSE;
  return TRUE;
}
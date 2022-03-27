#include "gvarianttypeinfo.h"
#include "gtestutils.h"
#include "gthread.h"
#include "ghash.h"

struct _GVariantTypeInfo {
  gsize fixed_size;
  guchar alignment;
  guchar container_class;
};
typedef struct {
  GVariantTypeInfo info;
  gchar *type_string;
  gint ref_count;
} ContainerInfo;
typedef struct {
  ContainerInfo container;
  GVariantTypeInfo *element;
} ArrayInfo;
typedef struct {
  ContainerInfo container;
  GVariantMemberInfo *members;
  gsize n_members;
} TupleInfo;
static const GVariantTypeInfo g_variant_type_info_basic_table[24] = {
  #define fixed_aligned(x)  x, x - 1
  #define not_a_type  0,
  #define unaligned  0, 0
  #define aligned(x)  0, x - 1
  { fixed_aligned(1) },
  { not_a_type },
  { fixed_aligned(8) },
  { not_a_type },
  { not_a_type },
  { unaligned        },
  { fixed_aligned(4) },
  { fixed_aligned(4) },
  { not_a_type },
  { not_a_type },
  { not_a_type },
  { not_a_type },
  { fixed_aligned(2) },
  { unaligned        },
  { not_a_type },
  { fixed_aligned(2) },
  { not_a_type },
  { unaligned        },
  { fixed_aligned(8) },
  { fixed_aligned(4) },
  { aligned(8)       },
  { not_a_type },
  { fixed_aligned(8) },
  { fixed_aligned(1) },
  #undef fixed_aligned
  #undef not_a_type
  #undef unaligned
  #undef aligned
};
static const char g_variant_type_info_basic_chars[24][2] = {
  "b", " ", "d", " ", " ", "g", "h", "i", " ", " ", " ", " ", "n", "o", " ", "q", " ", "s", "t", "u", "v", " ", "x", "y"
};
static void g_variant_type_info_check(const GVariantTypeInfo *info, char container_class) {
  g_assert(!container_class || info->container_class == container_class);
  g_assert(info->alignment == 0 || info->alignment == 1 || info->alignment == 3 || info->alignment == 7);
  if (info->container_class) {
      ContainerInfo *container = (ContainerInfo*)info;
      g_assert_cmpint(container->ref_count, >, 0);
      g_assert(container->type_string != NULL);
  } else {
      gint index;
      index = info - g_variant_type_info_basic_table;
      g_assert(G_N_ELEMENTS(g_variant_type_info_basic_table) == 24);
      g_assert(G_N_ELEMENTS(g_variant_type_info_basic_chars) == 24);
      g_assert(0 <= index && index < 24);
      g_assert(g_variant_type_info_basic_chars[index][0] != ' ');
  }
}
const gchar *g_variant_type_info_get_type_string(GVariantTypeInfo *info) {
  g_variant_type_info_check (info, 0);
  if (info->container_class) {
      ContainerInfo *container = (ContainerInfo*)info;
      return container->type_string;
  } else {
      gint index;
      index = info - g_variant_type_info_basic_table;
      return g_variant_type_info_basic_chars[index];
  }
}
void g_variant_type_info_query(GVariantTypeInfo *info, guint *alignment, gsize *fixed_size) {
  g_variant_type_info_check (info, 0);
  if (alignment) *alignment = info->alignment;
  if (fixed_size) *fixed_size = info->fixed_size;
}
#define ARRAY_INFO_CLASS 'a'
static ArrayInfo *ARRAY_INFO(GVariantTypeInfo *info) {
  g_variant_type_info_check(info, ARRAY_INFO_CLASS);
  return (ArrayInfo*)info;
}
static void array_info_free(GVariantTypeInfo *info) {
  ArrayInfo *array_info;
  g_assert(info->container_class == ARRAY_INFO_CLASS);
  array_info = (ArrayInfo*)info;
  g_variant_type_info_unref(array_info->element);
  g_slice_free(ArrayInfo, array_info);
}
static ContainerInfo *array_info_new(const GVariantType *type) {
  ArrayInfo *info;
  info = g_slice_new(ArrayInfo);
  info->container.info.container_class = ARRAY_INFO_CLASS;
  info->element = g_variant_type_info_get(g_variant_type_element (type));
  info->container.info.alignment = info->element->alignment;
  info->container.info.fixed_size = 0;
  return (ContainerInfo *) info;
}
GVariantTypeInfo *g_variant_type_info_element(GVariantTypeInfo *info) {
  return ARRAY_INFO (info)->element;
}
void g_variant_type_info_query_element(GVariantTypeInfo *info, guint *alignment, gsize *fixed_size) {
  g_variant_type_info_query(ARRAY_INFO(info)->element, alignment, fixed_size);
}
#define TUPLE_INFO_CLASS 'r'
static TupleInfo *TUPLE_INFO(GVariantTypeInfo *info) {
  g_variant_type_info_check(info, TUPLE_INFO_CLASS);
  return (TupleInfo*)info;
}
static void tuple_info_free(GVariantTypeInfo *info) {
  TupleInfo *tuple_info;
  gint i;
  g_assert(info->container_class == TUPLE_INFO_CLASS);
  tuple_info = (TupleInfo*)info;
  for (i = 0; i < tuple_info->n_members; i++) g_variant_type_info_unref(tuple_info->members[i].type_info);
  g_slice_free1(sizeof(GVariantMemberInfo) * tuple_info->n_members, tuple_info->members);
  g_slice_free(TupleInfo, tuple_info);
}
static void tuple_allocate_members(const GVariantType *type, GVariantMemberInfo **members, gsize *n_members) {
  const GVariantType *item_type;
  gsize i = 0;
  *n_members = g_variant_type_n_items(type);
  *members = g_slice_alloc(sizeof(GVariantMemberInfo) * *n_members);
  item_type = g_variant_type_first (type);
  while(item_type) {
      GVariantMemberInfo *member = &(*members)[i++];
      member->type_info = g_variant_type_info_get(item_type);
      item_type = g_variant_type_next(item_type);
      if (member->type_info->fixed_size) member->ending_type = G_VARIANT_MEMBER_ENDING_FIXED;
      else if (item_type == NULL) member->ending_type = G_VARIANT_MEMBER_ENDING_LAST;
      else member->ending_type = G_VARIANT_MEMBER_ENDING_OFFSET;
  }
  g_assert(i == *n_members);
}
static gboolean tuple_get_item(TupleInfo *info, GVariantMemberInfo *item, gsize *d, gsize *e) {
  if (&info->members[info->n_members] == item) return FALSE;
  *d = item->type_info->alignment;
  *e = item->type_info->fixed_size;
  return TRUE;
}
static void tuple_table_append (GVariantMemberInfo **items, gsize i, gsize a, gsize b, gsize c) {
  GVariantMemberInfo *item = (*items)++;
  a += ~b & c;
  c &= b;
  item->i = i;
  item->a = a + b;
  item->b = ~b;
  item->c = c;
}
static gsize tuple_align(gsize offset, guint alignment) {
  return offset + ((-offset)&alignment);
}
static void tuple_generate_table(TupleInfo *info) {
  GVariantMemberInfo *items = info->members;
  gsize i = -1, a = 0, b = 0, c = 0, d, e;
  while(tuple_get_item(info, items, &d, &e)) {
      if (d <= b) c = tuple_align(c, d);
      else a += tuple_align(c, b), b = d, c = 0;
      tuple_table_append(&items, i, a, b, c);
      if (e == 0) i++, a = b = c = 0;
      else c += e;
  }
}
static void tuple_set_base_info(TupleInfo *info) {
  GVariantTypeInfo *base = &info->container.info;
  if (info->n_members > 0) {
      GVariantMemberInfo *m;
      base->alignment = 0;
      for (m = info->members; m < &info->members[info->n_members]; m++) base->alignment |= m->type_info->alignment;
      m--;
      if (m->i == -1 && m->type_info->fixed_size) base->fixed_size = tuple_align(((m->a & m->b) | m->c) + m->type_info->fixed_size, base->alignment);
      else base->fixed_size = 0;
  } else {
      base->alignment = 0;
      base->fixed_size = 1;
  }
}
static ContainerInfo *tuple_info_new(const GVariantType *type) {
  TupleInfo *info;
  info = g_slice_new(TupleInfo);
  info->container.info.container_class = TUPLE_INFO_CLASS;
  tuple_allocate_members(type, &info->members, &info->n_members);
  tuple_generate_table(info);
  tuple_set_base_info(info);
  return (ContainerInfo*)info;
}
gsize g_variant_type_info_n_members(GVariantTypeInfo *info) {
  return TUPLE_INFO(info)->n_members;
}
const GVariantMemberInfo *g_variant_type_info_member_info(GVariantTypeInfo *info, gsize index) {
  TupleInfo *tuple_info = TUPLE_INFO(info);
  if (index < tuple_info->n_members) return &tuple_info->members[index];
  return NULL;
}
static GStaticRecMutex g_variant_type_info_lock = G_STATIC_REC_MUTEX_INIT;
static GHashTable *g_variant_type_info_table;
GVariantTypeInfo *g_variant_type_info_get(const GVariantType *type) {
  char type_char;
  type_char = g_variant_type_peek_string (type)[0];
  if (type_char == G_VARIANT_TYPE_INFO_CHAR_MAYBE || type_char == G_VARIANT_TYPE_INFO_CHAR_ARRAY || type_char == G_VARIANT_TYPE_INFO_CHAR_TUPLE ||
      type_char == G_VARIANT_TYPE_INFO_CHAR_DICT_ENTRY) {
      GVariantTypeInfo *info;
      gchar *type_string;
      type_string = g_variant_type_dup_string(type);
      g_static_rec_mutex_lock(&g_variant_type_info_lock);
      if (g_variant_type_info_table == NULL) g_variant_type_info_table = g_hash_table_new(g_str_hash, g_str_equal);
      info = g_hash_table_lookup(g_variant_type_info_table, type_string);
      if (info == NULL) {
          ContainerInfo *container;
          if (type_char == G_VARIANT_TYPE_INFO_CHAR_MAYBE || type_char == G_VARIANT_TYPE_INFO_CHAR_ARRAY) container = array_info_new(type);
          else  container = tuple_info_new(type);
          info = (GVariantTypeInfo *) container;
          container->type_string = type_string;
          container->ref_count = 1;
          g_hash_table_insert(g_variant_type_info_table, type_string, info);
          type_string = NULL;
      } else g_variant_type_info_ref(info);
      g_static_rec_mutex_unlock(&g_variant_type_info_lock);
      g_variant_type_info_check(info, 0);
      g_free(type_string);
      return info;
  } else {
      const GVariantTypeInfo *info;
      int index;
      index = type_char - 'b';
      g_assert(G_N_ELEMENTS(g_variant_type_info_basic_table) == 24);
      g_assert_cmpint(0, <=, index);
      g_assert_cmpint(index, <, 24);
      info = g_variant_type_info_basic_table + index;
      g_variant_type_info_check(info, 0);
      return (GVariantTypeInfo*)info;
    }
}
GVariantTypeInfo *g_variant_type_info_ref(GVariantTypeInfo *info) {
  g_variant_type_info_check (info, 0);
  if (info->container_class) {
      ContainerInfo *container = (ContainerInfo*)info;
      g_assert_cmpint(container->ref_count, >, 0);
      g_atomic_int_inc(&container->ref_count);
  }
  return info;
}
void g_variant_type_info_unref(GVariantTypeInfo *info) {
  g_variant_type_info_check(info, 0);
  if (info->container_class) {
      ContainerInfo *container = (ContainerInfo*)info;
      g_static_rec_mutex_lock(&g_variant_type_info_lock);
      if (g_atomic_int_dec_and_test(&container->ref_count)) {
          g_hash_table_remove(g_variant_type_info_table, container->type_string);
          if (g_hash_table_size(g_variant_type_info_table) == 0) {
              g_hash_table_unref(g_variant_type_info_table);
              g_variant_type_info_table = NULL;
          }
          g_static_rec_mutex_unlock(&g_variant_type_info_lock);
          g_free (container->type_string);
          if (info->container_class == ARRAY_INFO_CLASS) array_info_free(info);
          else if (info->container_class == TUPLE_INFO_CLASS) tuple_info_free(info);
          else g_assert_not_reached();
      } else g_static_rec_mutex_unlock(&g_variant_type_info_lock);
  }
}
void g_variant_type_info_assert_no_infos (void) {
  g_assert(g_variant_type_info_table == NULL);
}
#include <string.h>
#include "../gio.h"
#include "gvdb-reader.h"
#include "gvdb-format.h"

struct _GvdbTable {
  gint ref_count;
  const gchar *data;
  gsize size;
  GMappedFile *mapped;
  gboolean byteswapped;
  gboolean trusted;
  const guint32_le *bloom_words;
  guint32 n_bloom_words;
  guint bloom_shift;
  const guint32_le *hash_buckets;
  guint32 n_buckets;
  struct gvdb_hash_item *hash_items;
  guint32 n_hash_items;
};
static const gchar *gvdb_table_item_get_key(GvdbTable *file, const struct gvdb_hash_item *item, gsize *size) {
  guint32 start, end;
  start = guint32_from_le(item->key_start);
  *size = guint16_from_le(item->key_size);
  end = start + *size;
  if G_UNLIKELY(start > end || end > file->size) return NULL;
  return file->data + start;
}
static gconstpointer gvdb_table_dereference(GvdbTable *file, const struct gvdb_pointer *pointer, gint alignment, gsize *size) {
  guint32 start, end;
  start = guint32_from_le(pointer->start);
  end = guint32_from_le(pointer->end);
  if G_UNLIKELY(start > end || end > file->size || start & (alignment - 1)) return NULL;
  *size = end - start;
  return file->data + start;
}
static void gvdb_table_setup_root(GvdbTable *file, const struct gvdb_pointer *pointer) {
  const struct gvdb_hash_header *header;
  guint32 n_bloom_words;
  guint32 bloom_shift;
  guint32 n_buckets;
  gsize size;
  header = gvdb_table_dereference(file, pointer, 4, &size);
  if G_UNLIKELY(header == NULL || size < sizeof *header) return;
  size -= sizeof *header;
  n_bloom_words = guint32_from_le(header->n_bloom_words);
  n_buckets = guint32_from_le(header->n_buckets);
  bloom_shift = n_bloom_words >> 27;
  n_bloom_words &= (1u << 27) - 1;
  if G_UNLIKELY(n_bloom_words * sizeof(guint32_le) > size) return;
  file->bloom_words = (gpointer)(header + 1);
  size -= n_bloom_words * sizeof(guint32_le);
  file->n_bloom_words = n_bloom_words;
  if G_UNLIKELY(n_buckets > G_MAXUINT / sizeof(guint32_le) || n_buckets * sizeof(guint32_le) > size) return;
  file->hash_buckets = file->bloom_words + file->n_bloom_words;
  size -= n_buckets * sizeof(guint32_le);
  file->n_buckets = n_buckets;
  if G_UNLIKELY(size % sizeof(struct gvdb_hash_item)) return;
  file->hash_items = (gpointer)(file->hash_buckets + n_buckets);
  file->n_hash_items = size / sizeof(struct gvdb_hash_item);
}
GvdbTable *gvdb_table_new(const gchar *filename, gboolean trusted, GError **error) {
  GMappedFile *mapped;
  GvdbTable *file;
  if ((mapped = g_mapped_file_new(filename, FALSE, error)) == NULL) return NULL;
  file = g_slice_new0(GvdbTable);
  file->data = g_mapped_file_get_contents(mapped);
  file->size = g_mapped_file_get_length(mapped);
  file->trusted = trusted;
  file->mapped = mapped;
  file->ref_count = 1;
  if (sizeof(struct gvdb_header) <= file->size) {
      const struct gvdb_header *header = (gpointer)file->data;
      if (header->signature[0] == GVDB_SIGNATURE0 && header->signature[1] == GVDB_SIGNATURE1 && guint32_from_le (header->version) == 0) file->byteswapped = FALSE;
      else if (header->signature[0] == GVDB_SWAPPED_SIGNATURE0 && header->signature[1] == GVDB_SWAPPED_SIGNATURE1 && guint32_from_le(header->version) == 0) {
        file->byteswapped = TRUE;
      } else {
          g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "%s: invalid header", filename);
          g_slice_free(GvdbTable, file);
          g_mapped_file_unref(mapped);
          return NULL;
      }
      gvdb_table_setup_root(file, &header->root);
  }
  return file;
}
static gboolean gvdb_table_bloom_filter (GvdbTable *file, guint32 hash_value) {
  guint32 word, mask;
  if (file->n_bloom_words == 0) return TRUE;
  word = (hash_value / 32) % file->n_bloom_words;
  mask = 1 << (hash_value & 31);
  mask |= 1 << ((hash_value >> file->bloom_shift) & 31);
  return (guint32_from_le (file->bloom_words[word]) & mask) == mask;
}
static gboolean gvdb_table_check_name(GvdbTable *file, struct gvdb_hash_item *item, const gchar *key, guint key_length) {
  const gchar *this_key;
  gsize this_size;
  guint32 parent;
  this_key = gvdb_table_item_get_key (file, item, &this_size);
  if G_UNLIKELY (this_key == NULL || this_size > key_length) return FALSE;
  key_length -= this_size;
  if G_UNLIKELY (memcmp (this_key, key + key_length, this_size) != 0) return FALSE;
  parent = guint32_from_le (item->parent);
  if (key_length == 0 && parent == 0xffffffffu) return TRUE;
  if G_LIKELY (parent < file->n_hash_items && this_size > 0) return gvdb_table_check_name (file, &file->hash_items[parent], key, key_length);
  return FALSE;
}
static const struct gvdb_hash_item *gvdb_table_lookup(GvdbTable *file, const gchar *key, gchar type) {
  guint32 hash_value = 5381;
  guint key_length;
  guint32 bucket;
  guint32 lastno;
  guint32 itemno;
  if G_UNLIKELY(file->n_buckets == 0 || file->n_hash_items == 0) return NULL;
  for (key_length = 0; key[key_length]; key_length++) hash_value = (hash_value * 33) + key[key_length];
  if (!gvdb_table_bloom_filter(file, hash_value)) return NULL;
  bucket = hash_value % file->n_buckets;
  itemno = guint32_from_le(file->hash_buckets[bucket]);
  if (bucket == file->n_buckets - 1 || (lastno = guint32_from_le(file->hash_buckets[bucket + 1])) > file->n_hash_items) lastno = file->n_hash_items;
  while G_LIKELY(itemno < lastno) {
      struct gvdb_hash_item *item = &file->hash_items[itemno];
      if (hash_value == guint32_from_le(item->hash_value))
          if G_LIKELY(gvdb_table_check_name(file, item, key, key_length))
              if G_LIKELY(item->type == type) return item;
      itemno++;
  }
  return NULL;
}
static const struct gvdb_hash_item *gvdb_table_get_item(GvdbTable *table, guint32_le item_no) {
  guint32 item_no_native = guint32_from_le(item_no);
  if G_LIKELY(item_no_native < table->n_hash_items) return table->hash_items + item_no_native;
  return NULL;
}
static gboolean gvdb_table_list_from_item(GvdbTable *table, const struct gvdb_hash_item  *item, const guint32_le **list, guint *length) {
  gsize size;
  *list = gvdb_table_dereference(table, &item->value.pointer, 4, &size);
  if G_LIKELY(*list == NULL || size % 4) return FALSE;
  *length = size / 4;
  return TRUE;
}
gchar **gvdb_table_list(GvdbTable *file, const gchar *key) {
  const struct gvdb_hash_item *item;
  const guint32_le *list;
  gchar **strv;
  guint length;
  guint i;
  if ((item = gvdb_table_lookup(file, key, 'L')) == NULL) return NULL;
  if (!gvdb_table_list_from_item(file, item, &list, &length)) return NULL;
  strv = g_new(gchar *, length + 1);
  for (i = 0; i < length; i++) {
      guint32 itemno = guint32_from_le(list[i]);
      if (itemno < file->n_hash_items) {
          const struct gvdb_hash_item *item;
          const gchar *string;
          gsize strsize;
          item = file->hash_items + itemno;
          string = gvdb_table_item_get_key(file, item, &strsize);
          if (string != NULL) strv[i] = g_strndup(string, strsize);
          else strv[i] = g_malloc0(1);
      } else strv[i] = g_malloc0(1);
  }
  strv[i] = NULL;
  return strv;
}
gboolean gvdb_table_has_value(GvdbTable *file, const gchar *key) {
  return gvdb_table_lookup (file, key, 'v') != NULL;
}
static GVariant *gvdb_table_value_from_item(GvdbTable *table, const struct gvdb_hash_item *item) {
  GVariant *variant, *value;
  gconstpointer data;
  gsize size;
  data = gvdb_table_dereference (table, &item->value.pointer, 8, &size);
  if G_UNLIKELY (data == NULL) return NULL;
  variant = g_variant_new_from_data(G_VARIANT_TYPE_VARIANT, data, size, table->trusted, (GDestroyNotify)g_mapped_file_unref, g_mapped_file_ref(table->mapped));
  value = g_variant_get_variant (variant);
  g_variant_unref(variant);
  return value;
}
GVariant *gvdb_table_get_value(GvdbTable *file, const gchar *key) {
  const struct gvdb_hash_item *item;
  GVariant *value;
  if ((item = gvdb_table_lookup (file, key, 'v')) == NULL) return NULL;
  value = gvdb_table_value_from_item (file, item);
  if (value && file->byteswapped) {
      GVariant *tmp;
      tmp = g_variant_byteswap(value);
      g_variant_unref(value);
      value = tmp;
  }
  return value;
}
GVariant *gvdb_table_get_raw_value(GvdbTable *table, const gchar *key) {
  const struct gvdb_hash_item *item;
  if ((item = gvdb_table_lookup(table, key, 'v')) == NULL) return NULL;
  return gvdb_table_value_from_item(table, item);
}
GvdbTable *gvdb_table_get_table(GvdbTable *file, const gchar *key) {
  const struct gvdb_hash_item *item;
  GvdbTable *new;
  item = gvdb_table_lookup(file, key, 'H');
  if (item == NULL) return NULL;
  new = g_slice_new0 (GvdbTable);
  new->mapped = g_mapped_file_ref(file->mapped);
  new->byteswapped = file->byteswapped;
  new->trusted = file->trusted;
  new->data = file->data;
  new->size = file->size;
  new->ref_count = 1;
  gvdb_table_setup_root(new, &item->value.pointer);
  return new;
}
GvdbTable *gvdb_table_ref(GvdbTable *file) {
  g_atomic_int_inc(&file->ref_count);
  return file;
}
void gvdb_table_unref(GvdbTable *file) {
  if (g_atomic_int_dec_and_test(&file->ref_count)) {
      g_mapped_file_unref(file->mapped);
      g_slice_free(GvdbTable, file);
  }
}
gboolean gvdb_table_is_valid(GvdbTable *table) {
  return !!*table->data;
}
void gvdb_table_walk(GvdbTable *table, const gchar *key, GvdbWalkOpenFunc open_func, GvdbWalkValueFunc value_func, GvdbWalkCloseFunc close_func, gpointer user_data) {
  const struct gvdb_hash_item *item;
  const guint32_le *pointers[64];
  const guint32_le *enders[64];
  gsize name_lengths[64];
  gint index = 0;
  item = gvdb_table_lookup(table, key, 'L');
  name_lengths[0] = 0;
  pointers[0] = NULL;
  enders[0] = NULL;
  goto start_here;
  while (index) {
      close_func(name_lengths[index], user_data);
      index--;
      while(pointers[index] < enders[index]) {
          const gchar *name;
          gsize name_len;
          item = gvdb_table_get_item(table, *pointers[index]++);
      start_here:
          if (item != NULL && (name = gvdb_table_item_get_key(table, item, &name_len))) {
              if (item->type == 'L') {
                  if (open_func(name, name_len, user_data)) {
                      guint length = 0;
                      index++;
                      g_assert(index < 64);
                      gvdb_table_list_from_item(table, item, &pointers[index], &length);
                      enders[index] = pointers[index] + length;
                      name_lengths[index] = name_len;
                  }
              } else if (item->type == 'v') {
                  GVariant *value;
                  value = gvdb_table_value_from_item (table, item);
                  if (value != NULL) {
                      if (table->byteswapped) {
                          GVariant *tmp;
                          tmp = g_variant_byteswap(value);
                          g_variant_unref(value);
                          value = tmp;
                      }
                      value_func(name, name_len, value, user_data);
                      g_variant_unref(value);
                  }
              }
          }
      }
  }
}
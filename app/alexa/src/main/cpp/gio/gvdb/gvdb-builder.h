#ifndef __gvdb_builder_h__
#define __gvdb_builder_h__

#include "../../glib/glib.h"
#include "../gio.h"

typedef struct _GvdbItem GvdbItem;
G_GNUC_INTERNAL GHashTable *gvdb_hash_table_new(GHashTable *parent, const gchar *key);
G_GNUC_INTERNAL GvdbItem *gvdb_hash_table_insert(GHashTable *table, const gchar *key);
G_GNUC_INTERNAL void gvdb_hash_table_insert_string(GHashTable *table, const gchar *key, const gchar *value);
G_GNUC_INTERNAL void gvdb_item_set_value(GvdbItem *item, GVariant *value);
G_GNUC_INTERNAL void gvdb_item_set_hash_table(GvdbItem *item, GHashTable *table);
G_GNUC_INTERNAL void gvdb_item_set_parent(GvdbItem *item, GvdbItem *parent);
G_GNUC_INTERNAL gboolean gvdb_table_write_contents(GHashTable *table, const gchar *filename, gboolean byteswap, GError **error);

#endif
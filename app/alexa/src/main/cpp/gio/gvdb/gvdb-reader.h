#ifndef __gvdb_reader_h__
#define __gvdb_reader_h__

#include "../../glib/glib.h"

typedef struct _GvdbTable GvdbTable;
G_BEGIN_DECLS
G_GNUC_INTERNAL
GvdbTable *gvdb_table_new(const gchar *filename, gboolean trusted, GError **error);
G_GNUC_INTERNAL GvdbTable *gvdb_table_ref(GvdbTable *table);
G_GNUC_INTERNAL void gvdb_table_unref(GvdbTable *table);
G_GNUC_INTERNAL gchar **gvdb_table_list(GvdbTable *table, const gchar *key);
G_GNUC_INTERNAL GvdbTable *gvdb_table_get_table(GvdbTable *table, const gchar  *key);
G_GNUC_INTERNAL GVariant *gvdb_table_get_raw_value(GvdbTable *table, const gchar *key);
G_GNUC_INTERNAL
GVariant *gvdb_table_get_value(GvdbTable *table, const gchar *key);
G_GNUC_INTERNAL
gboolean gvdb_table_has_value(GvdbTable *table, const gchar *key);
G_GNUC_INTERNAL gboolean gvdb_table_is_valid(GvdbTable *table);
typedef void (*GvdbWalkValueFunc)(const gchar *name, gsize name_len, GVariant *value, gpointer user_data);
typedef gboolean (*GvdbWalkOpenFunc)(const gchar *name, gsize name_len, gpointer user_data);
typedef void (*GvdbWalkCloseFunc)(gsize name_len, gpointer user_data);
G_GNUC_INTERNAL void gvdb_table_walk(GvdbTable *table, const gchar *key, GvdbWalkOpenFunc open_func, GvdbWalkValueFunc value_func, GvdbWalkCloseFunc close_func,
                                     gpointer user_data);
G_END_DECLS

#endif

#include "../../glib/glib.h"
#include "../../glib/gprintf.h"
#include "../config.h"
#include "fen-node.h"
#include "fen-dump.h"

G_LOCK_EXTERN (fen_lock);
char* (*file_obj_t)(node_t* node);
static void dump_node (node_t* node, gpointer data) {
    g_printf("n:0x%p ds:0x%p s:0x%p %s\n", node, node->dir_subs, node->subs, (*file_obj_t)(node));
}
static void dump_tree(node_t* node) {
    if (G_TRYLOCK(fen_lock)) {
        node_traverse(NULL, dump_node, NULL);
        G_UNLOCK(fen_lock);
    }
}
void dump_hash_cb(gpointer key, gpointer value, gpointer user_data) {
    g_printf("k:0x%p v:0x%p >\n", key, value);
}
gboolean dump_hash(GHashTable* hash, gpointer user_data) {
    if (G_TRYLOCK(fen_lock)) {
        if (g_hash_table_size(hash) > 0) g_hash_table_foreach(hash, dump_hash_cb, user_data);
        G_UNLOCK(fen_lock);
    }
    return TRUE;
}
void dump_event(node_event_t* ev, gpointer user_data) {
    node_t* node = ev->user_data;
    g_printf("ne:0x%p e:%p n:0x%p ds:0x%p s:0x%p s\n", ev, ev->e, node, node->dir_subs, node->subs, (*file_obj_t)(node));
}
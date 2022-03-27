#include <gio/config.h>
#include <sys/stat.h>
#include <errno.h>
#include <strings.h>
#include <glib/glib.h>
#include "fen-kernel.h"
#include "fen-node.h"
#include "fen-dump.h"
#ifndef GIO_COMPILATION
#include <gio/gfilemonitor.h>
#include <sys/un.h>
#include <glib/gi18n.h>
#else
#include "gam_event.h"
#include "gam_server.h"
#include "gam_protocol.h"
#endif
#ifndef GIO_COMPILATION
#define FN_W if (FALSE) g_debug
#else
#include "gam_error.h"
#define FN_W(...) GAM_DEBUG(DEBUG_INFO, __VA_ARGS__)
#endif

G_LOCK_EXTERN (fen_lock);
#define NODE_NEED_MONITOR(f)  (NODE_IS_ACTIVE(f) || node_children_num(f) > 0 || NODE_IS_REQUIRED_BY_PARENT(f))
static int concern_events[] = {
    /*FILE_DELETE,
    FILE_RENAME_FROM,
    UNMOUNTED,
    MOUNTEDOVER,
#ifdef GIO_COMPILATION
    FILE_MODIFIED,
    FILE_ATTRIB,
#else
    FILE_MODIFIED | FILE_ATTRIB,
#endif
    FILE_RENAME_TO*/
};
node_t *ROOT = NULL;
static void node_emit_one_event(node_t *f, GList *subs, node_t *other, int event);
static void node_emit_events(node_t *f, const node_event_t *ne);
static int node_event_translate(int event, gboolean pair);
static void node_add_event(node_t *f, node_event_t *ev);
static node_t* node_new(node_t* parent, const gchar* basename);
static void node_delete(node_t* parent);
static node_t* node_get_child(node_t *f, const gchar *basename);
static void children_add(node_t *p, node_t *f);
static void children_remove(node_t *p, node_t *f);
static gboolean children_remove_cb(gpointer key, gpointer value, gpointer user_data);
static guint node_children_num(node_t *f);
gboolean node_timeval_lt(const GTimeVal *val1, const GTimeVal *val2) {
    if (val1->tv_sec < val2->tv_sec) return TRUE;
    if (val1->tv_sec > val2->tv_sec) return FALSE;
    if (val1->tv_usec < val2->tv_usec) return TRUE;
    return FALSE;
}
void node_traverse(node_t* node, void(*traverse_cb)(node_t*, gpointer), gpointer user_data) {
    GHashTableIter iter;
    gpointer value;
    g_assert(traverse_cb);
    if (node == NULL) node = ROOT;
    if (node) traverse_cb(node, user_data);
    g_hash_table_iter_init(&iter, node->children);
    while(g_hash_table_iter_next (&iter, NULL, &value)) {
        node_traverse((node_t *)value, traverse_cb, user_data);
    }
}
node_t* node_find(node_t* node, const gchar* filename, gboolean create_on_missing) {
    gchar* str;
    gchar* token;
    gchar* lasts;
    node_t* parent;
    node_t* child;
    g_assert(filename && filename[0] == '/');
    if (node == NULL) {
        node = ROOT;
    }
    FN_W ("%s %s\n", __func__, filename);
    parent = child = node;
    str = g_strdup(filename);
    for (token = strtok_r(str, G_DIR_SEPARATOR_S, &lasts);
         token != NULL && child != NULL;
         token = strtok_r(NULL, G_DIR_SEPARATOR_S, &lasts)) {
        child = node_get_child(parent, token);
        if (child) { parent = child; }
        else if (create_on_missing) {
            child = node_new(parent, token);
            if (child) {
                children_add(parent, child);
                parent = child;
                continue;
            } else { FN_W("%s create %s failed", __func__, token); }
        } else break;
    }
    g_free(str);
    return child;
}
gint node_lstat(node_t *f) {
    struct stat buf;
    g_assert(!NODE_HAS_STATE(f, NODE_STATE_ASSOCIATED));
    /*if (lstat(NODE_NAME(f), &buf) == 0) {
        FN_W ("%s %s\n", __func__, NODE_NAME(f));
        FILE_OBJECT(f)->fo_atime = buf.st_atim;
        FILE_OBJECT(f)->fo_mtime = buf.st_mtim;
        FILE_OBJECT(f)->fo_ctime = buf.st_ctim;
        NODE_SET_FLAG(f, NODE_FLAG_STAT_UPDATED | (S_ISDIR(buf.st_mode) ? NODE_FLAG_DIR : NODE_FLAG_NONE));
        return 0;
    } else {
        FN_W("%s(lstat) %s %s\n", __func__, NODE_NAME(f), g_strerror(errno));
    }*/
    return errno;
}
void node_create_children_snapshot(node_t *f, gint created_event, gboolean emit) {
	GDir *dir = NULL;
	GError *err = NULL;
    //FN_W("%s %s [0x%p]\n", __func__, NODE_NAME(f), f);
    //dir = g_dir_open(NODE_NAME(f), 0, &err);
    if (dir) {
        const char *basename;
        node_t *child = NULL;
        while((basename = g_dir_read_name(dir))) {
            node_t* data;
            GList *idx;
            child = node_get_child(f, basename);
            if (child == NULL) {
                gchar *filename;
                child = node_new(f, basename);
                children_add(f, child);
            }
            if (f->dir_subs) {
                if (!NODE_HAS_STATE(child, NODE_STATE_ASSOCIATED) &&
                  node_lstat(child) == 0 && port_add(child) == 0) {
                    if (emit) {
                        node_emit_one_event(child, child->dir_subs, NULL, created_event);
                        node_emit_one_event(child, child->subs, NULL, created_event);
                        node_emit_one_event(child, f->dir_subs, NULL, created_event);
                        node_emit_one_event(child, f->subs, NULL, created_event);
                    }
                }
            }
        }
        g_dir_close(dir);
        NODE_SET_FLAG(f, NODE_FLAG_SNAPSHOT_UPDATED);
    } else {
        FN_W(err->message);
        g_error_free(err);
    }
}
static void foreach_known_children_scan(gpointer key, gpointer value, gpointer user_data) {
    /*node_t* f = (node_t*)value;
    FN_W ("%s 0x%p %s\n", __func__, f, NODE_NAME(f));
    if (!NODE_HAS_STATE(f, NODE_STATE_ASSOCIATED)) {
        if (node_lstat(f) == 0 && port_add(f) == 0) {
            node_emit_one_event(f, f->dir_subs, NULL, FN_EVENT_CREATED);
            node_emit_one_event(f, f->subs, NULL, FN_EVENT_CREATED);
            if (NODE_PARENT(f)) {
                node_emit_one_event(f, NODE_PARENT(f)->dir_subs, NULL, FN_EVENT_CREATED);
                node_emit_one_event(f, NODE_PARENT(f)->subs, NULL, FN_EVENT_CREATED);
            }
        }
    }*/
}
gboolean node_try_delete(node_t* node) {
    g_assert(node);
    //FN_W("%s 0x%p %s\n", __func__, node, NODE_NAME(node));
    if (node_children_num (node) > 0) g_hash_table_foreach_remove(node->children, children_remove_cb, NULL);
    if (!NODE_NEED_MONITOR(node)) {
        if (NODE_HAS_STATE(node, NODE_STATE_ASSOCIATED)) port_remove(node);
        if (node->state == 0 && NODE_PARENT(node)) {
            children_remove(NODE_PARENT(node), node);
            node_delete(node);
        }
    }
    return FALSE;
}
static node_t* node_new(node_t* parent, const gchar* basename) {
	node_t *f = NULL;
    /*g_assert(basename && basename[0]);
    if ((f = g_new0(node_t, 1)) != NULL) {
        if (parent) { NODE_NAME(f) = g_build_filename(NODE_NAME(parent), basename, NULL); }
        else { NODE_NAME(f) = g_strdup(G_DIR_SEPARATOR_S); }
        f->basename = g_strdup (basename);
        f->children = g_hash_table_new_full (g_str_hash, g_str_equal,NULL, NULL);
    #ifdef GIO_COMPILATION
        f->gfile = g_file_new_for_path(NODE_NAME(f));
    #endif
        FN_W ("%s 0x%p %s\n", __func__, f, NODE_NAME(f));
    }*/
	return f;
}
static void node_delete(node_t *f) {
    //FN_W ("%s 0x%p %s\n", __func__, f, NODE_NAME(f));
    f->flag = 0;
    g_assert(f->state == 0);
    g_assert(!NODE_IS_ACTIVE(f));
    g_assert(g_hash_table_size (f->children) == 0);
    g_assert(NODE_PARENT(f) == NULL);
    g_hash_table_unref(f->children);
#ifdef GIO_COMPILATION
    g_object_unref (f->gfile);
#endif
    g_free(f->basename);
    //g_free(NODE_NAME(f));
    g_free (f);
}
static void children_add(node_t *p, node_t *f) {
    /*FN_W ("%s %s %s\n", __func__, NODE_NAME(p), f->basename);
    g_hash_table_insert (p->children, f->basename, f);
    NODE_PARENT(f) = p;*/
}
static void children_remove(node_t *p, node_t *f) {
    /*FN_W ("%s %s %s\n", __func__, NODE_NAME(p), f->basename);
    g_hash_table_steal (p->children, f->basename);
    NODE_PARENT(f) = NULL;*/
}
static node_t *node_get_child(node_t *f, const gchar *basename) {
    if (f->children) return (node_t *) g_hash_table_lookup (f->children, (gpointer)basename);
    return NULL;
}
static guint node_children_num(node_t *f) {
    return g_hash_table_size (f->children);
}
static gboolean children_remove_cb(gpointer key, gpointer value, gpointer user_data) {
    return node_try_delete ((node_t*)value);
}
gboolean node_class_init() {
    ROOT = node_new (NULL, G_DIR_SEPARATOR_S);
    if (ROOT == NULL) {
        FN_W ("[node] Create ROOT node failed.\n");
        return FALSE;
    }
    return port_class_init (node_add_event);
}
void node_adjust_deleted(node_t* f) {
    node_t *ancestor;
    //FN_W ("%s %s\n", __func__, NODE_NAME(f));
    for (ancestor = NODE_PARENT(f);
         ancestor != NULL;
         ancestor = NODE_PARENT(ancestor)) {
        if (NODE_HAS_STATE(ancestor, NODE_STATE_ASSOCIATED) ||
          (node_lstat(ancestor) == 0 && port_add(ancestor) == 0)) {
            break;
        }
    }
}
static void node_emit_events(node_t *f, const node_event_t *ne) {
    gsize num = sizeof(concern_events)/sizeof(int);
    gint i;
    int translated_e;
    node_t *p;
    if (node_timeval_lt(&f->atv, &ne->ctv)) {
        int event = ne->e;
        if (ne->pair_data) {
            node_t *from = ne->pair_data;
            //node_emit_one_event(from, from->dir_subs, NULL, node_event_translate(FILE_DELETE, FALSE));
            //node_emit_one_event(from, from->subs, NULL, node_event_translate(FILE_DELETE, FALSE));
        }
        for (i = 0; i < num; i++) {
            if (event & concern_events[i]) {
                translated_e = node_event_translate(concern_events[i], FALSE);
            /*#ifdef GIO_COMPILATION
                if ((concern_events[i] & FILE_MODIFIED) == 0) node_emit_one_event(f, f->dir_subs, NULL, translated_e);
            #else
                if ((concern_events[i] & (FILE_MODIFIED | FILE_ATTRIB)) == 0) node_emit_one_event(f, f->dir_subs, NULL, translated_e);
            #endif*/
                node_emit_one_event(f, f->subs, NULL, translated_e);
            }
            event &= ~concern_events[i];
        }
    }
    p = NODE_PARENT(f);
    if (p != NULL && node_timeval_lt(&p->atv, &ne->ctv)) {
        int event = ne->e;
        for (i = 0; i < num; i++) {
            if (event & concern_events[i]) {
                translated_e = node_event_translate(concern_events[i], ne->pair_data != NULL);
                node_emit_one_event(f, p->dir_subs, ne->pair_data, translated_e);
                node_emit_one_event(f, p->subs, ne->pair_data, translated_e);
            }
            event &= ~concern_events[i];
        }
    }
}
static void node_add_event(node_t *f, node_event_t *ev) {
    FN_W("%s %d\n", __func__, ev->e);
    NODE_CLE_STATE(f, NODE_STATE_HAS_EVENTS);
    if (NODE_NEED_MONITOR(f)) {
        /*if (HAS_NO_EXCEPTION_EVENTS(ev->e)) {
            if (NODE_HAS_STATE(f, NODE_STATE_ASSOCIATED) || port_add(f) == 0) {
                if ((ev->e & FILE_MODIFIED) && NODE_HAS_FLAG(f, NODE_FLAG_DIR)) {
                    if (f->dir_subs) node_create_children_snapshot(f, FN_EVENT_CREATED, TRUE);
                    else g_hash_table_foreach(f->children, foreach_known_children_scan, NULL);
                }
            } else {
                ev->e |= FILE_DELETE;
                node_adjust_deleted(f);
            }
        } else node_adjust_deleted(f);*/
        node_emit_events(f, ev);
    } else {
        node_emit_events(f, ev);
        node_try_delete(f);
    }
    if (ev->pair_data) {
        node_t *from = ev->pair_data;
        //g_assert(ev->e == FILE_RENAME_TO);
        if (NODE_NEED_MONITOR(from)) {
            NODE_CLE_STATE(from, NODE_STATE_HAS_EVENTS);
            node_adjust_deleted(from);
        } else node_try_delete(from);
    }
    node_event_delete(ev);
}
static void node_emit_one_event(node_t *f, GList *subs, node_t *other, int event) {
    /*GList* idx;
    FN_W ("%s %s %d\n", __func__, NODE_NAME(f), event);
#ifdef GIO_COMPILATION
    for (idx = subs; idx; idx = idx->next) {
        g_file_monitor_emit_event(G_FILE_MONITOR(idx->data), f->gfile, (other == NULL ? NULL : other->gfile), event);
    }
#else
    for (idx = subs; idx; idx = idx->next) {
        gam_server_emit_one_event(NODE_NAME(f), gam_subscription_is_dir(idx->data), event, idx->data, 1);
    }
#endif*/
}
static int node_event_translate(int event, gboolean pair) {
#ifndef GIO_COMPILATION
    switch(event) {
        /*case FILE_DELETE:
        case FILE_RENAME_FROM:
            return G_FILE_MONITOR_EVENT_DELETED;
        case UNMOUNTED:
            return G_FILE_MONITOR_EVENT_UNMOUNTED;
        case FILE_ATTRIB:
            return G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED;
        case MOUNTEDOVER:
        case FILE_MODIFIED:
            return G_FILE_MONITOR_EVENT_CHANGED;
        case FILE_RENAME_TO:
            if (pair) return G_FILE_MONITOR_EVENT_MOVED;
            else return G_FILE_MONITOR_EVENT_CREATED;*/
        default:
            g_assert_not_reached();
            return -1;
    }
#else
    switch(event) {
        case FILE_DELETE: case FILE_RENAME_FROM: return GAMIN_EVENT_DELETED;
        case MOUNTEDOVER: case UNMOUNTED: return GAMIN_EVENT_CHANGED;
        case FILE_RENAME_TO:
            if (pair) return GAMIN_EVENT_MOVED;
            else return GAMIN_EVENT_CREATED;
        default:
            if (event & (FILE_ATTRIB | FILE_MODIFIED)) return GAMIN_EVENT_CHANGED;
            g_assert_not_reached ();
            return -1;
    }
#endif
}
node_event_t* node_event_new(int event, gpointer user_data) {
    node_event_t *ev;
    if ((ev = g_new(node_event_t, 1)) != NULL) {
        g_assert(ev);
        ev->e = event;
        ev->user_data = user_data;
        ev->pair_data = NULL;
        g_get_current_time(&ev->ctv);
        ev->rename_tv = ev->ctv;
    }
    return ev;
}
void node_event_delete(node_event_t* ev) {
    g_free(ev);
}

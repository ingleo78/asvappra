#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <opencl-c-base.h>
#include "../../glib/glib.h"
#include "../config.h"
#include "fen-kernel.h"
#include "fen-dump.h"

#ifndef GIO_COMPILATION
#define FK_W if (FALSE) g_debug
#else
#include "gam_error.h"
#define FK_W(...) GAM_DEBUG(DEBUG_INFO, __VA_ARGS__)
#endif

G_GNUC_INTERNAL G_LOCK_DEFINE (fen_lock);
static ulong max_port_events = 512;
static GList *pn_visible_list;
static GQueue *g_eventq = NULL;
//static timespec_t zero_wait;
static void (*user_process_events_cb) (gpointer, node_event_t*);
//static port_event_t *pevents = NULL;
static gint PE_ALLOC = 2048;
static GHashTable *renamed_hash = NULL;
typedef struct _PSource {
    GSource  source;
    GPollFD  gfd;
    gboolean pending;
    uint_t   event_growing_factor;
    uint_t   pending_events;
} PSource;
#define PGPFD(s)             (&((PSource *)(s))->gfd)
#define SLEEP_BASE_TIME      10
#define EXPECT_INC_EVENTS(pn)  (1 << (pn->event_growing_factor))
#define RENAME_EVENTS_INTERVAL 500
#define PROCESS_PORT_EVENTS_TIME 1000
guint process_port_event_id = 0;
static gchar* _event_strings(int event);
static const gchar* _event_string (int event);
static GSource *psource_new();
static gboolean port_prepare(GSource *source, gint *timeout_);
static gboolean port_check(GSource *source);
static gboolean port_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);
static GSourceFuncs fen_source_func = {
    port_prepare,
    port_check,
    port_dispatch,
    NULL
};
static gboolean port_prepare(GSource *source, gint *timeout_) {
    return FALSE;
}
static gboolean port_check(GSource *source) {
	PSource *pn = (PSource *)source;
    uint_t nget;
    if (pn->pending) {
        pn->pending = FALSE;
        g_source_add_poll(source, PGPFD(source));
        g_source_unref(source);
        return FALSE;
    }
    if (!(PGPFD(pn)->revents & G_IO_IN))return FALSE;
    /*if (port_getn(PGPFD(source)->fd, NULL, 0, &nget, 0) == 0) {
        if (nget - pn->pending_events > EXPECT_INC_EVENTS(pn)) {
            pn->pending_events = nget;
            pn->event_growing_factor ++;
            pn->pending = TRUE;
            g_source_ref(source);
            g_source_remove_poll(source, PGPFD(source));
            g_timeout_add(SLEEP_BASE_TIME, (GSourceFunc)port_check, (gpointer)pn);
            return FALSE;
        }
    }*/
    pn->pending_events = 0;
    pn->event_growing_factor = 0;
    return TRUE;
}
static gboolean port_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
    node_t *f;
	uint_t nget = 0;
	uint_t total = 0;
    FK_W("%s 0x%p fd %d\n", __func__, source, PGPFD(source)->fd);
    G_LOCK (fen_lock);
    /*do {
        nget = 1;
        if (port_getn(PGPFD(source)->fd, pevents, PE_ALLOC, &nget, &zero_wait) == 0) {
            int i;
            for (i = 0; i < nget; i++) {
                f = (node_t *)pevents[i].portev_user;
                if (pevents[i].portev_source == PORT_SOURCE_FILE) {
                    NODE_CLE_STATE(f, NODE_STATE_ASSOCIATED);
                    NODE_SET_STATE(f, NODE_STATE_HAS_EVENTS);
                    if (HAS_NO_EXCEPTION_EVENTS(pevents[i].portev_events)) {
                        if (node_lstat(f) != 0) pevents[i].portev_events |= FILE_DELETE;
                    }
                    g_queue_push_tail(g_eventq, node_event_new(pevents[i].portev_events, (gpointer)f));
                } else {
                    FK_W("[kernel] unknown portev_source %d\n", pevents[i].portev_source);
                }
            }
            total += nget;
        } else {
            FK_W ("[kernel] port_getn %s\n", g_strerror (errno));
            break;
        }
    } while (nget == PE_ALLOC);*/
    G_UNLOCK (fen_lock);
    if (total > 0 && callback) {
        FK_W ("[kernel] get total %ld events\n", total);
        return callback (user_data);
    }
    return TRUE;
}
static gboolean process_renamed_hash_cb(gpointer key, gpointer value, gpointer user_data) {
    node_event_t *ev = value;
#if 0
    node_add_event(ev->user_data, ev);
#else
    user_process_events_cb(ev->user_data, ev);
#endif
    return TRUE;
}
static gboolean port_events_process_cb(gpointer user_data) {
    node_event_t *ev;
    G_LOCK (fen_lock);
    while ((ev = (node_event_t*)g_queue_pop_head (g_eventq)) != NULL) {
        {
            gchar *log = _event_strings(ev->e);
            //FK_W("%s %s %s\n", __func__, NODE_NAME(ev->user_data), log);
            g_free(log);
        }
    #ifdef GIO_COMPILATION
        if (NODE_PARENT(ev->user_data)) {
            if (ev->e == FILE_RENAME_TO) {
                g_hash_table_insert(renamed_hash, NODE_PARENT(ev->user_data), ev);
                g_time_val_add(&ev->rename_tv, RENAME_EVENTS_INTERVAL);
                continue;
            }
            if (ev->e == FILE_RENAME_FROM) {
                node_event_t *pair_ev;
                pair_ev = g_hash_table_lookup(renamed_hash, NODE_PARENT(ev->user_data));
                if (pair_ev && node_timeval_lt(&ev->ctv, &pair_ev->rename_tv)) {
                    g_hash_table_remove(renamed_hash, NODE_PARENT(ev->user_data));
                    pair_ev->pair_data = ev->user_data;
                    /* Free ev, exchange pair_ev and ev. */
                    node_event_delete(ev);
                    ev = pair_ev;
                }
            }
        }
    #endif
    #if 0
        node_add_event(ev->user_data, ev);
    #else
        user_process_events_cb(ev->user_data, ev);
    #endif
    }
    g_hash_table_foreach_remove(renamed_hash, process_renamed_hash_cb, NULL);
    G_UNLOCK (fen_lock);
    process_port_event_id = 0;
    return FALSE;
}
static gboolean port_events_read_cb(gpointer user_data) {
    if (process_port_event_id == 0) process_port_event_id = g_timeout_add(PROCESS_PORT_EVENTS_TIME, port_events_process_cb,NULL);
	return TRUE;
}
static GSource* psource_new() {
    GSource *source = NULL;
    /*int fd;
    if ((fd = port_create()) >= 0) {
        source = g_source_new(&fen_source_func, sizeof(PSource));
        PGPFD(source)->fd = fd;
        PGPFD(source)->events = G_IO_IN | G_IO_HUP | G_IO_ERR;
        g_source_set_callback(source, port_events_read_cb, NULL, NULL);
        g_source_attach(source, NULL);
        g_source_unref(source);
        g_source_add_poll(source, PGPFD(source));
        FK_W ("%s 0x%p fd %d\n", __func__, source, PGPFD(source)->fd);
    } else {
        FK_W ("PORT_CREATE %s\n", g_strerror(errno));
        g_return_val_if_reached(NULL);
    }*/
	return source;
}
gint port_add(node_t *f) {
	GSource *source = f->source;
    //FK_W ("%s [0x%p] %s\n", __func__, f, NODE_NAME(f));
    g_assert(f);
    g_assert(NODE_HAS_FLAG(f, NODE_FLAG_STAT_UPDATED));
    if (!source) {
    start_over:
        if (pn_visible_list) source = (GSource *)pn_visible_list->data;
        else {
            if ((source = psource_new()) != NULL) {
                g_assert (g_list_find (pn_visible_list, source) == NULL);
                pn_visible_list = g_list_prepend (pn_visible_list, source);
            }
        }
    }
    /*if (port_associate(PGPFD(source)->fd, PORT_SOURCE_FILE, (uintptr_t)FILE_OBJECT(f),
        CONCERNED_EVENTS,
        (void *)f) == 0) {
        f->source = source;
        NODE_SET_STATE(f, NODE_STATE_ASSOCIATED);
        NODE_CLE_FLAG(f, NODE_FLAG_STAT_UPDATED);
        FK_W ("PORT_ASSOCIATE 0x%p OK\n", f);
        return 0;
    } else if (errno == EAGAIN) {
        pn_visible_list = g_list_remove (pn_visible_list, source);
        goto start_over;
    } else if (errno == ENOENT) { }
    else if (errno == ENOTSUP) {
        NODE_CLE_FLAG(f, NODE_FLAG_STAT_UPDATED);
        return 0;
    } else {
        FK_W ("PORT_ASSOCIATE 0x%p %s\n", f, g_strerror (errno));
    }*/
    NODE_CLE_FLAG(f, NODE_FLAG_STAT_UPDATED);
    return errno;
}
void port_remove(node_t *f) {
    /*if (NODE_HAS_STATE(f, NODE_STATE_ASSOCIATED)) {
        if (port_dissociate(PGPFD(f->source)->fd, PORT_SOURCE_FILE, (uintptr_t)FILE_OBJECT(f)) == 0) {
            NODE_CLE_STATE(f, NODE_STATE_ASSOCIATED);
            FK_W ("PORT_DISSOCIATE 0x%p OK\n", f);
        } else if (errno == ENOENT) {}
        else {
            FK_W ("PORT_DISSOCIATE 0x%p %s\n", f, g_strerror (errno));
            g_return_if_reached();
        }
    }*/
}
extern gboolean port_class_init(void (*user_process_events_callback) (gpointer, node_event_t*)) {
	/*rctlblk_t *rblk;
	if ((rblk = malloc (rctlblk_size())) == NULL) {
        FK_W("[kernel] rblk malloc %s\n", g_strerror(errno));
		return FALSE;
	}
	if (getrctl("process.max-port-events", NULL, rblk, RCTL_FIRST) == -1) {
        FK_W("[kernel] getrctl %s\n", g_strerror(errno));
        free(rblk);
        return FALSE;
	} else {
        max_port_events = rctlblk_get_value(rblk);
		FK_W("max_port_events = %u\n", max_port_events);
        free(rblk);
	}
    renamed_hash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
    if (renamed_hash == NULL) {
		FK_W("[kernel] FEN global renamed queue initializing faild\n");
        return FALSE;
    }
    if ((g_eventq = g_queue_new()) == NULL) {
		FK_W("[kernel] FEN global event queue initializing faild\n");
        return FALSE;
    }
    if (user_process_events_callback == NULL) {
		FK_W("[kernel] FEN global no user_process_events_callback\n");
        return FALSE;
    }
    user_process_events_cb = user_process_events_callback;
    memset(&zero_wait, 0, sizeof (timespec_t));
    pevents = g_malloc(PE_ALLOC * sizeof(port_event_t));
    if (pevents == NULL) {
		FK_W("[kernel] FEN global alloc pevents failed\n");
        return FALSE;
    }*/
	return TRUE;
}
static gchar* printevent(const char *pname, int event, const char *tag) {
    static gchar *event_string = NULL;
    /*GString	*str;
    if (event_string) g_free(event_string);
    str = g_string_new ("");
    g_string_printf (str, "[%s] [%-20s]", tag, pname);
    if (event & FILE_ACCESS) str = g_string_append(str, " ACCESS");
    if (event & FILE_MODIFIED) str = g_string_append(str, " MODIFIED");
    if (event & FILE_ATTRIB) str = g_string_append(str, " ATTRIB");
    if (event & FILE_DELETE) str = g_string_append(str, " DELETE");
    if (event & FILE_RENAME_TO) str = g_string_append(str, " RENAME_TO");
    if (event & FILE_RENAME_FROM) str = g_string_append(str, " RENAME_FROM");
    if (event & UNMOUNTED) str = g_string_append(str, " UNMOUNTED");
    if (event & MOUNTEDOVER) str = g_string_append(str, " MOUNTEDOVER");
    event_string = str->str;
    g_string_free(str, FALSE);*/
    return event_string;
}
static gchar *_event_strings(int event) {
    GString *str = g_string_sized_new(80);
    /*if (event & FILE_DELETE) g_string_append(str, " FILE_DELETE");
    if (event & FILE_RENAME_FROM) g_string_append(str, " FILE_RENAME_FROM");
    if (event & FILE_MODIFIED) g_string_append(str, " FILE_MODIFIED");
    if (event & FILE_RENAME_TO) g_string_append(str, " FILE_RENAME_TO");
    if (event & MOUNTEDOVER) g_string_append(str, " MOUNTEDOVER");
    if (event & FILE_ATTRIB) g_string_append(str, " FILE_ATTRIB");
    if (event & UNMOUNTED) g_string_append(str, " UNMOUNTED");
    if (event & FILE_ACCESS) g_string_append(str, " FILE_ACCESS");*/
    return g_string_free(str, FALSE);
}
static const gchar *_event_string(int event) {
    switch(event) {
        /*case FILE_DELETE: return "FILE_DELETE";
        case FILE_RENAME_FROM: return "FILE_RENAME_FROM";
        case FILE_MODIFIED: return "FILE_MODIFIED";
        case FILE_RENAME_TO: return "FILE_RENAME_TO";
        case MOUNTEDOVER: return "MOUNTEDOVER";
        case FILE_ATTRIB: return "FILE_ATTRIB";
        case UNMOUNTED: return "UNMOUNTED";
        case FILE_ACCESS: return "FILE_ACCESS";*/
        default: return "EVENT_UNKNOWN";
    }
}
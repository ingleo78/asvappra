#if defined (__GLIB_GOBJECT_H_INSIDE__) && defined (GOBJECT_COMPILATION)
//#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_CLOSURE_H__
#define __G_CLOSURE_H__

#include "gtype.h"

G_BEGIN_DECLS
#define	G_CLOSURE_NEEDS_MARSHAL(closure) (((GClosure*)(closure))->marshal == NULL)
#define	G_CLOSURE_N_NOTIFIERS(cl)  ((cl)->meta_marshal + ((cl)->n_guards << 1L) + (cl)->n_fnotifiers + (cl)->n_inotifiers)
#define	G_CCLOSURE_SWAP_DATA(cclosure)	 (((GClosure*)(cclosure))->derivative_flag)
#define	G_CALLBACK(f)  ((GCallback)(f))
typedef struct _GClosure GClosure;
typedef struct _GClosureNotifyData GClosureNotifyData;
typedef void  (*GCallback)(void);
typedef void  (*GClosureNotify)(gpointer data, GClosure	*closure);
typedef void  (*GClosureMarshal)(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values, gpointer invocation_hint,
					 			 gpointer marshal_data);
typedef struct _GCClosure GCClosure;
struct _GClosureNotifyData {
    gpointer data;
    GClosureNotify notify;
};
struct _GClosure {
    volatile guint ref_count : 15;
    volatile guint meta_marshal : 1;
    volatile guint n_guards : 1;
    volatile guint n_fnotifiers : 2;
    volatile guint n_inotifiers : 8;
    volatile guint in_inotify : 1;
    volatile guint floating : 1;
    volatile guint derivative_flag : 1;
    volatile guint in_marshal : 1;
    volatile guint is_invalid : 1;
    void (*marshal)(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values, gpointer invocation_hint, gpointer marshal_data);
    gpointer data;
    GClosureNotifyData *notifiers;
};
struct _GCClosure {
    GClosure closure;
    gpointer callback;
};
GClosure* g_cclosure_new(GCallback callback_func, gpointer	user_data, GClosureNotify destroy_data);
GClosure* g_cclosure_new_swap(GCallback	callback_func, gpointer user_data, GClosureNotify destroy_data);
GClosure* g_signal_type_cclosure_new(GType itype, guint struct_offset);
GClosure* g_closure_ref(GClosure *closure);
void g_closure_sink(GClosure *closure);
void g_closure_unref(GClosure *closure);
GClosure* g_closure_new_simple(guint sizeof_closure, gpointer data);
void g_closure_add_finalize_notifier(GClosure *closure, gpointer notify_data, GClosureNotify notify_func);
void g_closure_remove_finalize_notifier(GClosure *closure, gpointer notify_data, GClosureNotify notify_func);
void g_closure_add_invalidate_notifier(GClosure *closure, gpointer notify_data, GClosureNotify notify_func);
void g_closure_remove_invalidate_notifier(GClosure *closure, gpointer notify_data, GClosureNotify notify_func);
void g_closure_add_marshal_guards(GClosure *closure, gpointer pre_marshal_data, GClosureNotify	pre_marshal_notify, gpointer post_marshal_data,
						          GClosureNotify post_marshal_notify);
void g_closure_set_marshal(GClosure *closure, GClosureMarshal marshal);
void g_closure_set_meta_marshal(GClosure *closure, gpointer marshal_data, GClosureMarshal meta_marshal);
void g_closure_invalidate(GClosure *closure);
void g_closure_invoke(GClosure *closure, GValue	*return_value, guint n_param_values, const GValue *param_values, gpointer invocation_hint);
G_END_DECLS
#endif
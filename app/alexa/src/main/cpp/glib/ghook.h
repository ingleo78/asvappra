#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_HOOK_H__
#define __G_HOOK_H__

#include "gmem.h"

G_BEGIN_DECLS
typedef struct _GHook GHook;
typedef struct _GHookList GHookList;
typedef gint (*GHookCompareFunc)(GHook *new_hook, GHook	*sibling);
typedef gboolean (*GHookFindFunc)(GHook	 *hook, gpointer data);
typedef void (*GHookMarshaller)(GHook *hook, gpointer marshal_data);
typedef gboolean (*GHookCheckMarshaller)(GHook *hook, gpointer marshal_data);
typedef void (*GHookFunc)(gpointer data);
typedef gboolean (*GHookCheckFunc)(gpointer	data);
typedef void (*GHookFinalizeFunc)(GHookList *hook_list, GHook *hook);
typedef enum {
  G_HOOK_FLAG_ACTIVE = 1 << 0,
  G_HOOK_FLAG_IN_CALL = 1 << 1,
  G_HOOK_FLAG_MASK = 0x0f
} GHookFlagMask;
#define G_HOOK_FLAG_USER_SHIFT	(4)
struct _GHookList {
  gulong	    seq_id;
  guint		    hook_size : 16;
  guint		    is_setup : 1;
  GHook		   *hooks;
  gpointer	    dummy3;
  GHookFinalizeFunc finalize_hook;
  gpointer	    dummy[2];
};
struct _GHook {
  gpointer	 data;
  GHook		*next;
  GHook		*prev;
  guint		 ref_count;
  gulong	 hook_id;
  guint		 flags;
  gpointer	 func;
  GDestroyNotify destroy;
};
#define	G_HOOK(hook) ((GHook*)(hook))
#define	G_HOOK_FLAGS(hook)	(G_HOOK(hook)->flags)
#define	G_HOOK_ACTIVE(hook)	((G_HOOK_FLAGS(hook) & G_HOOK_FLAG_ACTIVE) != 0)
#define	G_HOOK_IN_CALL(hook)  ((G_HOOK_FLAGS(hook) & G_HOOK_FLAG_IN_CALL) != 0)
#define G_HOOK_IS_VALID(hook) (G_HOOK(hook)->hook_id != 0 && (G_HOOK_FLAGS(hook) & G_HOOK_FLAG_ACTIVE))
#define G_HOOK_IS_UNLINKED(hook)  (G_HOOK(hook)->next == NULL && G_HOOK(hook)->prev == NULL && G_HOOK(hook)->hook_id == 0 && G_HOOK(hook)->ref_count == 0)
void g_hook_list_init(GHookList *hook_list, guint hook_size);
void g_hook_list_clear(GHookList *hook_list);
GHook* g_hook_alloc(GHookList *hook_list);
void g_hook_free(GHookList *hook_list, GHook *hook);
GHook* g_hook_ref(GHookList *hook_list, GHook *hook);
void g_hook_unref(GHookList *hook_list, GHook *hook);
gboolean g_hook_destroy(GHookList *hook_list, gulong hook_id);
void g_hook_destroy_link(GHookList *hook_list, GHook *hook);
void g_hook_prepend(GHookList *hook_list, GHook *hook);
void g_hook_insert_before(GHookList *hook_list, GHook *sibling, GHook *hook);
void g_hook_insert_sorted(GHookList *hook_list, GHook *hook, GHookCompareFunc func);
GHook* g_hook_get(GHookList *hook_list, gulong hook_id);
GHook* g_hook_find(GHookList *hook_list, gboolean need_valids, GHookFindFunc func, gpointer data);
GHook* g_hook_find_data(GHookList *hook_list, gboolean need_valids, gpointer data);
GHook* g_hook_find_func(GHookList *hook_list, gboolean need_valids, gpointer func);
GHook* g_hook_find_func_data(GHookList *hook_list, gboolean need_valids, gpointer func, gpointer data);
GHook* g_hook_first_valid(GHookList *hook_list, gboolean may_be_in_call);
GHook* g_hook_next_valid(GHookList *hook_list, GHook *hook, gboolean may_be_in_call);
gint g_hook_compare_ids(GHook *new_hook, GHook *sibling);
#define	 g_hook_append( hook_list, hook ) g_hook_insert_before((hook_list), NULL, (hook))
void g_hook_list_invoke(GHookList *hook_list, gboolean may_recurse);
void g_hook_list_invoke_check(GHookList *hook_list, gboolean may_recurse);
void g_hook_list_marshal(GHookList *hook_list, gboolean may_recurse, GHookMarshaller marshaller, gpointer marshal_data);
void g_hook_list_marshal_check(GHookList *hook_list, gboolean may_recurse, GHookCheckMarshaller marshaller, gpointer marshal_data);
G_END_DECLS

#endif
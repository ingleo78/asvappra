#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_NODE_H__
#define __G_NODE_H__

#include "gmem.h"

G_BEGIN_DECLS
typedef struct _GNode		GNode;
typedef enum {
  G_TRAVERSE_LEAVES     = 1 << 0,
  G_TRAVERSE_NON_LEAVES = 1 << 1,
  G_TRAVERSE_ALL        = G_TRAVERSE_LEAVES | G_TRAVERSE_NON_LEAVES,
  G_TRAVERSE_MASK       = 0x03,
  G_TRAVERSE_LEAFS      = G_TRAVERSE_LEAVES,
  G_TRAVERSE_NON_LEAFS  = G_TRAVERSE_NON_LEAVES
} GTraverseFlags;
typedef enum {
  G_IN_ORDER,
  G_PRE_ORDER,
  G_POST_ORDER,
  G_LEVEL_ORDER
} GTraverseType;
typedef gboolean (*GNodeTraverseFunc)(GNode	*node, gpointer data);
typedef void (*GNodeForeachFunc)(GNode *node, gpointer data);
typedef gpointer (*GCopyFunc)(gconstpointer src, gpointer data);
struct _GNode {
  gpointer data;
  GNode	  *next;
  GNode	  *prev;
  GNode	  *parent;
  GNode	  *children;
};
#define	 G_NODE_IS_ROOT(node)  (((GNode*)(node))->parent == NULL && ((GNode*) (node))->prev == NULL && ((GNode*) (node))->next == NULL)
#define	 G_NODE_IS_LEAF(node)  (((GNode*)(node))->children == NULL)
GNode* g_node_new(gpointer data);
void g_node_destroy(GNode *root);
void g_node_unlink(GNode *node);
GNode* g_node_copy_deep(GNode *node, GCopyFunc copy_func, gpointer data);
GNode* g_node_copy(GNode *node);
GNode* g_node_insert(GNode *parent, gint position, GNode *node);
GNode* g_node_insert_before(GNode *parent, GNode *sibling, GNode *node);
GNode* g_node_insert_after(GNode *parent, GNode *sibling, GNode *node);
GNode* g_node_prepend(GNode *parent, GNode *node);
guint g_node_n_nodes(GNode *root, GTraverseFlags flags);
GNode* g_node_get_root(GNode *node);
gboolean g_node_is_ancestor(GNode *node, GNode *descendant);
guint g_node_depth(GNode *node);
GNode* g_node_find(GNode *root, GTraverseType order, GTraverseFlags flags, gpointer data);
#define g_node_append(parent, node)	  g_node_insert_before ((parent), NULL, (node))
#define	g_node_insert_data(parent, position, data)	 g_node_insert ((parent), (position), g_node_new (data))
#define	g_node_insert_data_before(parent, sibling, data)  g_node_insert_before ((parent), (sibling), g_node_new (data))
#define	g_node_prepend_data(parent, data)	g_node_prepend ((parent), g_node_new (data))
#define	g_node_append_data(parent, data)   g_node_insert_before ((parent), NULL, g_node_new (data))
void g_node_traverse(GNode *root, GTraverseType order, GTraverseFlags flags, gint max_depth, GNodeTraverseFunc func, gpointer data);
guint g_node_max_height(GNode *root);
void g_node_children_foreach(GNode	*node, GTraverseFlags flags, GNodeForeachFunc func, gpointer data);
void g_node_reverse_children(GNode *node);
guint g_node_n_children(GNode *node);
GNode* g_node_nth_child(GNode *node, guint n);
GNode* g_node_last_child(GNode *node);
GNode* g_node_find_child(GNode *node, GTraverseFlags flags, gpointer data);
gint g_node_child_position(GNode *node, GNode *child);
gint g_node_child_index(GNode *node, gpointer data);
GNode* g_node_first_sibling(GNode *node);
GNode* g_node_last_sibling(GNode *node);
#define	 g_node_prev_sibling(node)	((node) ? ((GNode*) (node))->prev : NULL)
#define	 g_node_next_sibling(node)	((node) ? ((GNode*) (node))->next : NULL)
#define	 g_node_first_child(node)	((node) ? ((GNode*) (node))->children : NULL)
#ifndef G_DISABLE_DEPRECATED
void g_node_push_allocator(gpointer dummy);
void g_node_pop_allocator(void);
#endif
G_END_DECLS

#endif
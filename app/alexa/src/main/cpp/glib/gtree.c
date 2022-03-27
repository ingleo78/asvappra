#include "gtree.h"
#include "gatomic.h"
#include "gtestutils.h"

#undef G_TREE_DEBUG
#define MAX_GTREE_HEIGHT 40
typedef struct _GTreeNode  GTreeNode;
struct _GTree {
  GTreeNode *root;
  GCompareDataFunc key_compare;
  GDestroyNotify key_destroy_func;
  GDestroyNotify value_destroy_func;
  gpointer key_compare_data;
  guint nnodes;
  gint ref_count;
};
struct _GTreeNode {
  gpointer key;
  gpointer value;
  GTreeNode *left;
  GTreeNode *right;
  gint8 balance;
  guint8 left_child;
  guint8 right_child;
};
static GTreeNode* g_tree_node_new(gpointer key, gpointer value);
static void g_tree_insert_internal(GTree *tree, gpointer key, gpointer value, gboolean replace);
static gboolean g_tree_remove_internal(GTree *tree, gconstpointer key, gboolean steal);
static GTreeNode* g_tree_node_balance(GTreeNode *node);
static GTreeNode *g_tree_find_node(GTree *tree, gconstpointer key);
static gint g_tree_node_pre_order(GTreeNode *node, GTraverseFunc traverse_func, gpointer data);
static gint g_tree_node_in_order(GTreeNode *node, GTraverseFunc traverse_func, gpointer data);
static gint g_tree_node_post_order(GTreeNode*node, GTraverseFunc  traverse_func, gpointer data);
static gpointer g_tree_node_search(GTreeNode *node, GCompareFunc search_func, gconstpointer data);
static GTreeNode* g_tree_node_rotate_left(GTreeNode *node);
static GTreeNode* g_tree_node_rotate_right(GTreeNode *node);
#ifndef G_TREE_DEBUG
static void g_tree_node_check(GTreeNode *node);
#endif
static GTreeNode*
g_tree_node_new(gpointer key, gpointer value) {
  GTreeNode *node = g_slice_new (GTreeNode);
  node->balance = 0;
  node->left = NULL;
  node->right = NULL;
  node->left_child = FALSE;
  node->right_child = FALSE;
  node->key = key;
  node->value = value;
  return node;
}
GTree* g_tree_new(GCompareFunc key_compare_func) {
  g_return_val_if_fail(key_compare_func != NULL, NULL);
  return g_tree_new_full((GCompareDataFunc) key_compare_func,NULL,NULL,NULL);
}
GTree* g_tree_new_with_data(GCompareDataFunc key_compare_func, gpointer key_compare_data) {
  g_return_val_if_fail(key_compare_func != NULL, NULL);
  return g_tree_new_full(key_compare_func, key_compare_data,NULL,NULL);
}
GTree* g_tree_new_full(GCompareDataFunc key_compare_func, gpointer key_compare_data, GDestroyNotify key_destroy_func, GDestroyNotify value_destroy_func) {
  GTree *tree;
  g_return_val_if_fail(key_compare_func != NULL, NULL);
  tree = g_slice_new(GTree);
  tree->root = NULL;
  tree->key_compare = key_compare_func;
  tree->key_destroy_func  = key_destroy_func;
  tree->value_destroy_func = value_destroy_func;
  tree->key_compare_data = key_compare_data;
  tree->nnodes = 0;
  tree->ref_count = 1;
  return tree;
}
static inline GTreeNode* g_tree_first_node(GTree *tree) {
  GTreeNode *tmp;
  if (!tree->root) return NULL;
  tmp = tree->root;
  while (tmp->left_child) tmp = tmp->left;
  return tmp;
}
static inline GTreeNode* g_tree_node_previous(GTreeNode *node) {
  GTreeNode *tmp;
  tmp = node->left;
  if (node->left_child)
      while(tmp->right_child) tmp = tmp->right;
  return tmp;
}
static inline GTreeNode* g_tree_node_next(GTreeNode *node) {
  GTreeNode *tmp;
  tmp = node->right;
  if (node->right_child)
      while (tmp->left_child) tmp = tmp->left;
  return tmp;
}
static void g_tree_remove_all(GTree *tree) {
  GTreeNode *node;
  GTreeNode *next;
  g_return_if_fail(tree != NULL);
  node = g_tree_first_node (tree);
  while(node) {
      next = g_tree_node_next(node);
      if (tree->key_destroy_func) tree->key_destroy_func(node->key);
      if (tree->value_destroy_func) tree->value_destroy_func(node->value);
      g_slice_free(GTreeNode, node);
      node = next;
  }
  tree->root = NULL;
  tree->nnodes = 0;
}
GTree* g_tree_ref(GTree *tree) {
  g_return_val_if_fail (tree != NULL, NULL);
  g_atomic_int_inc (&tree->ref_count);
  return tree;
}
void g_tree_unref(GTree *tree) {
  g_return_if_fail(tree != NULL);
  if (g_atomic_int_dec_and_test(&tree->ref_count)) {
      g_tree_remove_all(tree);
      g_slice_free(GTree, tree);
  }
}
void g_tree_destroy(GTree *tree) {
  g_return_if_fail(tree != NULL);
  g_tree_remove_all(tree);
  g_tree_unref(tree);
}
void g_tree_insert(GTree *tree, gpointer key, gpointer value) {
  g_return_if_fail(tree != NULL);
  g_tree_insert_internal(tree, key, value, FALSE);
#ifndef G_TREE_DEBUG
  g_tree_node_check(tree->root);
#endif
}
void g_tree_replace(GTree *tree, gpointer key, gpointer value) {
  g_return_if_fail(tree != NULL);
  g_tree_insert_internal(tree, key, value, TRUE);
#ifdef G_TREE_DEBUG
  g_tree_node_check(tree->root);
#endif
}
static void g_tree_insert_internal(GTree *tree, gpointer key, gpointer value, gboolean replace) {
  GTreeNode *node;
  GTreeNode *path[MAX_GTREE_HEIGHT];
  int idx;
  g_return_if_fail(tree != NULL);
  if (!tree->root) {
      tree->root = g_tree_node_new(key, value);
      tree->nnodes++;
      return;
  }
  idx = 0;
  path[idx++] = NULL;
  node = tree->root;
  while(1) {
      int cmp = tree->key_compare(key, node->key, tree->key_compare_data);
      if (cmp == 0) {
          if (tree->value_destroy_func) tree->value_destroy_func(node->value);
          node->value = value;
          if (replace) {
              if (tree->key_destroy_func) tree->key_destroy_func(node->key);
              node->key = key;
          } else {
              if (tree->key_destroy_func) tree->key_destroy_func(key);
          }
          return;
      } else if (cmp < 0) {
          if (node->left_child) {
              path[idx++] = node;
              node = node->left;
          } else {
              GTreeNode *child = g_tree_node_new(key, value);
              child->left = node->left;
              child->right = node;
              node->left = child;
              node->left_child = TRUE;
              node->balance -= 1;
	          tree->nnodes++;
              break;
          }
      } else {
          if (node->right_child) {
              path[idx++] = node;
              node = node->right;
          } else {
              GTreeNode *child = g_tree_node_new(key, value);
              child->right = node->right;
              child->left = node;
              node->right = child;
              node->right_child = TRUE;
              node->balance += 1;
              tree->nnodes++;
              break;
          }
      }
  };
  while(1) {
      GTreeNode *bparent = path[--idx];
      gboolean left_node = (bparent && node == bparent->left);
      g_assert (!bparent || bparent->left == node || bparent->right == node);
      if (node->balance < -1 || node->balance > 1) {
          node = g_tree_node_balance (node);
          if (bparent == NULL) tree->root = node;
          else if (left_node) bparent->left = node;
          else bparent->right = node;
      }
      if (node->balance == 0 || bparent == NULL) break;
      if (left_node) bparent->balance -= 1;
      else bparent->balance += 1;
      node = bparent;
  }
}
gboolean g_tree_remove(GTree *tree, gconstpointer key) {
  gboolean removed;
  g_return_val_if_fail(tree != NULL, FALSE);
  removed = g_tree_remove_internal(tree, key, FALSE);
#ifndef G_TREE_DEBUG
  g_tree_node_check(tree->root);
#endif
  return removed;
}
gboolean g_tree_steal(GTree *tree, gconstpointer key) {
  gboolean removed;
  g_return_val_if_fail(tree != NULL, FALSE);
  removed = g_tree_remove_internal(tree, key, TRUE);
#ifdef G_TREE_DEBUG
  g_tree_node_check(tree->root);
#endif
  return removed;
}
static gboolean g_tree_remove_internal(GTree *tree, gconstpointer key, gboolean steal) {
    GTreeNode *node, *parent, *balance;
    GTreeNode *path[MAX_GTREE_HEIGHT];
    int idx;
    gboolean left_node;
    g_return_val_if_fail(tree != NULL, FALSE);
    if (!tree->root) return FALSE;
    idx = 0;
    path[idx++] = NULL;
    node = tree->root;
    while (1) {
        int cmp = tree->key_compare(key, node->key, tree->key_compare_data);
        if (cmp == 0) break;
        else if (cmp < 0) {
            if (!node->left_child) return FALSE;
            path[idx++] = node;
            node = node->left;
        } else {
            if (!node->right_child) return FALSE;
            path[idx++] = node;
            node = node->right;
        }
    }
    balance = parent = path[--idx];
    g_assert(!parent || parent->left == node || parent->right == node);
    left_node = (parent && node == parent->left);
    if (!node->left_child) {
        if (!node->right_child) {
            if (!parent) tree->root = NULL;
            else if (left_node) {
                parent->left_child = FALSE;
                parent->left = node->left;
                parent->balance += 1;
            } else {
                parent->right_child = FALSE;
                parent->right = node->right;
                parent->balance -= 1;
            }
        } else {
            GTreeNode *tmp = g_tree_node_next(node);
            tmp->left = node->left;
            if (!parent) tree->root = node->right;
            else if (left_node) {
                parent->left = node->right;
                parent->balance += 1;
            } else {
                parent->right = node->right;
                parent->balance -= 1;
            }
        }
    } else {
        if (!node->right_child) {
            GTreeNode *tmp = g_tree_node_previous(node);
            tmp->right = node->right;
            if (parent == NULL) tree->root = node->left;
            else if (left_node) {
                parent->left = node->left;
                parent->balance += 1;
            } else {
                parent->right = node->left;
                parent->balance -= 1;
            }
        } else {
            GTreeNode *prev = node->left;
            GTreeNode *next = node->right;
            GTreeNode *nextp = node;
            int old_idx = idx + 1;
            idx++;
            while (next->left_child) {
                path[++idx] = nextp = next;
                next = next->left;
            }
            path[old_idx] = next;
            balance = path[idx];
            if (nextp != node) {
                if (next->right_child) nextp->left = next->right;
                else nextp->left_child = FALSE;
                nextp->balance += 1;
                next->right_child = TRUE;
                next->right = node->right;
            } else node->balance -= 1;
            while (prev->right_child) prev = prev->right;
            prev->right = next;
            next->left_child = TRUE;
            next->left = node->left;
            next->balance = node->balance;
            if (!parent) tree->root = next;
            else if (left_node) parent->left = next;
            else parent->right = next;
        }
    }
    if (balance) {
        while (1) {
            GTreeNode *bparent = path[--idx];
            g_assert (!bparent || bparent->left == balance || bparent->right == balance);
            left_node = (bparent && balance == bparent->left);
            if (balance->balance < -1 || balance->balance > 1) {
                balance = g_tree_node_balance(balance);
                if (!bparent) tree->root = balance;
                else if (left_node) bparent->left = balance;
                else bparent->right = balance;
            }
            if (balance->balance != 0 || !bparent) break;
            if (left_node) bparent->balance += 1;
            else bparent->balance -= 1;
            balance = bparent;
        }
    }
    if (!steal) {
        if (tree->key_destroy_func) tree->key_destroy_func (node->key);
        if (tree->value_destroy_func) tree->value_destroy_func (node->value);
    }
    g_slice_free (GTreeNode, node);
    tree->nnodes--;
    return TRUE;
}
gpointer g_tree_lookup(GTree *tree, gconstpointer key) {
  GTreeNode *node;
  g_return_val_if_fail(tree != NULL, NULL);
  node = g_tree_find_node(tree, key);
  return node ? node->value : NULL;
}
gboolean g_tree_lookup_extended(GTree *tree, gconstpointer lookup_key, gpointer *orig_key, gpointer *value) {
  GTreeNode *node;
  g_return_val_if_fail(tree != NULL, FALSE);
  node = g_tree_find_node(tree, lookup_key);
  if (node) {
      if (orig_key) *orig_key = node->key;
      if (value)  *value = node->value;
      return TRUE;
  } else return FALSE;
}
void g_tree_foreach(GTree *tree, GTraverseFunc func, gpointer user_data) {
  GTreeNode *node;
  g_return_if_fail(tree != NULL);
  if (!tree->root) return;
  node = g_tree_first_node(tree);
  while (node) {
      if ((*func)(node->key, node->value, user_data)) break;
      node = g_tree_node_next (node);
  }
}
void g_tree_traverse(GTree *tree, GTraverseFunc traverse_func, GTraverseType traverse_type, gpointer user_data) {
  g_return_if_fail(tree != NULL);
  if (!tree->root) return;
  switch (traverse_type) {
      case G_PRE_ORDER: g_tree_node_pre_order (tree->root, traverse_func, user_data); break;
      case G_IN_ORDER: g_tree_node_in_order (tree->root, traverse_func, user_data); break;
      case G_POST_ORDER: g_tree_node_post_order (tree->root, traverse_func, user_data); break;
      case G_LEVEL_ORDER: g_warning ("g_tree_traverse(): traverse type G_LEVEL_ORDER isn't implemented."); break;
  }
}
gpointer g_tree_search(GTree *tree, GCompareFunc search_func, gconstpointer user_data) {
  g_return_val_if_fail(tree != NULL, NULL);
  if (tree->root) return g_tree_node_search(tree->root, search_func, user_data);
  else return NULL;
}
gint g_tree_height(GTree *tree) {
  GTreeNode *node;
  gint height;
  g_return_val_if_fail(tree != NULL, 0);
  if (!tree->root) return 0;
  height = 0;
  node = tree->root;
  while (1) {
      height += 1 + MAX(node->balance, 0);
      if (!node->left_child) return height;
      node = node->left;
  }
}
gint g_tree_nnodes(GTree *tree) {
  g_return_val_if_fail(tree != NULL, 0);
  return tree->nnodes;
}
static GTreeNode* g_tree_node_balance(GTreeNode *node) {
  if (node->balance < -1) {
      if (node->left->balance > 0) node->left = g_tree_node_rotate_left(node->left);
      node = g_tree_node_rotate_right(node);
  } else if (node->balance > 1) {
      if (node->right->balance < 0) node->right = g_tree_node_rotate_right(node->right);
      node = g_tree_node_rotate_left(node);
  }
  return node;
}
static GTreeNode* g_tree_find_node (GTree *tree, gconstpointer key) {
  GTreeNode *node;
  gint cmp;
  node = tree->root;
  if (!node) return NULL;
  while(1) {
      cmp = tree->key_compare (key, node->key, tree->key_compare_data);
      if (cmp == 0) return node;
      else if (cmp < 0) {
          if (!node->left_child) return NULL;
          node = node->left;
	  } else {
          if (!node->right_child) return NULL;
          node = node->right;
	  }
  }
}
static gint g_tree_node_pre_order(GTreeNode *node, GTraverseFunc traverse_func, gpointer data) {
  if ((*traverse_func) (node->key, node->value, data)) return TRUE;
  if (node->left_child) {
      if (g_tree_node_pre_order (node->left, traverse_func, data)) return TRUE;
  }
  if (node->right_child) {
      if (g_tree_node_pre_order (node->right, traverse_func, data)) return TRUE;
  }
  return FALSE;
}
static gint g_tree_node_in_order(GTreeNode *node, GTraverseFunc traverse_func, gpointer data) {
  if (node->left_child) {
      if (g_tree_node_in_order (node->left, traverse_func, data)) return TRUE;
  }
  if ((*traverse_func)(node->key, node->value, data)) return TRUE;
  if (node->right_child) {
      if (g_tree_node_in_order (node->right, traverse_func, data)) return TRUE;
  }
  return FALSE;
}
static gint g_tree_node_post_order(GTreeNode *node, GTraverseFunc traverse_func, gpointer data) {
  if (node->left_child) {
      if (g_tree_node_post_order (node->left, traverse_func, data)) return TRUE;
  }
  if (node->right_child) {
      if (g_tree_node_post_order (node->right, traverse_func, data)) return TRUE;
  }
  if ((*traverse_func) (node->key, node->value, data)) return TRUE;
  return FALSE;
}
static gpointer g_tree_node_search (GTreeNode *node, GCompareFunc search_func, gconstpointer data) {
  gint dir;
  if (!node) return NULL;
  while(1) {
      dir = (* search_func) (node->key, data);
      if (dir == 0) return node->value;
      else if (dir < 0) {
          if (!node->left_child) return NULL;
          node = node->left;
	  } else {
          if (!node->right_child) return NULL;
          node = node->right;
	  }
  }
}
static GTreeNode* g_tree_node_rotate_left(GTreeNode *node) {
  GTreeNode *right;
  gint a_bal;
  gint b_bal;
  right = node->right;
  if (right->left_child) node->right = right->left;
  else {
      node->right_child = FALSE;
      node->right = right;
      right->left_child = TRUE;
  }
  right->left = node;
  a_bal = node->balance;
  b_bal = right->balance;
  if (b_bal <= 0) {
      if (a_bal >= 1) right->balance = b_bal - 1;
      else right->balance = a_bal + b_bal - 2;
      node->balance = a_bal - 1;
  } else {
      if (a_bal <= b_bal) right->balance = a_bal - 2;
      else right->balance = b_bal - 1;
      node->balance = a_bal - b_bal - 1;
  }
  return right;
}
static GTreeNode* g_tree_node_rotate_right(GTreeNode *node) {
  GTreeNode *left;
  gint a_bal;
  gint b_bal;
  left = node->left;
  if (left->right_child) node->left = left->right;
  else {
      node->left_child = FALSE;
      node->left = left;
      left->right_child = TRUE;
  }
  left->right = node;
  a_bal = node->balance;
  b_bal = left->balance;
  if (b_bal <= 0) {
      if (b_bal > a_bal) left->balance = b_bal + 1;
      else left->balance = a_bal + 2;
      node->balance = a_bal - b_bal + 1;
  } else {
      if (a_bal <= -1) left->balance = b_bal + 1;
      else left->balance = a_bal + b_bal + 2;
      node->balance = a_bal + 1;
  }
  return left;
}
#ifdef G_TREE_DEBUG
static gint g_tree_node_height(GTreeNode *node) {
  gint left_height;
  gint right_height;
  if (node) {
      left_height = 0;
      right_height = 0;
      if (node->left_child) left_height = g_tree_node_height(node->left);
      if (node->right_child) right_height = g_tree_node_height(node->right);
      return MAX(left_height, right_height) + 1;
  }
  return 0;
}
static void g_tree_node_check(GTreeNode *node) {
  gint left_height;
  gint right_height;
  gint balance;
  GTreeNode *tmp;
  if (node) {
      if (node->left_child) {
          tmp = g_tree_node_previous (node);
          g_assert (tmp->right == node);
	  }
      if (node->right_child) {
          tmp = g_tree_node_next (node);
          g_assert (tmp->left == node);
	  }
      left_height = 0;
      right_height = 0;
      if (node->left_child) left_height = g_tree_node_height(node->left);
      if (node->right_child) right_height = g_tree_node_height(node->right);
      balance = right_height - left_height;
      g_assert (balance == node->balance);
      if (node->left_child) g_tree_node_check(node->left);
      if (node->right_child) g_tree_node_check(node->right);
  }
}
static void g_tree_node_dump(GTreeNode *node, gint indent) {
  g_print ("%*s%c\n", indent, "", *(char *)node->key);
  if (node->left_child) g_tree_node_dump(node->left, indent + 2);
  else if (node->left) g_print("%*s<%c\n", indent + 2, "", *(char*)node->left->key);
  if (node->right_child) g_tree_node_dump(node->right, indent + 2);
  else if (node->right) g_print("%*s>%c\n", indent + 2, "", *(char*)node->right->key);
}
void g_tree_dump(GTree *tree) {
  if (tree->root) g_tree_node_dump(tree->root, 0);
}
#endif
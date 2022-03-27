#include "gsequence.h"
#include "gmem.h"
#include "gtestutils.h"

typedef struct _GSequenceNode GSequenceNode;
struct _GSequence {
  GSequenceNode* end_node;
  GDestroyNotify data_destroy_notify;
  gboolean access_prohibited;
  GSequence* real_sequence;
};
struct _GSequenceNode {
  gint n_nodes;
  GSequenceNode* parent;
  GSequenceNode* left;
  GSequenceNode* right;
  gpointer data;
};
static GSequenceNode *node_new(gpointer data);
static GSequenceNode *node_get_first(GSequenceNode *node);
static GSequenceNode *node_get_last(GSequenceNode *node);
static GSequenceNode *node_get_prev(GSequenceNode *node);
static GSequenceNode *node_get_next(GSequenceNode *node);
static gint node_get_pos(GSequenceNode *node);
static GSequenceNode *node_get_by_pos(GSequenceNode *node, gint pos);
static GSequenceNode *node_find(GSequenceNode *haystack, GSequenceNode *needle, GSequenceNode *end, GSequenceIterCompareFunc cmp, gpointer user_data);
static GSequenceNode *node_find_closest(GSequenceNode *haystack, GSequenceNode *needle, GSequenceNode *end, GSequenceIterCompareFunc cmp, gpointer user_data);
static gint node_get_length(GSequenceNode *node);
static void node_free(GSequenceNode *node, GSequence *seq);
static void node_cut(GSequenceNode *split);
static void node_insert_before(GSequenceNode *node, GSequenceNode *new);
static void node_unlink(GSequenceNode *node);
static void node_join(GSequenceNode *left, GSequenceNode *right);
static void node_insert_sorted(GSequenceNode *node, GSequenceNode *new, GSequenceNode *end, GSequenceIterCompareFunc cmp_func, gpointer cmp_data);
static void check_seq_access(GSequence *seq) {
  if (G_UNLIKELY(seq->access_prohibited)) {
      g_warning("Accessing a sequence while it is being sorted or searched is not allowed");
  }
}
static GSequence* get_sequence(GSequenceNode *node) {
  return (GSequence*)node_get_last(node)->data;
}
static void check_iter_access(GSequenceIter *iter) {
  check_seq_access (get_sequence(iter));
}
static gboolean is_end(GSequenceIter *iter) {
  GSequence *seq;
  if (iter->right) return FALSE;
  if (!iter->parent) return TRUE;
  if (iter->parent->right != iter) return FALSE;
  seq = get_sequence(iter);
  return seq->end_node == iter;
}
typedef struct {
  GCompareDataFunc cmp_func;
  gpointer cmp_data;
  GSequenceNode *end_node;
} SortInfo;
static gint iter_compare(GSequenceIter *node1, GSequenceIter *node2, gpointer data) {
  const SortInfo *info = data;
  gint retval;
  if (node1 == info->end_node) return 1;
  if (node2 == info->end_node) return -1;
  retval = info->cmp_func(node1->data, node2->data, info->cmp_data);
  return retval;
}
GSequence* g_sequence_new(GDestroyNotify data_destroy) {
  GSequence *seq = g_new(GSequence, 1);
  seq->data_destroy_notify = data_destroy;
  seq->end_node = node_new(seq);
  seq->access_prohibited = FALSE;
  seq->real_sequence = seq;
  return seq;
}
void g_sequence_free(GSequence *seq) {
  g_return_if_fail(seq != NULL);
  check_seq_access(seq);
  node_free(seq->end_node, seq);
  g_free(seq);
}
void g_sequence_foreach_range(GSequenceIter *begin, GSequenceIter *end, GFunc func, gpointer user_data) {
  GSequence *seq;
  GSequenceIter *iter;
  g_return_if_fail(func != NULL);
  g_return_if_fail(begin != NULL);
  g_return_if_fail(end != NULL);
  seq = get_sequence(begin);
  seq->access_prohibited = TRUE;
  iter = begin;
  while(iter != end) {
      GSequenceIter *next = node_get_next(iter);
      func(iter->data, user_data);
      iter = next;
  }
  seq->access_prohibited = FALSE;
}
void g_sequence_foreach(GSequence *seq, GFunc func, gpointer user_data) {
  GSequenceIter *begin, *end;
  check_seq_access(seq);
  begin = g_sequence_get_begin_iter(seq);
  end   = g_sequence_get_end_iter(seq);
  g_sequence_foreach_range(begin, end, func, user_data);
}
GSequenceIter* g_sequence_range_get_midpoint(GSequenceIter *begin, GSequenceIter *end) {
  int begin_pos, end_pos, mid_pos;
  g_return_val_if_fail(begin != NULL, NULL);
  g_return_val_if_fail(end != NULL, NULL);
  g_return_val_if_fail(get_sequence(begin) == get_sequence(end), NULL);
  begin_pos = node_get_pos(begin);
  end_pos = node_get_pos(end);
  g_return_val_if_fail(end_pos >= begin_pos, NULL);
  mid_pos = begin_pos + (end_pos - begin_pos) / 2;
  return node_get_by_pos(begin, mid_pos);
}
gint g_sequence_iter_compare(GSequenceIter *a, GSequenceIter *b) {
  gint a_pos, b_pos;
  g_return_val_if_fail(a != NULL, 0);
  g_return_val_if_fail(b != NULL, 0);
  g_return_val_if_fail(get_sequence (a) == get_sequence (b), 0);
  check_iter_access(a);
  check_iter_access(b);
  a_pos = node_get_pos(a);
  b_pos = node_get_pos(b);
  if (a_pos == b_pos) return 0;
  else if (a_pos > b_pos) return 1;
  else return -1;
}
GSequenceIter* g_sequence_append(GSequence *seq, gpointer data) {
  GSequenceNode *node;
  g_return_val_if_fail(seq != NULL, NULL);
  check_seq_access(seq);
  node = node_new(data);
  node_insert_before(seq->end_node, node);
  return node;
}
GSequenceIter* g_sequence_prepend(GSequence *seq, gpointer data) {
  GSequenceNode *node, *first;
  g_return_val_if_fail(seq != NULL, NULL);
  check_seq_access(seq);
  node = node_new(data);
  first = node_get_first(seq->end_node);
  node_insert_before(first, node);
  return node;
}
GSequenceIter* g_sequence_insert_before(GSequenceIter *iter, gpointer data) {
  GSequenceNode *node;
  g_return_val_if_fail(iter != NULL, NULL);
  check_iter_access(iter);
  node = node_new(data);
  node_insert_before(iter, node);
  return node;
}
void g_sequence_remove(GSequenceIter *iter) {
  GSequence *seq;
  g_return_if_fail(iter != NULL);
  g_return_if_fail(!is_end (iter));
  check_iter_access(iter);
  seq = get_sequence(iter);
  node_unlink(iter);
  node_free(iter, seq);
}
void g_sequence_remove_range(GSequenceIter *begin, GSequenceIter *end) {
  g_return_if_fail(get_sequence (begin) == get_sequence (end));
  check_iter_access(begin);
  check_iter_access(end);
  g_sequence_move_range(NULL, begin, end);
}
void g_sequence_move_range(GSequenceIter *dest, GSequenceIter *begin, GSequenceIter *end) {
  GSequence *src_seq;
  GSequenceNode *first;
  g_return_if_fail(begin != NULL);
  g_return_if_fail(end != NULL);
  check_iter_access(begin);
  check_iter_access(end);
  if (dest) check_iter_access(dest);
  src_seq = get_sequence(begin);
  g_return_if_fail(src_seq == get_sequence(end));
  if (dest == begin || dest == end) return;
  if (g_sequence_iter_compare (begin, end) >= 0) return;
  if (dest && get_sequence(dest) == src_seq && g_sequence_iter_compare(dest, begin) > 0 && g_sequence_iter_compare(dest, end) < 0) return;
  src_seq = get_sequence(begin);
  first = node_get_first(begin);
  node_cut(begin);
  node_cut(end);
  if (first != begin) node_join(first, end);
  if (dest) {
      first = node_get_first(dest);
      node_cut(dest);
      node_join(begin, dest);
      if (dest != first) node_join(first, begin);
  } else node_free(begin, src_seq);
}
void g_sequence_sort(GSequence *seq, GCompareDataFunc cmp_func, gpointer cmp_data) {
  SortInfo info;
  info.cmp_func = cmp_func;
  info.cmp_data = cmp_data;
  info.end_node = seq->end_node;
  check_seq_access(seq);
  g_sequence_sort_iter(seq, iter_compare, &info);
}
GSequenceIter* g_sequence_insert_sorted(GSequence *seq, gpointer data, GCompareDataFunc cmp_func, gpointer cmp_data) {
  SortInfo info;
  g_return_val_if_fail(seq != NULL, NULL);
  g_return_val_if_fail(cmp_func != NULL, NULL);
  info.cmp_func = cmp_func;
  info.cmp_data = cmp_data;
  info.end_node = seq->end_node;
  check_seq_access(seq);
  return g_sequence_insert_sorted_iter(seq, data, iter_compare, &info);
}
void g_sequence_sort_changed(GSequenceIter *iter, GCompareDataFunc cmp_func, gpointer cmp_data) {
  SortInfo info;
  g_return_if_fail(!is_end (iter));
  info.cmp_func = cmp_func;
  info.cmp_data = cmp_data;
  info.end_node = get_sequence(iter)->end_node;
  check_iter_access(iter);
  g_sequence_sort_changed_iter(iter, iter_compare, &info);
}
GSequenceIter* g_sequence_search(GSequence *seq, gpointer data, GCompareDataFunc cmp_func, gpointer cmp_data) {
  SortInfo info;
  g_return_val_if_fail(seq != NULL, NULL);
  info.cmp_func = cmp_func;
  info.cmp_data = cmp_data;
  info.end_node = seq->end_node;
  check_seq_access(seq);
  return g_sequence_search_iter(seq, data, iter_compare, &info);
}
GSequenceIter* g_sequence_lookup(GSequence *seq, gpointer data, GCompareDataFunc cmp_func, gpointer cmp_data) {
  SortInfo info;
  g_return_val_if_fail(seq != NULL, NULL);
  info.cmp_func = cmp_func;
  info.cmp_data = cmp_data;
  info.end_node = seq->end_node;
  check_seq_access(seq);
  return g_sequence_lookup_iter(seq, data, iter_compare, &info);
}
void g_sequence_sort_iter(GSequence *seq, GSequenceIterCompareFunc cmp_func, gpointer cmp_data) {
  GSequence *tmp;
  GSequenceNode *begin, *end;
  g_return_if_fail(seq != NULL);
  g_return_if_fail(cmp_func != NULL);
  check_seq_access(seq);
  begin = g_sequence_get_begin_iter(seq);
  end   = g_sequence_get_end_iter(seq);
  tmp = g_sequence_new(NULL);
  tmp->real_sequence = seq;
  g_sequence_move_range(g_sequence_get_begin_iter(tmp), begin, end);
  seq->access_prohibited = TRUE;
  tmp->access_prohibited = TRUE;
  while(g_sequence_get_length (tmp) > 0) {
      GSequenceNode *node = g_sequence_get_begin_iter(tmp);
      node_insert_sorted(seq->end_node, node, seq->end_node, cmp_func, cmp_data);
  }
  tmp->access_prohibited = FALSE;
  seq->access_prohibited = FALSE;
  g_sequence_free(tmp);
}
void g_sequence_sort_changed_iter(GSequenceIter *iter, GSequenceIterCompareFunc iter_cmp, gpointer cmp_data) {
  GSequence *seq, *tmp_seq;
  GSequenceIter *next, *prev;
  g_return_if_fail(iter != NULL);
  g_return_if_fail(!is_end (iter));
  g_return_if_fail(iter_cmp != NULL);
  check_iter_access(iter);
  next = node_get_next(iter);
  prev = node_get_prev(iter);
  if (prev != iter && iter_cmp(prev, iter, cmp_data) == 0) return;
  if (!is_end(next) && iter_cmp(next, iter, cmp_data) == 0) return;
  seq = get_sequence(iter);
  seq->access_prohibited = TRUE;
  tmp_seq = g_sequence_new(NULL);
  tmp_seq->real_sequence = seq;
  node_unlink(iter);
  node_insert_before(tmp_seq->end_node, iter);
  node_insert_sorted(seq->end_node, iter, seq->end_node, iter_cmp, cmp_data);
  g_sequence_free(tmp_seq);
  seq->access_prohibited = FALSE;
}
GSequenceIter* g_sequence_insert_sorted_iter(GSequence *seq, gpointer data, GSequenceIterCompareFunc iter_cmp, gpointer cmp_data) {
  GSequenceNode *new_node;
  GSequence *tmp_seq;
  g_return_val_if_fail(seq != NULL, NULL);
  g_return_val_if_fail(iter_cmp != NULL, NULL);
  check_seq_access(seq);
  seq->access_prohibited = TRUE;
  tmp_seq = g_sequence_new(NULL);
  tmp_seq->real_sequence = seq;
  new_node = g_sequence_append(tmp_seq, data);
  node_insert_sorted(seq->end_node, new_node, seq->end_node, iter_cmp, cmp_data);
  g_sequence_free(tmp_seq);
  seq->access_prohibited = FALSE;
  return new_node;
}
GSequenceIter* g_sequence_search_iter(GSequence *seq, gpointer data, GSequenceIterCompareFunc iter_cmp, gpointer cmp_data) {
  GSequenceNode *node;
  GSequenceNode *dummy;
  GSequence *tmp_seq;
  g_return_val_if_fail(seq != NULL, NULL);
  check_seq_access(seq);
  seq->access_prohibited = TRUE;
  tmp_seq = g_sequence_new(NULL);
  tmp_seq->real_sequence = seq;
  dummy = g_sequence_append(tmp_seq, data);
  node = node_find_closest(seq->end_node, dummy, seq->end_node, iter_cmp, cmp_data);
  g_sequence_free(tmp_seq);
  seq->access_prohibited = FALSE;
  return node;
}
GSequenceIter* g_sequence_lookup_iter(GSequence *seq, gpointer data, GSequenceIterCompareFunc iter_cmp, gpointer cmp_data) {
  GSequenceNode *node;
  GSequenceNode *dummy;
  GSequence *tmp_seq;
  g_return_val_if_fail (seq != NULL, NULL);
  check_seq_access (seq);
  seq->access_prohibited = TRUE;
  tmp_seq = g_sequence_new (NULL);
  tmp_seq->real_sequence = seq;
  dummy = g_sequence_append (tmp_seq, data);
  node = node_find (seq->end_node, dummy, seq->end_node, iter_cmp, cmp_data);
  g_sequence_free (tmp_seq);
  seq->access_prohibited = FALSE;
  return node;
}
GSequence* g_sequence_iter_get_sequence(GSequenceIter *iter) {
  GSequence *seq;
  g_return_val_if_fail(iter != NULL, NULL);
  seq = get_sequence(iter);
  return seq->real_sequence;
}
gpointer g_sequence_get(GSequenceIter *iter) {
  g_return_val_if_fail (iter != NULL, NULL);
  g_return_val_if_fail (!is_end (iter), NULL);
  return iter->data;
}
void g_sequence_set(GSequenceIter *iter, gpointer data) {
  GSequence *seq;
  g_return_if_fail(iter != NULL);
  g_return_if_fail(!is_end (iter));
  seq = get_sequence(iter);
  if (seq->data_destroy_notify) seq->data_destroy_notify(iter->data);
  iter->data = data;
}
gint g_sequence_get_length(GSequence *seq) {
  return node_get_length(seq->end_node) - 1;
}
GSequenceIter* g_sequence_get_end_iter(GSequence *seq) {
  g_return_val_if_fail(seq != NULL, NULL);
  return seq->end_node;
}
GSequenceIter* g_sequence_get_begin_iter(GSequence *seq) {
  g_return_val_if_fail(seq != NULL, NULL);
  return node_get_first(seq->end_node);
}
static int clamp_position(GSequence *seq, int pos) {
  gint len = g_sequence_get_length(seq);
  if (pos > len || pos < 0) pos = len;
  return pos;
}
GSequenceIter* g_sequence_get_iter_at_pos(GSequence *seq, gint pos) {
  g_return_val_if_fail(seq != NULL, NULL);
  pos = clamp_position(seq, pos);
  return node_get_by_pos(seq->end_node, pos);
}
void g_sequence_move(GSequenceIter *src, GSequenceIter *dest) {
  g_return_if_fail(src != NULL);
  g_return_if_fail(dest != NULL);
  g_return_if_fail(!is_end (src));
  if (src == dest) return;
  node_unlink(src);
  node_insert_before(dest, src);
}
gboolean g_sequence_iter_is_end(GSequenceIter *iter) {
  g_return_val_if_fail(iter != NULL, FALSE);
  return is_end(iter);
}
gboolean g_sequence_iter_is_begin(GSequenceIter *iter) {
  g_return_val_if_fail(iter != NULL, FALSE);
  return (node_get_prev(iter) == iter);
}
gint g_sequence_iter_get_position(GSequenceIter *iter) {
  g_return_val_if_fail(iter != NULL, -1);
  return node_get_pos(iter);
}
GSequenceIter* g_sequence_iter_next(GSequenceIter *iter) {
  g_return_val_if_fail(iter != NULL, NULL);
  return node_get_next(iter);
}
GSequenceIter* g_sequence_iter_prev(GSequenceIter *iter) {
  g_return_val_if_fail(iter != NULL, NULL);
  return node_get_prev(iter);
}
GSequenceIter* g_sequence_iter_move(GSequenceIter *iter, gint delta) {
  gint new_pos;
  g_return_val_if_fail(iter != NULL, NULL);
  new_pos = node_get_pos(iter) + delta;
  new_pos = clamp_position(get_sequence(iter), new_pos);
  return node_get_by_pos(iter, new_pos);
}
void g_sequence_swap(GSequenceIter *a, GSequenceIter *b) {
  GSequenceNode *leftmost, *rightmost, *rightmost_next;
  int a_pos, b_pos;
  g_return_if_fail(!g_sequence_iter_is_end (a));
  g_return_if_fail(!g_sequence_iter_is_end (b));
  if (a == b) return;
  a_pos = g_sequence_iter_get_position(a);
  b_pos = g_sequence_iter_get_position(b);
  if (a_pos > b_pos) {
      leftmost = b;
      rightmost = a;
  } else {
      leftmost = a;
      rightmost = b;
  }
  rightmost_next = node_get_next(rightmost);
  g_sequence_move(rightmost, leftmost);
  g_sequence_move(leftmost, rightmost_next);
}
static guint get_priority(GSequenceNode *node) {
  guint key = GPOINTER_TO_UINT(node);
  key = (key << 15) - key - 1;
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key + (key << 3) + (key << 11);
  key = key ^ (key >> 16);
  return key? key : 1;
}
static GSequenceNode* find_root(GSequenceNode *node) {
  while(node->parent) node = node->parent;
  return node;
}
static GSequenceNode* node_new(gpointer data) {
  GSequenceNode *node = g_slice_new0(GSequenceNode);
  node->n_nodes = 1;
  node->data = data;
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  return node;
}
static GSequenceNode* node_get_first(GSequenceNode *node) {
  node = find_root(node);
  while(node->left) node = node->left;
  return node;
}
static GSequenceNode* node_get_last(GSequenceNode *node) {
  node = find_root(node);
  while(node->right) node = node->right;
  return node;
}
#define NODE_LEFT_CHILD(n)  (((n)->parent) && ((n)->parent->left) == (n))
#define NODE_RIGHT_CHILD(n) (((n)->parent) && ((n)->parent->right) == (n))
static GSequenceNode* node_get_next(GSequenceNode *node) {
  GSequenceNode *n = node;
  if (n->right) {
      n = n->right;
      while (n->left) n = n->left;
  } else {
      while(NODE_RIGHT_CHILD(n)) n = n->parent;
      if (n->parent) n = n->parent;
      else n = node;
  }
  return n;
}
static GSequenceNode* node_get_prev(GSequenceNode *node) {
  GSequenceNode *n = node;
  if (n->left) {
      n = n->left;
      while(n->right) n = n->right;
  } else {
      while(NODE_LEFT_CHILD(n)) n = n->parent;
      if (n->parent) n = n->parent;
      else n = node;
  }
  return n;
}
#define N_NODES(n) ((n)? (n)->n_nodes : 0)
static gint node_get_pos(GSequenceNode *node) {
  int n_smaller = 0;
  if (node->left) n_smaller = node->left->n_nodes;
  while(node) {
      if (NODE_RIGHT_CHILD (node)) n_smaller += N_NODES(node->parent->left) + 1;
      node = node->parent;
  }
  return n_smaller;
}
static GSequenceNode* node_get_by_pos(GSequenceNode *node, gint pos) {
  int i;
  node = find_root(node);
  while((i = N_NODES(node->left)) != pos) {
      if (i < pos) {
          node = node->right;
          pos -= (i + 1);
      } else node = node->left;
  }
  return node;
}
static GSequenceNode* node_find(GSequenceNode *haystack, GSequenceNode *needle, GSequenceNode *end, GSequenceIterCompareFunc iter_cmp, gpointer cmp_data) {
  gint c;
  haystack = find_root(haystack);
  do {
      if (haystack == end) c = 1;
      else c = iter_cmp(haystack, needle, cmp_data);
      if (c == 0) break;
      if (c > 0) haystack = haystack->left;
      else haystack = haystack->right;
  } while(haystack != NULL);
  return haystack;
}
static GSequenceNode* node_find_closest(GSequenceNode *haystack, GSequenceNode *needle, GSequenceNode *end, GSequenceIterCompareFunc iter_cmp, gpointer cmp_data) {
  GSequenceNode *best;
  gint c;
  haystack = find_root(haystack);
  do {
      best = haystack;
      if (haystack == end) c = 1;
      else c = iter_cmp(haystack, needle, cmp_data);
      if (c > 0) haystack = haystack->left;
      else haystack = haystack->right;
  } while(haystack != NULL);
  if (best != end && c <= 0) best = node_get_next(best);
  return best;
}
static gint node_get_length(GSequenceNode *node) {
  node = find_root(node);
  return node->n_nodes;
}
static void real_node_free(GSequenceNode *node, GSequence *seq) {
  if (node) {
      real_node_free(node->left, seq);
      real_node_free(node->right, seq);
      if (seq && seq->data_destroy_notify && node != seq->end_node) seq->data_destroy_notify(node->data);
      g_slice_free(GSequenceNode, node);
  }
}
static void node_free(GSequenceNode *node, GSequence *seq) {
  node = find_root(node);
  real_node_free(node, seq);
}
static void node_update_fields(GSequenceNode *node) {
  int n_nodes = 1;
  n_nodes += N_NODES(node->left);
  n_nodes += N_NODES(node->right);
  node->n_nodes = n_nodes;
}
static void node_rotate(GSequenceNode *node) {
  GSequenceNode *tmp, *old;
  g_assert(node->parent);
  g_assert(node->parent != node);
  if (NODE_LEFT_CHILD (node)) {
      tmp = node->right;
      node->right = node->parent;
      node->parent = node->parent->parent;
      if (node->parent) {
          if (node->parent->left == node->right) node->parent->left = node;
          else  node->parent->right = node;
      }
      g_assert(node->right);
      node->right->parent = node;
      node->right->left = tmp;
      if (node->right->left) node->right->left->parent = node->right;
      old = node->right;
  } else {
      tmp = node->left;
      node->left = node->parent;
      node->parent = node->parent->parent;
      if (node->parent) {
          if (node->parent->right == node->left) node->parent->right = node;
          else node->parent->left = node;
      }
      g_assert(node->left);
      node->left->parent = node;
      node->left->right = tmp;
      if (node->left->right) node->left->right->parent = node->left;
      old = node->left;
  }
  node_update_fields(old);
  node_update_fields(node);
}
static void node_update_fields_deep(GSequenceNode *node) {
  if (node) {
      node_update_fields(node);
      node_update_fields_deep(node->parent);
  }
}
static void rotate_down(GSequenceNode *node, guint priority) {
  guint left, right;
  left = node->left ? get_priority(node->left)  : 0;
  right = node->right ? get_priority(node->right) : 0;
  while(priority < left || priority < right) {
      if (left > right) node_rotate (node->left);
      else node_rotate (node->right);
      left = node->left ? get_priority(node->left)  : 0;
      right = node->right ? get_priority(node->right) : 0;
  }
}
static void node_cut(GSequenceNode *node) {
  while(node->parent) node_rotate (node);
  if (node->left) node->left->parent = NULL;
  node->left = NULL;
  node_update_fields(node);
  rotate_down(node, get_priority(node));
}
static void node_join(GSequenceNode *left, GSequenceNode *right) {
  GSequenceNode *fake = node_new(NULL);
  fake->left = find_root(left);
  fake->right = find_root(right);
  fake->left->parent = fake;
  fake->right->parent = fake;
  node_update_fields(fake);
  node_unlink(fake);
  node_free(fake, NULL);
}
static void node_insert_before(GSequenceNode *node, GSequenceNode *new) {
  new->left = node->left;
  if (new->left) new->left->parent = new;
  new->parent = node;
  node->left = new;
  node_update_fields_deep (new);
  while(new->parent && get_priority(new) > get_priority(new->parent)) node_rotate(new);
  rotate_down(new, get_priority(new));
}
static void node_unlink(GSequenceNode *node) {
  rotate_down(node, 0);
  if (NODE_RIGHT_CHILD (node)) node->parent->right = NULL;
  else if (NODE_LEFT_CHILD (node)) node->parent->left = NULL;
  if (node->parent) node_update_fields_deep(node->parent);
  node->parent = NULL;
}
static void node_insert_sorted(GSequenceNode *node, GSequenceNode *new, GSequenceNode *end, GSequenceIterCompareFunc iter_cmp, gpointer cmp_data) {
  GSequenceNode *closest;
  closest = node_find_closest(node, new, end, iter_cmp, cmp_data);
  node_unlink(new);
  node_insert_before(closest, new);
}
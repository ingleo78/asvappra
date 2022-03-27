#include "gqueue.h"
#include "gtestutils.h"

GQueue* g_queue_new(void) {
  return g_slice_new0(GQueue);
}
void g_queue_free(GQueue *queue) {
  g_return_if_fail(queue != NULL);
  g_list_free(queue->head);
  g_slice_free(GQueue, queue);
}
void g_queue_init(GQueue *queue) {
  g_return_if_fail(queue != NULL);
  queue->head = queue->tail = NULL;
  queue->length = 0;
}
void g_queue_clear(GQueue *queue) {
  g_return_if_fail(queue != NULL);
  g_list_free(queue->head);
  g_queue_init(queue);
}
gboolean g_queue_is_empty(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, TRUE);
  return queue->head == NULL;
}
guint g_queue_get_length(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, 0);
  return queue->length;
}
void g_queue_reverse(GQueue *queue) {
  g_return_if_fail(queue != NULL);
  queue->tail = queue->head;
  queue->head = g_list_reverse(queue->head);
}
GQueue* g_queue_copy(GQueue *queue) {
  GQueue *result;
  GList *list;
  g_return_val_if_fail(queue != NULL, NULL);
  result = g_queue_new();
  for (list = queue->head; list != NULL; list = list->next) g_queue_push_tail(result, list->data);
  return result;
}
void g_queue_foreach(GQueue *queue, GFunc func, gpointer user_data) {
  GList *list;
  g_return_if_fail(queue != NULL);
  g_return_if_fail(func != NULL);
  list = queue->head;
  while(list) {
      GList *next = list->next;
      func(list->data, user_data);
      list = next;
  }
}
GList* g_queue_find(GQueue *queue, gconstpointer data) {
  g_return_val_if_fail(queue != NULL, NULL);
  return g_list_find(queue->head, data);
}
GList* g_queue_find_custom(GQueue *queue, gconstpointer data, GCompareFunc func) {
  g_return_val_if_fail(queue != NULL, NULL);
  g_return_val_if_fail(func != NULL, NULL);
  return g_list_find_custom(queue->head, data, func);
}
void g_queue_sort(GQueue *queue, GCompareDataFunc compare_func, gpointer user_data) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(compare_func != NULL);
  queue->head = g_list_sort_with_data(queue->head, compare_func, user_data);
  queue->tail = g_list_last(queue->head);
}
void g_queue_push_head(GQueue *queue, gpointer data) {
  g_return_if_fail(queue != NULL);
  queue->head = g_list_prepend(queue->head, data);
  if (!queue->tail) queue->tail = queue->head;
  queue->length++;
}
void g_queue_push_nth(GQueue *queue, gpointer  data, gint n) {
  g_return_if_fail(queue != NULL);
  if (n < 0 || n >= queue->length) {
      g_queue_push_tail(queue, data);
      return;
  }
  g_queue_insert_before (queue, g_queue_peek_nth_link(queue, n), data);
}
void g_queue_push_head_link(GQueue *queue, GList *link) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(link != NULL);
  g_return_if_fail(link->prev == NULL);
  g_return_if_fail(link->next == NULL);
  link->next = queue->head;
  if (queue->head) queue->head->prev = link;
  else queue->tail = link;
  queue->head = link;
  queue->length++;
}
void g_queue_push_tail(GQueue *queue, gpointer data) {
  g_return_if_fail(queue != NULL);
  queue->tail = g_list_append(queue->tail, data);
  if (queue->tail->next) queue->tail = queue->tail->next;
  else queue->head = queue->tail;
  queue->length++;
}
void g_queue_push_tail_link(GQueue *queue, GList *link) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(link != NULL);
  g_return_if_fail(link->prev == NULL);
  g_return_if_fail(link->next == NULL);
  link->prev = queue->tail;
  if (queue->tail) queue->tail->next = link;
  else queue->head = link;
  queue->tail = link;
  queue->length++;
}
void g_queue_push_nth_link(GQueue *queue, gint n, GList *link_) {
  GList *next;
  GList *prev;
  g_return_if_fail(queue != NULL);
  g_return_if_fail(link_ != NULL);
  if (n < 0 || n >= queue->length) {
      g_queue_push_tail_link(queue, link_);
      return;
  }
  g_assert(queue->head);
  g_assert(queue->tail);
  next = g_queue_peek_nth_link(queue, n);
  prev = next->prev;
  if (prev) prev->next = link_;
  next->prev = link_;
  link_->next = next;
  link_->prev = prev;
  if (queue->head->prev) queue->head = queue->head->prev;
  if (queue->tail->next) queue->tail = queue->tail->next;
  queue->length++;
}
gpointer g_queue_pop_head(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  if (queue->head) {
      GList *node = queue->head;
      gpointer data = node->data;
      queue->head = node->next;
      if (queue->head) queue->head->prev = NULL;
      else queue->tail = NULL;
      g_list_free_1(node);
      queue->length--;
      return data;
  }
  return NULL;
}
GList* g_queue_pop_head_link(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  if (queue->head) {
      GList *node = queue->head;
      queue->head = node->next;
      if (queue->head){
          queue->head->prev = NULL;
          node->next = NULL;
	  } else queue->tail = NULL;
      queue->length--;
      return node;
  }
  return NULL;
}
GList* g_queue_peek_head_link(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  return queue->head;
}
GList* g_queue_peek_tail_link(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  return queue->tail;
}
gpointer g_queue_pop_tail(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  if (queue->tail) {
      GList *node = queue->tail;
      gpointer data = node->data;
      queue->tail = node->prev;
      if (queue->tail) queue->tail->next = NULL;
      else queue->head = NULL;
      queue->length--;
      g_list_free_1(node);
      return data;
  }
  return NULL;
}
gpointer g_queue_pop_nth(GQueue *queue, guint n) {
  GList *nth_link;
  gpointer result;
  g_return_val_if_fail(queue != NULL, NULL);
  if (n >= queue->length) return NULL;
  nth_link = g_queue_peek_nth_link(queue, n);
  result = nth_link->data;
  g_queue_delete_link(queue, nth_link);
  return result;
}
GList* g_queue_pop_tail_link(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  if (queue->tail) {
      GList *node = queue->tail;
      queue->tail = node->prev;
      if (queue->tail) {
          queue->tail->next = NULL;
          node->prev = NULL;
	  } else queue->head = NULL;
      queue->length--;
      return node;
  }
  return NULL;
}
GList* g_queue_pop_nth_link(GQueue *queue, guint n) {
  GList *link;
  g_return_val_if_fail(queue != NULL, NULL);
  if (n >= queue->length) return NULL;
  link = g_queue_peek_nth_link(queue, n);
  g_queue_unlink (queue, link);
  return link;
}
GList* g_queue_peek_nth_link(GQueue *queue, guint n) {
  GList *link;
  gint i;
  g_return_val_if_fail(queue != NULL, NULL);
  if (n >= queue->length) return NULL;
  if (n > queue->length / 2) {
      n = queue->length - n - 1;
      link = queue->tail;
      for (i = 0; i < n; ++i) link = link->prev;
  } else {
      link = queue->head;
      for (i = 0; i < n; ++i) link = link->next;
  }
  return link;
}
gint g_queue_link_index(GQueue *queue, GList *link_) {
  g_return_val_if_fail(queue != NULL, -1);
  return g_list_position(queue->head, link_);
}
void g_queue_unlink(GQueue *queue, GList *link_) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(link_ != NULL);
  if (link_ == queue->tail) queue->tail = queue->tail->prev;
  queue->head = g_list_remove_link(queue->head, link_);
  queue->length--;
}
void g_queue_delete_link(GQueue *queue, GList *link_) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(link_ != NULL);
  g_queue_unlink(queue, link_);
  g_list_free(link_);
}
gpointer g_queue_peek_head(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  return queue->head ? queue->head->data : NULL;
}
gpointer g_queue_peek_tail(GQueue *queue) {
  g_return_val_if_fail(queue != NULL, NULL);
  return queue->tail ? queue->tail->data : NULL;
}
gpointer g_queue_peek_nth(GQueue *queue, guint n) {
  GList *link;
  g_return_val_if_fail(queue != NULL, NULL);
  link = g_queue_peek_nth_link(queue, n);
  if (link) return link->data;
  return NULL;
}
gint g_queue_index(GQueue *queue, gconstpointer data) {
  g_return_val_if_fail(queue != NULL, -1);
  return g_list_index(queue->head, data);
}
gboolean g_queue_remove(GQueue *queue, gconstpointer data) {
  GList *link;
  if (queue == NULL) return FALSE;
  link = g_list_find(queue->head, data);
  if (link) {
      g_queue_delete_link(queue, link);
      return TRUE;
  } else return FALSE;
}
void g_queue_remove_all(GQueue *queue, gconstpointer data) {
  GList *list;
  g_return_if_fail(queue != NULL);
  list = queue->head;
  while(list) {
      GList *next = list->next;
      if (list->data == data) g_queue_delete_link(queue, list);
      list = next;
  }
}
void g_queue_insert_before(GQueue *queue, GList *sibling, gpointer data) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(sibling != NULL);
  queue->head = g_list_insert_before(queue->head, sibling, data);
  queue->length++;
}
void g_queue_insert_after(GQueue *queue, GList *sibling, gpointer data) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(sibling != NULL);
  if (sibling == queue->tail) g_queue_push_tail(queue, data);
  else g_queue_insert_before(queue, sibling->next, data);
}
void g_queue_insert_sorted(GQueue *queue, gpointer data, GCompareDataFunc func, gpointer user_data) {
  GList *list;
  g_return_if_fail(queue != NULL);
  list = queue->head;
  while(list && func(list->data, data, user_data) < 0) list = list->next;
  if (list) g_queue_insert_before(queue, list, data);
  else g_queue_push_tail(queue, data);
}
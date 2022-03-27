#include <stdlib.h>
#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../gobject/gboxed.h"
#include "config.h"
#include "gsrvtarget.h"

struct _GSrvTarget {
  gchar *hostname;
  guint16 port;
  guint16 priority;
  guint16 weight;
};
G_DEFINE_BOXED_TYPE(GSrvTarget, g_srv_target, g_srv_target_copy, g_srv_target_free);
GSrvTarget *g_srv_target_new(const gchar *hostname, guint16 port, guint16 priority, guint16 weight) {
  GSrvTarget *target = g_slice_new0(GSrvTarget);
  target->hostname = g_strdup(hostname);
  target->port = port;
  target->priority = priority;
  target->weight = weight;
  return target;
}
GSrvTarget *g_srv_target_copy(GSrvTarget *target) {
  return g_srv_target_new(target->hostname, target->port, target->priority, target->weight);
}
void g_srv_target_free(GSrvTarget *target) {
  g_free(target->hostname);
  g_slice_free(GSrvTarget, target);
}
const gchar * g_srv_target_get_hostname(GSrvTarget *target) {
  return target->hostname;
}
guint16 g_srv_target_get_port(GSrvTarget *target) {
  return target->port;
}
guint16 g_srv_target_get_priority(GSrvTarget *target) {
  return target->priority;
}
guint16 g_srv_target_get_weight(GSrvTarget *target) {
  return target->weight;
}
gint compare_target(gconstpointer a, gconstpointer b) {
  GSrvTarget *ta = (GSrvTarget*)a;
  GSrvTarget *tb = (GSrvTarget*)b;
  if (ta->priority == tb->priority) return ta->weight - tb->weight;
  else return ta->priority - tb->priority;
}
GList *g_srv_target_list_sort(GList *targets) {
  gint sum, num, val, priority, weight;
  GList *t, *out, *tail;
  GSrvTarget *target;
  if (!targets) return NULL;
  if (!targets->next) {
      target = targets->data;
      if (!strcmp(target->hostname, ".")) {
          g_srv_target_free(target);
          g_list_free(targets);
          return NULL;
      }
  }
  targets = g_list_sort(targets, compare_target);
  out = tail = NULL;
  while(targets) {
      priority = ((GSrvTarget*)targets->data)->priority;
      sum = num = 0;
      for (t = targets; t; t = t->next) {
          target = (GSrvTarget*)t->data;
          if (target->priority != priority) break;
          sum += target->weight;
          num++;
      }
      while(num) {
          val = g_random_int_range(0, sum + 1);
          for (t = targets; ; t = t->next) {
              weight = ((GSrvTarget*)t->data)->weight;
              if (weight >= val) break;
              val -= weight;
          }
          targets = g_list_remove_link(targets, t);
          if (!out) out = t;
          else tail->next = t;
          tail = t;
          sum -= weight;
          num--;
      }
  }
  return out;
}
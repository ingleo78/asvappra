#include <string.h>
#include "../../glib/glib.h"
#include "../config.h"
#include "inotify-sub.h"

static gboolean is_debug_enabled = FALSE;
#define IS_W if (is_debug_enabled) g_warning
static gchar* dup_dirname(const gchar *dirname) {
  gchar *d_dirname = g_strdup(dirname);
  size_t len = strlen(d_dirname);
  if (d_dirname[len - 1] == '/') d_dirname[len - 1] = '\0';
  return d_dirname;
}
inotify_sub* _ih_sub_new(const gchar *dirname, const gchar *filename, gboolean pair_moves, gpointer user_data) {
  inotify_sub *sub = NULL;
  sub = g_new0(inotify_sub, 1);
  sub->dirname = dup_dirname(dirname);
  sub->filename = g_strdup(filename);
  sub->pair_moves = pair_moves;
  sub->user_data = user_data;
  IS_W("new subscription for %s being setup\n", sub->dirname);
  return sub;
}
void _ih_sub_free(inotify_sub *sub) {
  g_free(sub->dirname);
  g_free(sub->filename);
  g_free(sub);
}
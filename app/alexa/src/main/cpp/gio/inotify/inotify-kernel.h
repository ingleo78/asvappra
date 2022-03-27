#ifndef __INOTIFY_KERNEL_H
#define __INOTIFY_KERNEL_H

#include "../../glib/glib.h"

typedef struct ik_event_s {
  gint32 wd;
  guint32 mask;
  guint32 cookie;
  guint32 len;
  char *  name;
  struct ik_event_s *pair;
} ik_event_t;
gboolean _ik_startup(void (*cb)(ik_event_t *event));
ik_event_t *_ik_event_new_dummy(const char *name, gint32 wd, guint32 mask);
void  _ik_event_free(ik_event_t *event);
gint32 _ik_watch(const char *path, guint32 mask, int *err);
int _ik_ignore(const char *path, gint32 wd);
void _ik_move_stats(guint32 *matches, guint32 *misses);
const char *_ik_mask_to_string(guint32  mask);

#endif

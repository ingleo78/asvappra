#ifndef __INOTIFY_SUB_H
#define __INOTIFY_SUB_H

#include "../../glib/glib.h"

typedef struct {
	gchar* dirname;
	gchar* filename;
	gboolean cancelled;
	gpointer user_data;
    gboolean pair_moves;
} inotify_sub;
inotify_sub* _ih_sub_new(const gchar* dirname, const gchar* filename, gboolean pair_moves, gpointer user_data);
void _ih_sub_free(inotify_sub* sub);

#endif
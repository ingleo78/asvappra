#ifndef __G_ASYNC_HELPER_H__
#define __G_ASYNC_HELPER_H__

#include "../glib/glib.h"
#include "gio.h"
#include "giotypes.h"

G_BEGIN_DECLS
typedef gboolean (*GFDSourceFunc)(int fd, GIOCondition condition, gpointer user_data);
GSource *_g_fd_source_new(int fd, gushort events, GCancellable *cancellable);
G_END_DECLS

#endif
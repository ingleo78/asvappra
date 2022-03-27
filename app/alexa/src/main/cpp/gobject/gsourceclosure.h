#if defined (__GLIB_GOBJECT_H_INSIDE__) && !defined (GOBJECT_COMPILATION)
//#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_SOURCECLOSURE_H__
#define __G_SOURCECLOSURE_H__

#include "../glib/gmain.h"
#include "gclosure.h"

G_BEGIN_DECLS
void g_source_set_closure(GSource  *source, GClosure *closure);
void g_source_set_dummy_callback (GSource  *source);
GType g_io_channel_get_type(void);
GType g_io_condition_get_type(void);
#define G_TYPE_IO_CHANNEL  (g_io_channel_get_type())
#define G_TYPE_IO_CONDITION  (g_io_condition_get_type())
G_END_DECLS
#endif
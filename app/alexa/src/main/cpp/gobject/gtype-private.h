#if defined (__GLIB_GOBJECT_H_INSIDE__) && !defined (GOBJECT_COMPILATION)
#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_TYPE_PRIVATE_H__
#define __G_TYPE_PRIVATE_H__

#include "gboxed.h"

G_BEGIN_DECLS
gpointer _g_type_boxed_copy(GType type, gpointer value);
void _g_type_boxed_free(GType type, gpointer value);
void _g_type_boxed_init(GType type, GBoxedCopyFunc copy_func, GBoxedFreeFunc free_func);
G_END_DECLS
#endif
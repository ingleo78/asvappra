#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_DIR_H__
#define __G_DIR_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gerror.h"

G_BEGIN_DECLS
typedef struct _GDir GDir;
#ifdef G_OS_WIN32
#define g_dir_open g_dir_open_utf8
#define g_dir_read_name g_dir_read_name_utf8
#endif
GDir* g_dir_open(const gchar *path, guint flags, GError **error);
G_CONST_RETURN gchar *g_dir_read_name(GDir *dir);
void g_dir_rewind(GDir *dir);
void g_dir_close(GDir *dir);
G_END_DECLS

#endif
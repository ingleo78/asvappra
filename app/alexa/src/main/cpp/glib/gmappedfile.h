#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_MAPPED_FILE_H__
#define __G_MAPPED_FILE_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gerror.h"

G_BEGIN_DECLS
typedef struct _GMappedFile GMappedFile;
GMappedFile *g_mapped_file_new(const char *filename, int writable, GError **error) G_GNUC_MALLOC;
gsize g_mapped_file_get_length(GMappedFile *file);
char *g_mapped_file_get_contents(GMappedFile *file);
GMappedFile *g_mapped_file_ref(GMappedFile *file);
void g_mapped_file_unref(GMappedFile *file);
#ifndef G_DISABLE_DEPRECATED
void g_mapped_file_free(GMappedFile *file);
#endif
G_END_DECLS

#endif
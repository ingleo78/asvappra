#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILENAME_COMPLETER_H__
#define __G_FILENAME_COMPLETER_H__

#include "../gobject/gobject.h"
#include "../gobject/gtype.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_FILENAME_COMPLETER  (g_filename_completer_get_type())
#define G_FILENAME_COMPLETER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_FILENAME_COMPLETER, GFilenameCompleter))
#define G_FILENAME_COMPLETER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FILENAME_COMPLETER, GFilenameCompleterClass))
#define G_FILENAME_COMPLETER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_FILENAME_COMPLETER, GFilenameCompleterClass))
#define G_IS_FILENAME_COMPLETER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_FILENAME_COMPLETER))
#define G_IS_FILENAME_COMPLETER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_FILENAME_COMPLETER))
typedef struct _GFilenameCompleterClass GFilenameCompleterClass;
struct _GFilenameCompleterClass {
  GObjectClass parent_class;
  void (* got_completion_data)(GFilenameCompleter *filename_completer);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
};
GType g_filename_completer_get_type(void) G_GNUC_CONST;
GFilenameCompleter *g_filename_completer_new(void);
char *g_filename_completer_get_completion_suffix(GFilenameCompleter *completer, const char *initial_text);
char **g_filename_completer_get_completions(GFilenameCompleter *completer, const char *initial_text);
void g_filename_completer_set_dirs_only(GFilenameCompleter *completer, gboolean dirs_only);
G_END_DECLS

#endif
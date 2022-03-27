#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_APPLICATION_COMMAND_LINE_H__
#define __G_APPLICATION_COMMAND_LINE_H__

#include "../gobject/gtype.h"
#include "../gobject/gobject.h"
#include "../glib/gvariant.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_APPLICATION_COMMAND_LINE  (g_application_command_line_get_type())
#define G_APPLICATION_COMMAND_LINE(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_APPLICATION_COMMAND_LINE, GApplicationCommandLine))
#define G_APPLICATION_COMMAND_LINE_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_APPLICATION_COMMAND_LINE, GApplicationCommandLineClass))
#define G_IS_APPLICATION_COMMAND_LINE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_APPLICATION_COMMAND_LINE))
#define G_IS_APPLICATION_COMMAND_LINE_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_APPLICATION_COMMAND_LINE))
#define G_APPLICATION_COMMAND_LINE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_APPLICATION_COMMAND_LINE, GApplicationCommandLineClass))
typedef struct _GApplicationCommandLinePrivate GApplicationCommandLinePrivate;
typedef struct _GApplicationCommandLineClass GApplicationCommandLineClass;
struct _GApplicationCommandLine {
  GObject parent_instance;
  GApplicationCommandLinePrivate *priv;
};
struct _GApplicationCommandLineClass {
  GObjectClass parent_class;
  void (*print_literal)(GApplicationCommandLine *cmdline, const gchar *message);
  void (*printerr_literal)(GApplicationCommandLine *cmdline, const gchar *message);
  gpointer padding[12];
};
GType g_application_command_line_get_type(void) G_GNUC_CONST;
gchar **g_application_command_line_get_arguments(GApplicationCommandLine *cmdline, int *argc);
const gchar * const *g_application_command_line_get_environ(GApplicationCommandLine *cmdline);
const gchar *g_application_command_line_getenv(GApplicationCommandLine *cmdline, const gchar *name);
const gchar *g_application_command_line_get_cwd(GApplicationCommandLine *cmdline);
gboolean g_application_command_line_get_is_remote(GApplicationCommandLine *cmdline);
void g_application_command_line_print(GApplicationCommandLine *cmdline, const gchar *format, ...) G_GNUC_PRINTF(2,3);
void g_application_command_line_printerr(GApplicationCommandLine *cmdline, const gchar *format, ...) G_GNUC_PRINTF(2,3);
int g_application_command_line_get_exit_status(GApplicationCommandLine *cmdline);
void g_application_command_line_set_exit_status(GApplicationCommandLine *cmdline, int exit_status);
GVariant *g_application_command_line_get_platform_data(GApplicationCommandLine *cmdline);
G_END_DECLS

#endif
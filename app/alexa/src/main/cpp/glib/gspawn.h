#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_SPAWN_H__
#define __G_SPAWN_H__

#include "gerror.h"

G_BEGIN_DECLS
#define G_SPAWN_ERROR g_spawn_error_quark()
typedef enum {
  G_SPAWN_ERROR_FORK,
  G_SPAWN_ERROR_READ,
  G_SPAWN_ERROR_CHDIR,
  G_SPAWN_ERROR_ACCES,
  G_SPAWN_ERROR_PERM,
  G_SPAWN_ERROR_2BIG,
  G_SPAWN_ERROR_NOEXEC,
  G_SPAWN_ERROR_NAMETOOLONG,
  G_SPAWN_ERROR_NOENT,
  G_SPAWN_ERROR_NOMEM,
  G_SPAWN_ERROR_NOTDIR,
  G_SPAWN_ERROR_LOOP,
  G_SPAWN_ERROR_TXTBUSY,
  G_SPAWN_ERROR_IO,
  G_SPAWN_ERROR_NFILE,
  G_SPAWN_ERROR_MFILE,
  G_SPAWN_ERROR_INVAL,
  G_SPAWN_ERROR_ISDIR,
  G_SPAWN_ERROR_LIBBAD,
  G_SPAWN_ERROR_FAILED
} GSpawnError;
typedef void (*GSpawnChildSetupFunc)(gpointer user_data);
typedef enum {
  G_SPAWN_LEAVE_DESCRIPTORS_OPEN = 1 << 0,
  G_SPAWN_DO_NOT_REAP_CHILD      = 1 << 1,
  G_SPAWN_SEARCH_PATH            = 1 << 2,
  G_SPAWN_STDOUT_TO_DEV_NULL     = 1 << 3,
  G_SPAWN_STDERR_TO_DEV_NULL     = 1 << 4,
  G_SPAWN_CHILD_INHERITS_STDIN   = 1 << 5,
  G_SPAWN_FILE_AND_ARGV_ZERO     = 1 << 6
} GSpawnFlags;
GQuark g_spawn_error_quark (void);
#ifdef G_OS_WIN32
#define g_spawn_async g_spawn_async_utf8
#define g_spawn_async_with_pipes g_spawn_async_with_pipes_utf8
#define g_spawn_sync g_spawn_sync_utf8
#define g_spawn_command_line_sync g_spawn_command_line_sync_utf8
#define g_spawn_command_line_async g_spawn_command_line_async_utf8
#endif
gboolean g_spawn_async(const gchar *working_directory, gchar **argv, gchar **envp, GSpawnFlags flags, GSpawnChildSetupFunc child_setup, gpointer user_data,
                       GPid *child_pid, GError **error);
gboolean g_spawn_async_with_pipes(const gchar *working_directory, gchar **argv, gchar **envp, GSpawnFlags flags, GSpawnChildSetupFunc child_setup, gpointer user_data,
                                  GPid *child_pid, gint *standard_input, gint *standard_output, gint *standard_error, GError **error);
gboolean g_spawn_sync(const gchar *working_directory, gchar **argv, gchar **envp, GSpawnFlags flags, GSpawnChildSetupFunc child_setup, gpointer user_data,
                      gchar **standard_output, gchar **standard_error, gint *exit_status, GError **error);
gboolean g_spawn_command_line_sync(const gchar *command_line, gchar **standard_output, gchar **standard_error, gint *exit_status, GError **error);
gboolean g_spawn_command_line_async(const gchar *command_line, GError **error);
void g_spawn_close_pid (GPid pid);
G_END_DECLS

#endif
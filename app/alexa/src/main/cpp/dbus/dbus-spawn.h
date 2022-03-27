#ifndef DBUS_SPAWN_H
#define DBUS_SPAWN_H

#include "dbus-string.h"
#include "dbus-errors.h"
#include "dbus-watch.h"

DBUS_BEGIN_DECLS
typedef void (*DBusSpawnChildSetupFunc)(void *user_data);
typedef struct DBusBabysitter DBusBabysitter;
typedef void (*DBusBabysitterFinishedFunc)(DBusBabysitter *sitter, void *user_data);
typedef enum {
  DBUS_SPAWN_REDIRECT_OUTPUT = (1 << 0),
  DBUS_SPAWN_SILENCE_OUTPUT = (1 << 1),
  DBUS_SPAWN_NONE = 0
} DBusSpawnFlags;
dbus_bool_t _dbus_spawn_async_with_babysitter(DBusBabysitter **sitter_p, const char *log_name, char * const *argv, char **env, DBusSpawnFlags flags,
                                              DBusSpawnChildSetupFunc child_setup, void *user_data, DBusError *error);
void _dbus_babysitter_set_result_function(DBusBabysitter *sitter, DBusBabysitterFinishedFunc finished, void *user_data);
DBusBabysitter* _dbus_babysitter_ref(DBusBabysitter *sitter);
void _dbus_babysitter_unref(DBusBabysitter *sitter);
void _dbus_babysitter_kill_child(DBusBabysitter *sitter);
dbus_bool_t _dbus_babysitter_get_child_exited(DBusBabysitter *sitter);
void _dbus_babysitter_set_child_exit_error(DBusBabysitter *sitter, DBusError *error);
dbus_bool_t _dbus_babysitter_get_child_exit_status(DBusBabysitter *sitter, int *status);
dbus_bool_t _dbus_babysitter_set_watch_functions(DBusBabysitter *sitter, DBusAddWatchFunction add_function, DBusRemoveWatchFunction remove_function,
                                                 DBusWatchToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function);
void _dbus_babysitter_block_for_child_exit(DBusBabysitter *sitter);
DBUS_END_DECLS

#endif
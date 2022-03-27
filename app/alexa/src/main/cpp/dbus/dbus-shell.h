#ifndef DBUS_SHELL_H
#define DBUS_SHELL_H

#include "dbus-macros.h"
#include "dbus-types.h"

DBUS_BEGIN_DECLS
char* _dbus_shell_unquote(const char *quoted_string);
dbus_bool_t _dbus_shell_parse_argv (const char *command_line, int *argcp, char ***argvp, DBusError *error);
DBUS_END_DECLS

#endif
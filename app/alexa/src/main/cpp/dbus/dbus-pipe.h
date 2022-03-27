#ifndef DBUS_PIPE_H
#define DBUS_PIPE_H

#include <stdint.h>
#include <inttypes.h>
#include "dbus-types.h"
#include "dbus-errors.h"
#include "dbus-string.h"
#include "dbus-sysdeps.h"

struct DBusPipe {
  int fd;
};
DBUS_PRIVATE_EXPORT void _dbus_pipe_init(DBusPipe *pipe, int fd);
DBUS_PRIVATE_EXPORT void _dbus_pipe_init_stdout(DBusPipe *pipe);
DBUS_PRIVATE_EXPORT int _dbus_pipe_write(DBusPipe *pipe, const DBusString *buffer, int start, int len, DBusError *error);
DBUS_PRIVATE_EXPORT int _dbus_pipe_close(DBusPipe *pipe, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_pipe_is_valid(DBusPipe *pipe);
DBUS_PRIVATE_EXPORT void _dbus_pipe_invalidate(DBusPipe *pipe);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_pipe_is_stdout_or_stderr(DBusPipe *pipe);

#endif
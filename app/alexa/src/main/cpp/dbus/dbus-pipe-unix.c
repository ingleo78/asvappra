#include <errno.h>
#include "config.h"
#include "dbus-protocol.h"
#include "dbus-string.h"
#include "dbus-internals.h"
#include "dbus-pipe.h"
#include "dbus-sysdeps-unix.h"

int _dbus_pipe_write(DBusPipe *pipe, const DBusString *buffer, int start, int len, DBusError *error) {
  int written;
  written = _dbus_write(pipe->fd, buffer, start, len);
  if (written < 0) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Writing to pipe: %s\n", _dbus_strerror(errno));
  }
  return written;
}
int _dbus_pipe_close(DBusPipe *pipe, DBusError *error) {
  if (!_dbus_close(pipe->fd, error)) return -1;
  else {
      _dbus_pipe_invalidate(pipe);
      return 0;
  }
}
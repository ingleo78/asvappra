#include "config.h"
#include "dbus-protocol.h"
#include "dbus-string.h"
#include "dbus-internals.h"
#include "dbus-pipe.h"

int _dbus_pipe_write(DBusPipe *pipe, const DBusString *buffer, int start, int len, DBusError *error) {
  /*const char *buffer_c = _dbus_string_get_const_data(buffer);
  int written;
  written = _write(pipe->fd, buffer_c + start, len);
  if (written >= 0) return written;
  dbus_set_error(error, _dbus_error_from_system_errno(),"Writing to pipe: %s", _dbus_strerror_from_errno());*/
  return -1;
}
int _dbus_pipe_close(DBusPipe *pipe, DBusError *error) {
  /*_DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (_close (pipe->fd) != 0) {
      dbus_set_error(error, _dbus_error_from_system_errno (),"Could not close pipe fd %d: %s", pipe->fd, _dbus_strerror_from_errno());
      return -1;
  } else {
      _dbus_pipe_invalidate(pipe);
      return 0;
  }*/
  return -1;
}
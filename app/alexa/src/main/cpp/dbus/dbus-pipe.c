#include "config.h"
#include "dbus-pipe.h"

void _dbus_pipe_init(DBusPipe *pipe, int fd) {
  pipe->fd = fd;
}
void _dbus_pipe_init_stdout(DBusPipe *pipe) {
  _dbus_pipe_init(pipe, 1);
}
dbus_bool_t _dbus_pipe_is_valid(DBusPipe *pipe) {
  return pipe->fd >= 0;
}
dbus_bool_t _dbus_pipe_is_stdout_or_stderr(DBusPipe *pipe) {
  return pipe->fd == 1 || pipe->fd == 2;
}
void _dbus_pipe_invalidate(DBusPipe *pipe) {
  pipe->fd = -1;
}
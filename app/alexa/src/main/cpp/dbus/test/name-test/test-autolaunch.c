#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../config.h"
#include "../../dbus.h"
#include "../../dbus-internals.h"
#include "../../dbus-sysdeps.h"

int main(int argc, char *argv[]) {
  DBusConnection *conn = NULL;
  DBusError error;
  DBusGUID uuid;
  dbus_setenv("DBUS_SESSION_BUS_ADDRESS", NULL);
  dbus_error_init(&error);
  if (!_dbus_read_local_machine_uuid(&uuid, FALSE, &error)) {
      fprintf(stderr, "*** %s\n", error.message);
      dbus_error_free(&error);
      return 0;
  }
  conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
#ifdef DBUS_ENABLE_X11_AUTOLAUNCH
  if (_dbus_getenv("DISPLAY") != NULL && dbus_error_is_set(&error)) {
      fprintf(stderr, "*** Failed to autolaunch session bus: %s\n", error.message);
      dbus_error_free(&error);
      return 1;
  }
#endif
  if (dbus_error_is_set(&error) && conn != NULL) {
      fprintf(stderr, "*** Autolaunched session bus, but an error was set!\n");
      return 1;
  }
  if (!dbus_error_is_set(&error) && conn == NULL) {
      fprintf(stderr, "*** Failed to autolaunch session bus but no error was set\n");
      return 1;
  }
  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "utils.h"
#include "activation-helper.h"
#include "activation-exit-codes.h"

static int convert_error_to_exit_code(DBusError *error) {
  if (dbus_error_has_name(error, DBUS_ERROR_NO_MEMORY)) return BUS_SPAWN_EXIT_CODE_NO_MEMORY;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_CONFIG_INVALID)) return BUS_SPAWN_EXIT_CODE_CONFIG_INVALID;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_SETUP_FAILED)) return BUS_SPAWN_EXIT_CODE_SETUP_FAILED;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_SERVICE_INVALID)) return BUS_SPAWN_EXIT_CODE_NAME_INVALID;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_SERVICE_NOT_FOUND)) return BUS_SPAWN_EXIT_CODE_SERVICE_NOT_FOUND;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_PERMISSIONS_INVALID)) return BUS_SPAWN_EXIT_CODE_PERMISSIONS_INVALID;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_FILE_INVALID)) return BUS_SPAWN_EXIT_CODE_FILE_INVALID;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_EXEC_FAILED)) return BUS_SPAWN_EXIT_CODE_EXEC_FAILED;
  if (dbus_error_has_name(error, DBUS_ERROR_INVALID_ARGS)) return BUS_SPAWN_EXIT_CODE_INVALID_ARGS;
  if (dbus_error_has_name(error, DBUS_ERROR_SPAWN_CHILD_SIGNALED)) return BUS_SPAWN_EXIT_CODE_CHILD_SIGNALED;
  fprintf(stderr, "%s: %s\n", error->name, error->message);
  return BUS_SPAWN_EXIT_CODE_GENERIC_FAILURE;
}
int main(int argc, char **argv) {
  DBusError error;
  int retval;
  retval = 0;
  if (argc != 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0) {
      fprintf(stderr, "dbus-daemon-activation-helper service.to.activate\n");
      exit(0);
  }
  dbus_error_init(&error);
  if (!run_launch_helper(argv[1], &error)) {
      retval = convert_error_to_exit_code(&error);
      dbus_error_free(&error);
  }
  return retval;
}
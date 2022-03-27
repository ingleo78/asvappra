#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "../dbus-misc.h"
#include "../dbus-shell.h"
#include "../dbus-marshal-validate.h"
#include "bus.h"
#include "driver.h"
#include "utils.h"
#include "desktop-file.h"
#include "config-parser-trivial.h"
#include "activation-helper.h"
#include "activation-exit-codes.h"

static BusDesktopFile *desktop_file_for_name(BusConfigParser *parser, const char *name, DBusError *error) {
  BusDesktopFile *desktop_file;
  DBusList **service_dirs;
  DBusList *link;
  DBusError tmp_error;
  DBusString full_path;
  DBusString filename;
  const char *dir;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  desktop_file = NULL;
  if (!_dbus_string_init(&filename)) {
      BUS_SET_OOM(error);
      goto out_all;
  }
  if (!_dbus_string_init(&full_path)) {
      BUS_SET_OOM(error);
      goto out_filename;
  }
  if (!_dbus_string_append(&filename, name) || !_dbus_string_append(&filename, ".service")) {
      BUS_SET_OOM(error);
      goto out;
  }
  service_dirs = bus_config_parser_get_service_paths(parser);
  for (link = _dbus_list_get_first_link(service_dirs); link != NULL; link = _dbus_list_get_next_link(service_dirs, link)) {
      dir = link->data;
      _dbus_verbose("Looking at '%s'\n", dir);
      dbus_error_init(&tmp_error);
      _dbus_string_set_length(&full_path, 0);
      if (!_dbus_string_append(&full_path, dir) || !_dbus_concat_dir_and_file(&full_path, &filename)) {
          BUS_SET_OOM(error);
          goto out;
      }
      _dbus_verbose("Trying to load file '%s'\n", _dbus_string_get_data(&full_path));
      desktop_file = bus_desktop_file_load(&full_path, &tmp_error);
      if (desktop_file == NULL) {
          _DBUS_ASSERT_ERROR_IS_SET(&tmp_error);
          _dbus_verbose("Could not load %s: %s: %s\n", _dbus_string_get_const_data(&full_path), tmp_error.name, tmp_error.message);
          if (dbus_error_has_name(&tmp_error, DBUS_ERROR_NO_MEMORY)) {
              dbus_move_error(&tmp_error, error);
              goto out;
          }
          dbus_error_free(&tmp_error);
      }
      if (desktop_file != NULL) break;
  }
  if (desktop_file == NULL) dbus_set_error(error, DBUS_ERROR_SPAWN_SERVICE_NOT_FOUND,"The name %s was not provided by any .service files", name);
out:
  _dbus_string_free(&full_path);
out_filename:
  _dbus_string_free(&filename);
out_all:
  return desktop_file;
}
static dbus_bool_t clear_environment(DBusError *error) {
#ifndef ACTIVATION_LAUNCHER_TEST
  if (!_dbus_clearenv()) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_SETUP_FAILED,"could not clear environment\n");
      return FALSE;
  }
  dbus_setenv("DBUS_STARTER_ADDRESS", DBUS_SYSTEM_BUS_DEFAULT_ADDRESS);
  dbus_setenv("DBUS_STARTER_BUS_TYPE", "system");
#endif
  return TRUE;
}
static dbus_bool_t check_permissions(const char *dbus_user, DBusError *error) {
#ifndef ACTIVATION_LAUNCHER_TEST
  uid_t uid, euid;
  struct passwd *pw;
  pw = NULL;
  uid = 0;
  euid = 0;
  pw = getpwnam(dbus_user);
  if (!pw) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_PERMISSIONS_INVALID,"cannot find user '%s'", dbus_user);
      return FALSE;
  }
  uid = getuid();
  if (pw->pw_uid != uid) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_PERMISSIONS_INVALID,"not invoked from user '%s'", dbus_user);
      return FALSE;
  }
  euid = geteuid();
  if (euid != 0) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_PERMISSIONS_INVALID,"not setuid root");
      return FALSE;
  }
#endif
  return TRUE;
}
static dbus_bool_t check_service_name(BusDesktopFile *desktop_file, const char *service_name, DBusError *error) {
  char *name_tmp;
  dbus_bool_t retval;
  retval = FALSE;
  if (!bus_desktop_file_get_string(desktop_file, DBUS_SERVICE_SECTION, DBUS_SERVICE_NAME, &name_tmp, error)) goto failed;
  if (strcmp(service_name, name_tmp) != 0) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_FILE_INVALID,"Service '%s' does not match expected value", name_tmp);
      goto failed_free;
  }
  retval = TRUE;
failed_free:
  dbus_free(name_tmp);
failed:
  return retval;
}
static dbus_bool_t get_parameters_for_service(BusDesktopFile *desktop_file, const char *service_name, char **exec, char **user, DBusError *error) {
  char *exec_tmp;
  char *user_tmp;
  exec_tmp = NULL;
  user_tmp = NULL;
  if (!check_service_name(desktop_file, service_name, error)) goto failed;
  if (!bus_desktop_file_get_string(desktop_file, DBUS_SERVICE_SECTION, DBUS_SERVICE_EXEC, &exec_tmp, error)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      goto failed;
  }
  if (!bus_desktop_file_get_string(desktop_file, DBUS_SERVICE_SECTION, DBUS_SERVICE_USER, &user_tmp, error)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      goto failed;
  }
  *exec = exec_tmp;
  *user = user_tmp;
  return TRUE;
failed:
  dbus_free(exec_tmp);
  dbus_free(user_tmp);
  return FALSE;
}
static dbus_bool_t switch_user(char *user, DBusError *error) {
#ifndef ACTIVATION_LAUNCHER_TEST
  struct passwd *pw;
  pw = getpwnam(user);
  if (!pw) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_SETUP_FAILED,"cannot find user '%s'\n", user);
      return FALSE;
  }
  if (initgroups(user, pw->pw_gid)) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_SETUP_FAILED,"could not initialize groups");
      return FALSE;
  }
  if (setgid(pw->pw_gid)) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_SETUP_FAILED,"cannot setgid group %i", pw->pw_gid);
      return FALSE;
  }
  if (setuid(pw->pw_uid) < 0) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_SETUP_FAILED,"cannot setuid user %i", pw->pw_uid);
      return FALSE;
  }
#endif
  return TRUE;
}
static dbus_bool_t exec_for_correct_user(char *exec, char *user, DBusError *error) {
  char **argv;
  int argc;
  dbus_bool_t retval;
  argc = 0;
  retval = TRUE;
  argv = NULL;
  if (!switch_user(user, error)) return FALSE;
  if (!_dbus_shell_parse_argv(exec, &argc, &argv, error)) return FALSE;
#ifndef ACTIVATION_LAUNCHER_DO_OOM
  if (execv(argv[0], argv) < 0) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_EXEC_FAILED,"Failed to exec: %s", argv[0]);
      retval = FALSE;
  }
#endif
  dbus_free_string_array(argv);
  return retval;
}
static dbus_bool_t check_bus_name(const char *bus_name, DBusError *error) {
  DBusString str;
  _dbus_string_init_const(&str, bus_name);
  if (!_dbus_validate_bus_name(&str, 0, _dbus_string_get_length(&str))) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_SERVICE_INVALID,"bus name '%s' is not a valid bus name\n", bus_name);
      return FALSE;
  }
  return TRUE;
}
static dbus_bool_t get_correct_parser(BusConfigParser **parser, DBusError *error) {
  DBusString config_file;
  dbus_bool_t retval;
#ifdef ACTIVATION_LAUNCHER_TEST
  const char *test_config_file;
#endif
  retval = FALSE;
#ifdef ACTIVATION_LAUNCHER_TEST
  test_config_file = NULL;
  if (getuid() != geteuid()) _dbus_assert_not_reached("dbus-daemon-launch-helper-test binary is setuid!");
  test_config_file = _dbus_getenv("TEST_LAUNCH_HELPER_CONFIG");
  if (test_config_file == NULL) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_SETUP_FAILED, "the TEST_LAUNCH_HELPER_CONFIG env variable is not set");
      goto out;
  }
#endif
  if (!_dbus_string_init (&config_file)) {
      BUS_SET_OOM(error);
      goto out;
  }
#ifndef ACTIVATION_LAUNCHER_TEST
  if (!_dbus_string_append(&config_file, DBUS_SYSTEM_CONFIG_FILE)) {
      BUS_SET_OOM(error);
      goto out_free_config;
  }
#else
  if (!_dbus_string_append(&config_file, test_config_file)) {
      BUS_SET_OOM(error);
      goto out_free_config;
  }
#endif
  _dbus_verbose("dbus-daemon-activation-helper: using config file: %s\n", _dbus_string_get_const_data(&config_file));
  *parser = bus_config_load(&config_file, TRUE, NULL, error);
  if (*parser == NULL) goto out_free_config;
  retval = TRUE;
out_free_config:
  _dbus_string_free(&config_file);
out:
  return retval;
}
static dbus_bool_t launch_bus_name(const char *bus_name, BusConfigParser *parser, DBusError *error) {
  BusDesktopFile *desktop_file;
  char *exec, *user;
  dbus_bool_t retval;
  exec = NULL;
  user = NULL;
  retval = FALSE;
  desktop_file = desktop_file_for_name(parser, bus_name, error);
  if (desktop_file == NULL) return FALSE;
  if (!get_parameters_for_service(desktop_file, bus_name, &exec, &user, error)) goto finish;
  _dbus_verbose("dbus-daemon-activation-helper: Name='%s'\n", bus_name);
  _dbus_verbose("dbus-daemon-activation-helper: Exec='%s'\n", exec);
  _dbus_verbose("dbus-daemon-activation-helper: User='%s'\n", user);
  if (!exec_for_correct_user(exec, user, error)) goto finish;
  retval = TRUE;
finish:
  dbus_free(exec);
  dbus_free(user);
  bus_desktop_file_free(desktop_file);
  return retval;
}
static dbus_bool_t check_dbus_user(BusConfigParser *parser, DBusError *error) {
  const char *dbus_user;
  dbus_user = bus_config_parser_get_user(parser);
  if (dbus_user == NULL) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_CONFIG_INVALID,"could not get user from config file\n");
      return FALSE;
  }
  if (!check_permissions(dbus_user, error)) return FALSE;
  return TRUE;
}
dbus_bool_t run_launch_helper(const char *bus_name, DBusError *error) {
  BusConfigParser *parser;
  dbus_bool_t retval;
  parser = NULL;
  retval = FALSE;
  if (!clear_environment(error)) goto error;
  if (!check_bus_name(bus_name, error)) goto error;
  if (!get_correct_parser(&parser, error)) goto error;
  if (!check_dbus_user(parser, error)) goto error_free_parser;
  if (!launch_bus_name(bus_name, parser, error)) goto error_free_parser;
  retval = TRUE;
error_free_parser:
  bus_config_parser_unref(parser);
error:
  return retval;
}
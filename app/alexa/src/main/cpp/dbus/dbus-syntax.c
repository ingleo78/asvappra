#include "config.h"
#include "dbus-syntax.h"
#include "dbus-internals.h"
#include "dbus-marshal-validate.h"
#include "dbus-shared.h"

dbus_bool_t dbus_validate_path(const char *path, DBusError *error) {
  DBusString str;
  int len;
  _dbus_return_val_if_fail(path != NULL, FALSE);
  _dbus_string_init_const(&str, path);
  len = _dbus_string_get_length(&str);
  if (_DBUS_LIKELY(_dbus_validate_path(&str, 0, len))) return TRUE;
  if (!_dbus_string_validate_utf8(&str, 0, len)) {
      dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Object path was not valid UTF-8");
      return FALSE;
  }
  dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Object path was not valid: '%s'", path);
  return FALSE;
}
dbus_bool_t dbus_validate_interface(const char *name, DBusError *error) {
  DBusString str;
  int len;
  _dbus_return_val_if_fail(name != NULL, FALSE);
  _dbus_string_init_const(&str, name);
  len = _dbus_string_get_length(&str);
  if (_DBUS_LIKELY(_dbus_validate_interface (&str, 0, len))) return TRUE;
  if (!_dbus_string_validate_utf8(&str, 0, len)) {
      dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Interface name was not valid UTF-8");
      return FALSE;
  }
  dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Interface name was not valid: '%s'", name);
  return FALSE;
}
dbus_bool_t dbus_validate_member(const char *name, DBusError *error) {
  DBusString str;
  int len;
  _dbus_return_val_if_fail(name != NULL, FALSE);
  _dbus_string_init_const(&str, name);
  len = _dbus_string_get_length(&str);
  if (_DBUS_LIKELY(_dbus_validate_member(&str, 0, len))) return TRUE;
  if (!_dbus_string_validate_utf8(&str, 0, len)) {
      dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Member name was not valid UTF-8");
      return FALSE;
  }
  dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Member name was not valid: '%s'", name);
  return FALSE;
}
dbus_bool_t dbus_validate_error_name(const char *name, DBusError *error) {
  DBusString str;
  int len;
  _dbus_return_val_if_fail(name != NULL, FALSE);
  _dbus_string_init_const(&str, name);
  len = _dbus_string_get_length(&str);
  if (_DBUS_LIKELY(_dbus_validate_error_name(&str, 0, len))) return TRUE;
  if (!_dbus_string_validate_utf8(&str, 0, len)) {
      dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Error name was not valid UTF-8");
      return FALSE;
  }
  dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Error name was not valid: '%s'", name);
  return FALSE;
}
dbus_bool_t dbus_validate_bus_name(const char *name, DBusError *error) {
  DBusString str;
  int len;
  _dbus_return_val_if_fail(name != NULL, FALSE);
  _dbus_string_init_const(&str, name);
  len = _dbus_string_get_length(&str);
  if (_DBUS_LIKELY(_dbus_validate_bus_name(&str, 0, len))) return TRUE;
  if (!_dbus_string_validate_utf8(&str, 0, len)) {
      dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Bus name was not valid UTF-8");
      return FALSE;
  }
  dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Bus name was not valid: '%s'", name);
  return FALSE;
}
dbus_bool_t dbus_validate_utf8(const char *alleged_utf8, DBusError *error) {
  DBusString str;
  _dbus_return_val_if_fail(alleged_utf8 != NULL, FALSE);
  _dbus_string_init_const(&str, alleged_utf8);
  if (_DBUS_LIKELY(_dbus_string_validate_utf8(&str, 0, _dbus_string_get_length(&str)))) return TRUE;
  dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"String was not valid UTF-8");
  return FALSE;
}
#include <stdarg.h>
#include <string.h>
#include "config.h"
#include "dbus-errors.h"
#include "dbus-internals.h"
#include "dbus-string.h"
#include "dbus-protocol.h"

typedef struct {
  char *name;
  char *message;
  unsigned int const_message : 1;
  unsigned int dummy2 : 1;
  unsigned int dummy3 : 1;
  unsigned int dummy4 : 1;
  unsigned int dummy5 : 1;
  void *padding1;
} DBusRealError;
_DBUS_STATIC_ASSERT(sizeof(DBusRealError) == sizeof(DBusError));
static const char* message_from_error(const char *error) {
  if (strcmp(error, DBUS_ERROR_FAILED) == 0) return "Unknown error";
  else if (strcmp(error, DBUS_ERROR_NO_MEMORY) == 0) return "Not enough memory available";
  else if (strcmp(error, DBUS_ERROR_IO_ERROR) == 0) return "Error reading or writing data";
  else if (strcmp(error, DBUS_ERROR_BAD_ADDRESS) == 0) return "Could not parse address";
  else if (strcmp(error, DBUS_ERROR_NOT_SUPPORTED) == 0) return "Feature not supported";
  else if (strcmp(error, DBUS_ERROR_LIMITS_EXCEEDED) == 0) return "Resource limits exceeded";
  else if (strcmp(error, DBUS_ERROR_ACCESS_DENIED) == 0) return "Permission denied";
  else if (strcmp(error, DBUS_ERROR_AUTH_FAILED) == 0) return "Could not authenticate to server";
  else if (strcmp(error, DBUS_ERROR_NO_SERVER) == 0) return "No server available at address";
  else if (strcmp(error, DBUS_ERROR_TIMEOUT) == 0) return "Connection timed out";
  else if (strcmp(error, DBUS_ERROR_NO_NETWORK) == 0) return "Network unavailable";
  else if (strcmp(error, DBUS_ERROR_ADDRESS_IN_USE) == 0) return "Address already in use";
  else if (strcmp(error, DBUS_ERROR_DISCONNECTED) == 0) return "Disconnected.";
  else if (strcmp(error, DBUS_ERROR_INVALID_ARGS) == 0) return "Invalid arguments.";
  else if (strcmp(error, DBUS_ERROR_NO_REPLY) == 0) return "Did not get a reply message.";
  else if (strcmp(error, DBUS_ERROR_FILE_NOT_FOUND) == 0) return "File doesn't exist.";
  else if (strcmp(error, DBUS_ERROR_OBJECT_PATH_IN_USE) == 0) return "Object path already in use";
  else return error;
}
void dbus_error_init(DBusError *error) {
  DBusRealError *real;
  _DBUS_STATIC_ASSERT(sizeof(DBusError) == sizeof(DBusRealError));
  _dbus_return_if_fail(error != NULL);
  real = (DBusRealError*)error;
  real->name = NULL;  
  real->message = NULL;
  real->const_message = TRUE;
}
void dbus_error_free(DBusError *error) {
  DBusRealError *real;
  _dbus_return_if_fail(error != NULL);
  real = (DBusRealError*)error;
  if (!real->const_message) {
      dbus_free(real->name);
      dbus_free(real->message);
  }
  dbus_error_init(error);
}
void dbus_set_error_const(DBusError *error, const char *name, const char *message) {
  DBusRealError *real;
  _dbus_return_if_error_is_set(error);
  _dbus_return_if_fail(name != NULL);
  if (error == NULL) return;
  _dbus_assert(error->name == NULL);
  _dbus_assert(error->message == NULL);
  if (message == NULL) message = message_from_error(name);
  real = (DBusRealError*)error;
  real->name = (char*)name;
  real->message = (char*)message;
  real->const_message = TRUE;
}
void dbus_move_error(DBusError *src, DBusError *dest) {
  _dbus_return_if_error_is_set(dest);
  if (dest) {
      dbus_error_free(dest);
      *dest = *src;
      dbus_error_init(src);
  } else dbus_error_free(src);
}
dbus_bool_t dbus_error_has_name(const DBusError *error, const char *name) {
  _dbus_return_val_if_fail(error != NULL, FALSE);
  _dbus_return_val_if_fail(name != NULL, FALSE);
  _dbus_assert((error->name != NULL && error->message != NULL) || (error->name == NULL && error->message == NULL));
  if (error->name != NULL) {
      DBusString str1, str2;
      _dbus_string_init_const(&str1, error->name);
      _dbus_string_init_const(&str2, name);
      return _dbus_string_equal(&str1, &str2);
  } else return FALSE;
}
dbus_bool_t dbus_error_is_set(const DBusError *error) {
  _dbus_return_val_if_fail(error != NULL, FALSE);
  _dbus_assert((error->name != NULL && error->message != NULL) || (error->name == NULL && error->message == NULL));
  return error->name != NULL;
}
void dbus_set_error(DBusError *error, const char *name, const char *format, ...) {
  va_list args;
  if (error == NULL) return;
  va_start(args, format);
  _dbus_set_error_valist(error, name, format, args);
  va_end(args);
}
void _dbus_set_error_valist(DBusError *error, const char *name, const char *format, va_list args) {
  DBusRealError *real;
  DBusString str;
  _dbus_assert(name != NULL);
  if (error == NULL) return;
  _dbus_assert(error->name == NULL);
  _dbus_assert(error->message == NULL);
  if (!_dbus_string_init(&str)) goto nomem;
  if (format == NULL) {
      if (!_dbus_string_append(&str, message_from_error (name))) {
          _dbus_string_free(&str);
          goto nomem;
      }
  } else {
      if (!_dbus_string_append_printf_valist(&str, format, args)) {
          _dbus_string_free(&str);
          goto nomem;
      }
  }
  real = (DBusRealError*)error;
  if (!_dbus_string_steal_data(&str, &real->message)) {
      _dbus_string_free(&str);
      goto nomem;
  }
  _dbus_string_free(&str);
  real->name = _dbus_strdup(name);
  if (real->name == NULL) {
      dbus_free(real->message);
      real->message = NULL;
      goto nomem;
  }
  real->const_message = FALSE;
  return;
nomem:
  _DBUS_SET_OOM(error);
}
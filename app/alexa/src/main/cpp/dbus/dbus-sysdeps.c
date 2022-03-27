#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-sysdeps.h"
#include "dbus-threads.h"
#include "dbus-protocol.h"
#include "dbus-string.h"
#include "dbus-list.h"
#include "dbus-misc.h"

extern char **environ;
void _dbus_abort(void) {
  const char *s;
  _dbus_print_backtrace();
  s = _dbus_getenv("DBUS_BLOCK_ON_ABORT");
  if (s && *s) {
      fprintf(stderr, "  Process %lu sleeping for gdb attach\n", _dbus_pid_for_log());
      _dbus_sleep_milliseconds(1000 * 180);
  }
  abort();
  _dbus_exit(1);
}
dbus_bool_t dbus_setenv(const char *varname, const char *value) {
  _dbus_assert(varname != NULL);
  if (value == NULL) {
  #ifdef HAVE_UNSETENV
      unsetenv(varname);
      return TRUE;
  #else
      char *putenv_value;
      size_t len;
      len = strlen(varname);
      putenv_value = malloc(len + 2);
      if (putenv_value == NULL) return FALSE;
      strcpy(putenv_value, varname);
  #if defined(DBUS_WIN)
      strcat(putenv_value, "=");
  #endif
      return(putenv(putenv_value) == 0);
#endif
  } else {
  #ifdef HAVE_SETENV
      return (setenv(varname, value, TRUE) == 0);
  #else
      char *putenv_value;
      size_t len;
      size_t varname_len;
      size_t value_len;
      varname_len = strlen(varname);
      value_len = strlen(value);
      len = varname_len + value_len + 1 /* '=' */ ;
      putenv_value = malloc(len + 1);
      if (putenv_value == NULL) return FALSE;
      strcpy(putenv_value, varname);
      strcpy(putenv_value + varname_len, "=");
      strcpy(putenv_value + varname_len + 1, value);
      return(putenv(putenv_value) == 0);
  #endif
    }
}
const char* _dbus_getenv(const char *varname) {
  if (_dbus_check_setuid()) return NULL;
  return getenv(varname);
}
dbus_bool_t _dbus_clearenv(void) {
  dbus_bool_t rc = TRUE;
#ifdef HAVE_CLEARENV
  if (clearenv() != 0) rc = FALSE;
#else
  if (environ != NULL) environ[0] = NULL;
#endif
  return rc;
}
dbus_bool_t _dbus_split_paths_and_append(DBusString *dirs, const char *suffix, DBusList **dir_list) {
  int start;
  int i;
  int len;
  char *cpath;
  DBusString file_suffix;
  start = 0;
  i = 0;
  _dbus_string_init_const(&file_suffix, suffix);
  len = _dbus_string_get_length(dirs);
  while(_dbus_string_find(dirs, start, _DBUS_PATH_SEPARATOR, &i)) {
      DBusString path;
      if (!_dbus_string_init(&path)) goto oom;
      if (!_dbus_string_copy_len(dirs, start,i - start, &path,0)) {
           _dbus_string_free(&path);
           goto oom;
      }
       _dbus_string_chop_white(&path);
       if (_dbus_string_get_length(&path) == 0) goto next;
       if (!_dbus_concat_dir_and_file(&path, &file_suffix)) {
           _dbus_string_free(&path);
           goto oom;
       }
       if (!_dbus_string_copy_data(&path, &cpath)) {
           _dbus_string_free(&path);
           goto oom;
       }
       if (!_dbus_list_append(dir_list, cpath)) {
           _dbus_string_free(&path);
           dbus_free(cpath);
           goto oom;
       }
  next:
       _dbus_string_free(&path);
       start = i + 1;
  }
  if (start != len) {
      DBusString path;
      if (!_dbus_string_init(&path)) goto oom;
      if (!_dbus_string_copy_len(dirs, start,len - start, &path,0)) {
          _dbus_string_free(&path);
          goto oom;
      }
      if (!_dbus_concat_dir_and_file(&path, &file_suffix)) {
          _dbus_string_free(&path);
          goto oom;
      }
      if (!_dbus_string_copy_data(&path, &cpath)) {
          _dbus_string_free(&path);
          goto oom;
      }
      if (!_dbus_list_append(dir_list, cpath)) {
          _dbus_string_free(&path);
          dbus_free(cpath);
          goto oom;
      }
      _dbus_string_free(&path);
  }
  return TRUE;
oom:
  _dbus_list_foreach(dir_list, (DBusForeachFunction)dbus_free, NULL);
  _dbus_list_clear(dir_list);
  return FALSE;
}
dbus_bool_t _dbus_string_append_int(DBusString *str, long value) {
  #define MAX_LONG_LEN  ((sizeof(long) * 8 + 2) / 3 + 1)
  int orig_len;
  int i;
  char *buf;
  orig_len = _dbus_string_get_length(str);
  if (!_dbus_string_lengthen(str, MAX_LONG_LEN)) return FALSE;
  buf = _dbus_string_get_data_len(str, orig_len, MAX_LONG_LEN);
  snprintf(buf, MAX_LONG_LEN, "%ld", value);
  i = 0;
  while(*buf) {
      ++buf;
      ++i;
  }
  _dbus_string_shorten(str, MAX_LONG_LEN - i);
  return TRUE;
}
dbus_bool_t _dbus_string_append_uint(DBusString *str, unsigned long value) {
  #define MAX_ULONG_LEN  (MAX_LONG_LEN * 2)
  int orig_len;
  int i;
  char *buf;
  orig_len = _dbus_string_get_length(str);
  if (!_dbus_string_lengthen(str, MAX_ULONG_LEN)) return FALSE;
  buf = _dbus_string_get_data_len(str, orig_len, MAX_ULONG_LEN);
  snprintf(buf, MAX_ULONG_LEN, "%lu", value);
  i = 0;
  while(*buf) {
      ++buf;
      ++i;
  }
  _dbus_string_shorten(str, MAX_ULONG_LEN - i);
  return TRUE;
}
dbus_bool_t _dbus_string_parse_int(const DBusString *str, int start, long *value_return, int *end_return) {
  long v;
  const char *p;
  char *end;
  p = _dbus_string_get_const_data_len(str, start,_dbus_string_get_length(str) - start);
  end = NULL;
  _dbus_set_errno_to_zero();
  v = strtol(p, &end, 0);
  if (end == NULL || end == p || errno != 0) return FALSE;
  if (value_return) *value_return = v;
  if (end_return) *end_return = start + (end - p);
  return TRUE;
}
dbus_bool_t _dbus_string_parse_uint(const DBusString *str, int start, unsigned long *value_return, int *end_return) {
  unsigned long v;
  const char *p;
  char *end;
  p = _dbus_string_get_const_data_len(str, start,_dbus_string_get_length(str) - start);
  end = NULL;
  _dbus_set_errno_to_zero();
  v = strtoul(p, &end, 0);
  if (end == NULL || end == p || errno != 0) return FALSE;
  if (value_return) *value_return = v;
  if (end_return) *end_return = start + (end - p);
  return TRUE;
}
dbus_bool_t _dbus_generate_random_bytes_buffer(char *buffer, int n_bytes, DBusError *error) {
  DBusString str;
  if (!_dbus_string_init(&str)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_generate_random_bytes(&str, n_bytes, error)) {
      _dbus_string_free(&str);
      return FALSE;
  }
  _dbus_string_copy_to_buffer(&str, buffer, n_bytes);
  _dbus_string_free(&str);
  return TRUE;
}
dbus_bool_t _dbus_generate_random_ascii(DBusString *str, int n_bytes, DBusError *error) {
  static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";
  int i;
  int len;
  if (!_dbus_generate_random_bytes(str, n_bytes, error)) return FALSE;
  len = _dbus_string_get_length(str);
  i = len - n_bytes;
  while(i < len) {
      _dbus_string_set_byte(str, i,letters[_dbus_string_get_byte(str, i) % (sizeof(letters) - 1)]);
      ++i;
  }
  _dbus_assert(_dbus_string_validate_ascii(str, len - n_bytes, n_bytes));
  return TRUE;
}
const char* _dbus_error_from_errno(int error_number) {
  switch(error_number) {
      case 0: return DBUS_ERROR_FAILED;
  #ifdef EPROTONOSUPPORT
      case EPROTONOSUPPORT: return DBUS_ERROR_NOT_SUPPORTED;
  #elif defined(WSAEPROTONOSUPPORT)
      case WSAEPROTONOSUPPORT: return DBUS_ERROR_NOT_SUPPORTED;
  #endif
  #ifdef EAFNOSUPPORT
      case EAFNOSUPPORT: return DBUS_ERROR_NOT_SUPPORTED;
  #elif defined(WSAEAFNOSUPPORT)
      case WSAEAFNOSUPPORT: return DBUS_ERROR_NOT_SUPPORTED;
  #endif
  #ifdef ENFILE
      case ENFILE: return DBUS_ERROR_LIMITS_EXCEEDED;
  #endif
  #ifdef EMFILE
      case EMFILE: return DBUS_ERROR_LIMITS_EXCEEDED;
  #endif
  #ifdef EACCES
      case EACCES: return DBUS_ERROR_ACCESS_DENIED;
  #endif
  #ifdef EPERM
      case EPERM: return DBUS_ERROR_ACCESS_DENIED;
  #endif
  #ifdef ENOBUFS
      case ENOBUFS: return DBUS_ERROR_NO_MEMORY;
  #endif
  #ifdef ENOMEM
      case ENOMEM: return DBUS_ERROR_NO_MEMORY;
  #endif
  #ifdef ECONNREFUSED
      case ECONNREFUSED: return DBUS_ERROR_NO_SERVER;
  #elif defined(WSAECONNREFUSED)
      case WSAECONNREFUSED: return DBUS_ERROR_NO_SERVER;
  #endif
  #ifdef ETIMEDOUT
      case ETIMEDOUT: return DBUS_ERROR_TIMEOUT;
  #elif defined(WSAETIMEDOUT)
      case WSAETIMEDOUT: return DBUS_ERROR_TIMEOUT;
  #endif
  #ifdef ENETUNREACH
      case ENETUNREACH: return DBUS_ERROR_NO_NETWORK;
  #elif defined(WSAENETUNREACH)
      case WSAENETUNREACH: return DBUS_ERROR_NO_NETWORK;
  #endif
  #ifdef EADDRINUSE
      case EADDRINUSE: return DBUS_ERROR_ADDRESS_IN_USE;
  #elif defined(WSAEADDRINUSE)
      case WSAEADDRINUSE: return DBUS_ERROR_ADDRESS_IN_USE;
  #endif
  #ifdef EEXIST
      case EEXIST: return DBUS_ERROR_FILE_EXISTS;
  #endif
  #ifdef ENOENT
      case ENOENT: return DBUS_ERROR_FILE_NOT_FOUND;
  #endif
      default: return DBUS_ERROR_FAILED;
  }
}
const char* _dbus_error_from_system_errno(void) {
  return _dbus_error_from_errno(errno);
}
void _dbus_set_errno_to_zero(void) {
#ifdef DBUS_WINCE
  SetLastError(0);
#else
  errno = 0;
#endif
}
dbus_bool_t _dbus_get_is_errno_enomem(int e) {
  return e == ENOMEM;
}
dbus_bool_t _dbus_get_is_errno_eintr(int e) {
  return e == EINTR;
}
dbus_bool_t _dbus_get_is_errno_epipe(int e) {
  return e == EPIPE;
}
dbus_bool_t _dbus_get_is_errno_etoomanyrefs(int e) {
#ifdef ETOOMANYREFS
  return e == ETOOMANYREFS;
#else
  return FALSE;
#endif
}
const char* _dbus_strerror_from_errno(void) {
  return _dbus_strerror(errno);
}
void _dbus_log(DBusSystemLogSeverity severity, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  _dbus_logv(severity, msg, args);
  va_end(args);
}
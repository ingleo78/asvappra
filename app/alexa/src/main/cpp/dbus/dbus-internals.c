#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-protocol.h"
#include "dbus-marshal-basic.h"
#include "dbus-test.h"
#include "dbus-test-tap.h"
#include "dbus-valgrind-internal.h"
#ifdef DBUS_USE_OUTPUT_DEBUG_STRING
#include <windows.h>
#include <mbstring.h>
#endif

const char *_dbus_no_memory_message = "Not enough memory";
static dbus_bool_t warn_initted = FALSE;
static dbus_bool_t fatal_warnings = FALSE;
static dbus_bool_t fatal_warnings_on_check_failed = TRUE;
static void init_warnings(void) {
  if (!warn_initted) {
      const char *s;
      s = _dbus_getenv("DBUS_FATAL_WARNINGS");
      if (s && *s) {
          if (*s == '0') {
              fatal_warnings = FALSE;
              fatal_warnings_on_check_failed = FALSE;
          } else if (*s == '1') {
              fatal_warnings = TRUE;
              fatal_warnings_on_check_failed = TRUE;
          } else fprintf(stderr, "DBUS_FATAL_WARNINGS should be set to 0 or 1 if set, not '%s'", s);
      }
      warn_initted = TRUE;
  }
}
void _dbus_warn(const char *format, ...) {
  DBusSystemLogSeverity severity = DBUS_SYSTEM_LOG_WARNING;
  va_list args;
  if (!warn_initted) init_warnings();
  if (fatal_warnings) severity = DBUS_SYSTEM_LOG_ERROR;
  va_start(args, format);
  _dbus_logv(severity, format, args);
  va_end(args);
  if (fatal_warnings) {
      fflush(stderr);
      _dbus_abort();
  }
}
void _dbus_warn_check_failed(const char *format, ...) {
  DBusSystemLogSeverity severity = DBUS_SYSTEM_LOG_WARNING;
  va_list args;
  if (!warn_initted) init_warnings();
  if (fatal_warnings_on_check_failed) severity = DBUS_SYSTEM_LOG_ERROR;
  va_start (args, format);
  _dbus_logv (severity, format, args);
  va_end (args);
  if (fatal_warnings_on_check_failed) {
      fflush (stderr);
      _dbus_abort();
  }
}
#ifdef DBUS_ENABLE_VERBOSE_MODE
static dbus_bool_t verbose_initted = FALSE;
static dbus_bool_t verbose = TRUE;
#ifdef DBUS_USE_OUTPUT_DEBUG_STRING
static char module_name[1024];
#endif
static inline void _dbus_verbose_init(void) {
  if (!verbose_initted) {
      const char *p = _dbus_getenv("DBUS_VERBOSE");
      verbose = p != NULL && *p == '1';
      verbose_initted = TRUE;
  #ifdef DBUS_USE_OUTPUT_DEBUG_STRING
      {
          char *last_period, *last_slash;
          GetModuleFileName(0,module_name,sizeof(module_name)-1);
          last_period = _mbsrchr(module_name,'.');
          if (last_period) *last_period ='\0';
          last_slash = _mbsrchr(module_name,'\\');
          if (last_slash) strcpy(module_name,last_slash+1);
          strcat(module_name,": ");
      }
  #endif
  }
}
#ifdef DBUS_WIN 
#define DBUS_IS_DIR_SEPARATOR(c) (c == '\\' || c == '/')
#else
#define DBUS_IS_DIR_SEPARATOR(c) (c == '/')
#endif
static char *_dbus_file_path_extract_elements_from_tail(const char *file,int level) {
  int prefix = 0;
  char *p = (char *)file + strlen(file);
  int i = 0;
  for (;p >= file;p--) {
      if (DBUS_IS_DIR_SEPARATOR(*p)) {
          if (++i >= level) {
              prefix = p-file+1;
              break;
          }
      }
  }
  return (char*)file+prefix;
}
dbus_bool_t _dbus_is_verbose_real(void) {
  _dbus_verbose_init();
  return verbose;
}
void _dbus_set_verbose(dbus_bool_t state) {
    verbose = state;
}
dbus_bool_t _dbus_get_verbose(void) {
    return verbose;
}
void _dbus_verbose_real(
                    #ifdef DBUS_CPP_SUPPORTS_VARIABLE_MACRO_ARGUMENTS
                        const char *file, const int line, const char *function,
                    #endif
                        const char *format, ...) {
  va_list args;
  static dbus_bool_t need_pid = TRUE;
  int len;
  long sec, usec;
  if (!_dbus_is_verbose_real()) return;
#ifndef DBUS_USE_OUTPUT_DEBUG_STRING
  if (need_pid) _dbus_print_thread();
  _dbus_get_real_time(&sec, &usec);
  fprintf(stderr, "%ld.%06ld ", sec, usec);
#endif
  len = strlen(format);
  if (format[len-1] == '\n') need_pid = TRUE;
  else need_pid = FALSE;
  va_start(args, format);
#ifdef DBUS_USE_OUTPUT_DEBUG_STRING
  {
      char buf[1024];
      strcpy(buf,module_name);
  #ifdef DBUS_CPP_SUPPORTS_VARIABLE_MACRO_ARGUMENTS
      sprintf(buf+strlen(buf), "[%s(%d):%s] ",_dbus_file_path_extract_elements_from_tail(file,2),line,function);
  #endif
      vsprintf(buf+strlen(buf),format, args);
      va_end(args);
      OutputDebugStringA(buf);
  }
#else
#ifdef DBUS_CPP_SUPPORTS_VARIABLE_MACRO_ARGUMENTS
  fprintf(stderr, "[%s(%d):%s] ",_dbus_file_path_extract_elements_from_tail(file,2),line,function);
#endif
  vfprintf(stderr, format, args);
  va_end(args);
  fflush(stderr);
#endif
}
void _dbus_verbose_reset_real(void) {
  verbose_initted = FALSE;
}
void _dbus_trace_ref(const char *obj_name, void *obj, int old_refcount, int new_refcount, const char *why, const char *env_var, int *enabled) {
  _dbus_assert(obj_name != NULL);
  _dbus_assert(obj != NULL);
  _dbus_assert(old_refcount >= -1);
  _dbus_assert(new_refcount >= -1);
  if (old_refcount == -1) { _dbus_assert(new_refcount == -1); }
  else {
      _dbus_assert(new_refcount >= 0);
      _dbus_assert(old_refcount >= 0);
      _dbus_assert(old_refcount > 0 || new_refcount > 0);
  }
  _dbus_assert(why != NULL);
  _dbus_assert(env_var != NULL);
  _dbus_assert(enabled != NULL);
  if (*enabled < 0) {
      const char *s = _dbus_getenv(env_var);
      *enabled = FALSE;
      if (s && *s) {
          if (*s == '0') *enabled = FALSE;
          else if (*s == '1') *enabled = TRUE;
          else _dbus_warn("%s should be 0 or 1 if set, not '%s'", env_var, s);
      }
  }
  if (*enabled) {
      if (old_refcount == -1) {
          VALGRIND_PRINTF_BACKTRACE("%s %p ref stolen (%s)", obj_name, obj, why);
          _dbus_verbose("%s %p ref stolen (%s)\n", obj_name, obj, why);
      } else {
          VALGRIND_PRINTF_BACKTRACE("%s %p %d -> %d refs (%s)", obj_name, obj, old_refcount, new_refcount, why);
          _dbus_verbose("%s %p %d -> %d refs (%s)\n", obj_name, obj, old_refcount, new_refcount, why);
      }
  }
}
#endif
char* _dbus_strdup(const char *str) {
  size_t len;
  char *copy;
  if (str == NULL) return NULL;
  len = strlen (str);
  copy = dbus_malloc(len + 1);
  if (copy == NULL) return NULL;
  memcpy(copy, str, len + 1);
  return copy;
}
void* _dbus_memdup(const void *mem, size_t n_bytes) {
  void *copy;
  copy = dbus_malloc(n_bytes);
  if (copy == NULL) return NULL;
  memcpy(copy, mem, n_bytes);
  return copy;
}
char** _dbus_dup_string_array(const char **array) {
  int len;
  int i;
  char **copy;
  if (array == NULL) return NULL;
  for (len = 0; array[len] != NULL; ++len);
  copy = dbus_new0(char*, len + 1);
  if (copy == NULL) return NULL;
  i = 0;
  while(i < len) {
      copy[i] = _dbus_strdup(array[i]);
      if (copy[i] == NULL) {
          dbus_free_string_array(copy);
          return NULL;
      }
      ++i;
  }
  return copy;
}
dbus_bool_t _dbus_string_array_contains(const char **array, const char *str) {
  int i;
  i = 0;
  while(array[i] != NULL) {
      if (strcmp(array[i], str) == 0) return TRUE;
      ++i;
  }
  return FALSE;
}
size_t _dbus_string_array_length(const char **array) {
  size_t i;
  for (i = 0; array[i]; i++);
  return i;
}
dbus_bool_t _dbus_generate_uuid(DBusGUID *uuid, DBusError *error) {
  DBusError rand_error;
  long now;
  dbus_error_init(&rand_error);
  _dbus_get_real_time(&now, NULL);
  uuid->as_uint32s[DBUS_UUID_LENGTH_WORDS - 1] = DBUS_UINT32_TO_BE(now);
  if (!_dbus_generate_random_bytes_buffer(uuid->as_bytes,DBUS_UUID_LENGTH_BYTES - 4, &rand_error)) {
      dbus_set_error(error, rand_error.name,"Failed to generate UUID: %s", rand_error.message);
      dbus_error_free(&rand_error);
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_uuid_encode(const DBusGUID *uuid, DBusString *encoded) {
  DBusString binary;
  _dbus_string_init_const_len(&binary, uuid->as_bytes, DBUS_UUID_LENGTH_BYTES);
  return _dbus_string_hex_encode(&binary, 0, encoded, _dbus_string_get_length(encoded));
}
static dbus_bool_t _dbus_read_uuid_file_without_creating(const DBusString *filename, DBusGUID *uuid, DBusError *error) {
  DBusString contents;
  DBusString decoded;
  int end;
  if (!_dbus_string_init(&contents)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_string_init(&decoded)) {
      _dbus_string_free(&contents);
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_file_get_contents(&contents, filename, error)) goto error;
  _dbus_string_chop_white(&contents);
  if (_dbus_string_get_length(&contents) != DBUS_UUID_LENGTH_HEX) {
      dbus_set_error(error, DBUS_ERROR_INVALID_FILE_CONTENT,"UUID file '%s' should contain a hex string of length %d, not length %d, with no other text",
                     _dbus_string_get_const_data(filename), DBUS_UUID_LENGTH_HEX, _dbus_string_get_length(&contents));
      goto error;
  }
  if (!_dbus_string_hex_decode(&contents, 0, &end, &decoded, 0)) {
      _DBUS_SET_OOM(error);
      goto error;
  }
  if (end == 0) {
      dbus_set_error(error, DBUS_ERROR_INVALID_FILE_CONTENT,"UUID file '%s' contains invalid hex data", _dbus_string_get_const_data(filename));
      goto error;
  }
  if (_dbus_string_get_length(&decoded) != DBUS_UUID_LENGTH_BYTES) {
      dbus_set_error(error, DBUS_ERROR_INVALID_FILE_CONTENT,"UUID file '%s' contains %d bytes of hex-encoded data instead of %d",
                     _dbus_string_get_const_data(filename), _dbus_string_get_length(&decoded), DBUS_UUID_LENGTH_BYTES);
      goto error;
  }
  _dbus_string_copy_to_buffer(&decoded, uuid->as_bytes, DBUS_UUID_LENGTH_BYTES);
  _dbus_string_free(&decoded);
  _dbus_string_free(&contents);
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  return TRUE;
error:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  _dbus_string_free(&contents);
  _dbus_string_free(&decoded);
  return FALSE;
}
dbus_bool_t _dbus_write_uuid_file(const DBusString *filename, const DBusGUID *uuid, DBusError *error) {
  DBusString encoded;
  if (!_dbus_string_init(&encoded)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_uuid_encode(uuid, &encoded)) {
      _DBUS_SET_OOM(error);
      goto error;
  }
  if (!_dbus_string_append_byte(&encoded, '\n')) {
      _DBUS_SET_OOM(error);
      goto error;
  }
  if (!_dbus_string_save_to_file(&encoded, filename, TRUE, error)) goto error;
  _dbus_string_free(&encoded);
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  return TRUE;
error:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  _dbus_string_free(&encoded);
  return FALSE;        
}
dbus_bool_t _dbus_read_uuid_file(const DBusString *filename, DBusGUID *uuid, dbus_bool_t create_if_not_found, DBusError *error) {
  DBusError read_error = DBUS_ERROR_INIT;
  if (_dbus_read_uuid_file_without_creating(filename, uuid, &read_error)) return TRUE;
  if (!create_if_not_found) {
      dbus_move_error(&read_error, error);
      return FALSE;
  }
  if (dbus_error_has_name(&read_error, DBUS_ERROR_INVALID_FILE_CONTENT)) {
      dbus_move_error(&read_error, error);
      return FALSE;
  } else {
      dbus_error_free(&read_error);
      if (!_dbus_generate_uuid(uuid, error)) return FALSE;
      return _dbus_write_uuid_file(filename, uuid, error);
  }
}
static int machine_uuid_initialized_generation = 0;
static DBusGUID machine_uuid;
dbus_bool_t _dbus_get_local_machine_uuid_encoded(DBusString *uuid_str, DBusError *error) {
  dbus_bool_t ok = TRUE;
  if (!_DBUS_LOCK(machine_uuid)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (machine_uuid_initialized_generation != _dbus_current_generation) {
      if (!_dbus_read_local_machine_uuid(&machine_uuid, FALSE, error)) ok = FALSE;
  }
  if (ok) {
      if (!_dbus_uuid_encode(&machine_uuid, uuid_str)) {
          ok = FALSE;
          _DBUS_SET_OOM(error);
      }
  }
  _DBUS_UNLOCK(machine_uuid);
  return ok;
}
#ifdef DBUS_DISABLE_CHECKS
void _dbus_warn_return_if_fail(const char *function, const char *assertion, const char *file, int line) {
  _dbus_warn_check_failed("arguments to %s() were incorrect, assertion \"%s\" failed in file %s line %d.\nThis is normally a bug in some "
                          "application using the D-Bus library.\n", function, assertion, file, line);
}
#endif
#ifdef DBUS_DISABLE_ASSERT
void _dbus_real_assert(dbus_bool_t condition, const char *condition_text, const char *file, int line, const char *func) {
  if (_DBUS_UNLIKELY(!condition)) {
      _dbus_warn("assertion failed \"%s\" file \"%s\" line %d function %s", condition_text, file, line, func);
      _dbus_abort();
  }
}
void _dbus_real_assert_not_reached (const char *explanation, const char *file, int line) {
  _dbus_warn("File \"%s\" line %d should not have been reached: %s", file, line, explanation);
  _dbus_abort();
}
#endif
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
static dbus_bool_t run_failing_each_malloc(int n_mallocs, const char *description, DBusTestMemoryFunction func, void *data) {
  n_mallocs += 10;
  while(n_mallocs >= 0) {
      _dbus_set_fail_alloc_counter(n_mallocs);
      _dbus_verbose("\n===\n%s: (will fail malloc %d with %d failures)\n===\n", description, n_mallocs, _dbus_get_fail_alloc_failures());
      if (!(*func)(data, FALSE)) return FALSE;
      n_mallocs -= 1;
  }
  _dbus_set_fail_alloc_counter(_DBUS_INT_MAX);
  return TRUE;
}
dbus_bool_t _dbus_test_oom_handling(const char *description,DBusTestMemoryFunction func, void *data) {
  int approx_mallocs;
  const char *setting;
  int max_failures_to_try;
  int i;
  _dbus_set_fail_alloc_counter(_DBUS_INT_MAX);
  _dbus_test_diag("Running \"%s\" once to count mallocs", description);
  if (!(*func)(data, TRUE)) return FALSE;
  approx_mallocs = _DBUS_INT_MAX - _dbus_get_fail_alloc_counter();
  _dbus_test_diag("\"%s\" has about %d mallocs in total", description, approx_mallocs);
  setting = _dbus_getenv("DBUS_TEST_MALLOC_FAILURES");
  if (setting != NULL) {
      DBusString str;
      long v;
      _dbus_string_init_const(&str, setting);
      v = 4;
      if (!_dbus_string_parse_int(&str, 0, &v, NULL)) _dbus_warn("couldn't parse '%s' as integer\n", setting);
      max_failures_to_try = v;
  } else max_failures_to_try = 4;
  if (max_failures_to_try < 1) {
      _dbus_test_diag("not testing OOM handling");
      return TRUE;
  }
  _dbus_test_diag("testing \"%s\" with up to %d consecutive malloc failures", description, max_failures_to_try);
  i = setting ? max_failures_to_try - 1 : 1;
  while(i < max_failures_to_try) {
      _dbus_test_diag("testing \"%s\" with %d consecutive malloc failures", description, i + 1);
      _dbus_set_fail_alloc_failures(i);
      if (!run_failing_each_malloc(approx_mallocs, description, func, data)) return FALSE;
      ++i;
  }
  _dbus_verbose("\"%s\" coped OK with malloc failures", description);
  return TRUE;
}
#endif
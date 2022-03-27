#include <string.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-marshal-recursive.h"
#include "dbus-marshal-validate.h"
#include "dbus-marshal-byteswap.h"
#include "dbus-marshal-header.h"
#include "dbus-signature.h"
#include "dbus-message-private.h"
#include "dbus-object-tree.h"
#include "dbus-memory.h"
#include "dbus-list.h"
#include "dbus-threads-internal.h"
#ifdef HAVE_UNIX_FD_PASSING
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-unix.h"
#endif

#define _DBUS_TYPE_IS_STRINGLIKE(type)  (type == DBUS_TYPE_STRING || type == DBUS_TYPE_SIGNATURE || type == DBUS_TYPE_OBJECT_PATH)
static void dbus_message_finalize(DBusMessage *message);
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
static dbus_bool_t _dbus_enable_message_cache(void) {
  static int enabled = -1;
  if (enabled < 0) {
      const char *s = _dbus_getenv("DBUS_MESSAGE_CACHE");
      enabled = TRUE;
      if (s && *s) {
          if (*s == '0') enabled = FALSE;
          else if (*s == '1') enabled = TRUE;
          else _dbus_warn("DBUS_MESSAGE_CACHE should be 0 or 1 if set, not '%s'", s);
      }
  }
  return enabled;
}
#else
#define _dbus_enable_message_cache()  (TRUE)
#endif
#ifndef _dbus_message_trace_ref
void _dbus_message_trace_ref(DBusMessage *message, int old_refcount, int new_refcount, const char *why) {
  static int enabled = -1;
  _dbus_trace_ref("DBusMessage", message, old_refcount, new_refcount, why,"DBUS_MESSAGE_TRACE", &enabled);
}
#endif
_DBUS_STRING_DEFINE_STATIC(_dbus_empty_signature_str,  "");
enum {
  DBUS_MESSAGE_ITER_TYPE_READER = 3,
  DBUS_MESSAGE_ITER_TYPE_WRITER = 7
};
typedef struct DBusMessageRealIter DBusMessageRealIter;
struct DBusMessageRealIter {
  DBusMessage *message;
  dbus_uint32_t changed_stamp : CHANGED_STAMP_BITS;
  dbus_uint32_t iter_type : 3;
  dbus_uint32_t sig_refcount : 8;
  union {
      DBusTypeWriter writer;
      DBusTypeReader reader;
  } u;
};
typedef struct {
  void *dummy1;
  void *dummy2;
  dbus_uint32_t dummy3;
  int dummy4;
  int dummy5;
  int dummy6;
  int dummy7;
  int dummy8;
  int dummy9;
  int dummy10;
  int dummy11;
  int pad1;
  int pad2;
  void *pad3;
} DBusMessageIter_1_10_0;
static void get_const_signature(DBusHeader *header, const DBusString **type_str_p, int *type_pos_p) {
  if (_dbus_header_get_field_raw(header, DBUS_HEADER_FIELD_SIGNATURE, type_str_p, type_pos_p)) *type_pos_p += 1;
  else {
      *type_str_p = &_dbus_empty_signature_str;
      *type_pos_p = 0;
  }
}
static void _dbus_message_byteswap(DBusMessage *message) {
  const DBusString *type_str;
  int type_pos;
  char byte_order;
  byte_order = _dbus_header_get_byte_order(&message->header);
  if (byte_order == DBUS_COMPILER_BYTE_ORDER) return;
  _dbus_verbose("Swapping message into compiler byte order\n");
  get_const_signature(&message->header, &type_str, &type_pos);
  _dbus_marshal_byteswap(type_str, type_pos, byte_order, DBUS_COMPILER_BYTE_ORDER, &message->body, 0);
  _dbus_header_byteswap(&message->header, DBUS_COMPILER_BYTE_ORDER);
  _dbus_assert(_dbus_header_get_byte_order(&message->header) == DBUS_COMPILER_BYTE_ORDER);
}
#define ensure_byte_order(message)  _dbus_message_byteswap(message)
void _dbus_message_get_network_data(DBusMessage *message, const DBusString **header, const DBusString **body) {
  _dbus_assert(message->locked);
  *header = &message->header.data;
  *body = &message->body;
}
void _dbus_message_get_unix_fds(DBusMessage *message, const int  **fds, unsigned *n_fds) {
  _dbus_assert(message->locked);
#ifdef HAVE_UNIX_FD_PASSING
  *fds = message->unix_fds;
  *n_fds = message->n_unix_fds;
#else
  *fds = NULL;
  *n_fds = 0;
#endif
}
dbus_bool_t _dbus_message_remove_unknown_fields(DBusMessage *message) {
  return _dbus_header_remove_unknown_fields(&message->header);
}
void dbus_message_set_serial(DBusMessage *message, dbus_uint32_t serial) {
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(!message->locked);
  _dbus_header_set_serial(&message->header, serial);
}
void _dbus_message_add_counter_link(DBusMessage *message, DBusList *link) {
  if (message->counters == NULL) {
      message->size_counter_delta = _dbus_string_get_length(&message->header.data) + _dbus_string_get_length(&message->body);
  #ifdef HAVE_UNIX_FD_PASSING
      message->unix_fd_counter_delta = message->n_unix_fds;
  #endif
  #if 0
      _dbus_verbose("message has size %ld\n", message->size_counter_delta);
  #endif
  }
  _dbus_list_append_link(&message->counters, link);
  _dbus_counter_adjust_size(link->data, message->size_counter_delta);
#ifdef HAVE_UNIX_FD_PASSING
  _dbus_counter_adjust_unix_fd(link->data, message->unix_fd_counter_delta);
#endif
}
dbus_bool_t _dbus_message_add_counter(DBusMessage *message, DBusCounter *counter) {
  DBusList *link;
  link = _dbus_list_alloc_link(counter);
  if (link == NULL) return FALSE;
  _dbus_counter_ref(counter);
  _dbus_message_add_counter_link(message, link);
  return TRUE;
}
void _dbus_message_remove_counter(DBusMessage *message, DBusCounter *counter) {
  DBusList *link;
  link = _dbus_list_find_last(&message->counters, counter);
  _dbus_assert(link != NULL);
  _dbus_list_remove_link(&message->counters, link);
  _dbus_counter_adjust_size(counter, - message->size_counter_delta);
#ifdef HAVE_UNIX_FD_PASSING
  _dbus_counter_adjust_unix_fd(counter, - message->unix_fd_counter_delta);
#endif
  _dbus_counter_notify(counter);
  _dbus_counter_unref(counter);
}
void dbus_message_lock(DBusMessage  *message) {
  if (!message->locked) {
      _dbus_header_update_lengths(&message->header, _dbus_string_get_length(&message->body));
      _dbus_assert(_dbus_string_get_length(&message->body) == 0 || dbus_message_get_signature(message) != NULL);
      message->locked = TRUE;
  }
}
static dbus_bool_t set_or_delete_string_field(DBusMessage *message, int field, int typecode, const char *value) {
  if (value == NULL) return _dbus_header_delete_field(&message->header, field);
  else return _dbus_header_set_field_basic(&message->header, field, typecode, &value);
}
#define MAX_MESSAGE_SIZE_TO_CACHE 10 * _DBUS_ONE_KILOBYTE
#define MAX_MESSAGE_CACHE_SIZE    5
static DBusMessage *message_cache[MAX_MESSAGE_CACHE_SIZE];
static int message_cache_count = 0;
static dbus_bool_t message_cache_shutdown_registered = FALSE;
static void dbus_message_cache_shutdown(void *data) {
  int i;
  if (!_DBUS_LOCK(message_cache)) _dbus_assert_not_reached("we would have initialized global locks before registering a shutdown function");
  i = 0;
  while(i < MAX_MESSAGE_CACHE_SIZE) {
      if (message_cache[i]) dbus_message_finalize(message_cache[i]);
      ++i;
  }
  message_cache_count = 0;
  message_cache_shutdown_registered = FALSE;
  _DBUS_UNLOCK(message_cache);
}
static DBusMessage* dbus_message_get_cached(void) {
  DBusMessage *message;
  int i;
  message = NULL;
  if (!_DBUS_LOCK(message_cache)) return NULL;
  _dbus_assert(message_cache_count >= 0);
  if (message_cache_count == 0) {
      _DBUS_UNLOCK(message_cache);
      return NULL;
  }
  _dbus_assert(message_cache_shutdown_registered);
  i = 0;
  while(i < MAX_MESSAGE_CACHE_SIZE) {
      if (message_cache[i]) {
          message = message_cache[i];
          message_cache[i] = NULL;
          message_cache_count -= 1;
          break;
      }
      ++i;
  }
  _dbus_assert(message_cache_count >= 0);
  _dbus_assert(i < MAX_MESSAGE_CACHE_SIZE);
  _dbus_assert(message != NULL);
  _dbus_assert(_dbus_atomic_get(&message->refcount) == 0);
  _dbus_assert(message->counters == NULL);
  _DBUS_UNLOCK(message_cache);
  return message;
}
#ifdef HAVE_UNIX_FD_PASSING
static void close_unix_fds(int *fds, unsigned *n_fds) {
  DBusError e;
  unsigned int i;
  if (*n_fds <= 0) return;
  dbus_error_init(&e);
  for (i = 0; i < *n_fds; i++) {
      if (!_dbus_close(fds[i], &e)) {
          _dbus_warn("Failed to close file descriptor: %s", e.message);
          dbus_error_free(&e);
      }
  }
  *n_fds = 0;
}
#endif
static void free_counter(void *element, void *data) {
  DBusCounter *counter = element;
  DBusMessage *message = data;
  _dbus_counter_adjust_size(counter, - message->size_counter_delta);
#ifdef HAVE_UNIX_FD_PASSING
  _dbus_counter_adjust_unix_fd(counter, - message->unix_fd_counter_delta);
#endif
  _dbus_counter_notify(counter);
  _dbus_counter_unref(counter);
}
static void dbus_message_cache_or_finalize(DBusMessage *message) {
  dbus_bool_t was_cached;
  int i;
  _dbus_assert(_dbus_atomic_get(&message->refcount) == 0);
  _dbus_data_slot_list_clear(&message->slot_list);
  _dbus_list_foreach(&message->counters, free_counter, message);
  _dbus_list_clear(&message->counters);
#ifdef HAVE_UNIX_FD_PASSING
  close_unix_fds(message->unix_fds, &message->n_unix_fds);
#endif
  was_cached = FALSE;
  if (!_DBUS_LOCK(message_cache)) { _dbus_assert_not_reached("we would have initialized global locks the first time we constructed a message"); }
  if (!message_cache_shutdown_registered) {
      _dbus_assert(message_cache_count == 0);
      if (!_dbus_register_shutdown_func(dbus_message_cache_shutdown, NULL)) goto out;
      i = 0;
      while(i < MAX_MESSAGE_CACHE_SIZE) {
          message_cache[i] = NULL;
          ++i;
      }
      message_cache_shutdown_registered = TRUE;
  }
  _dbus_assert(message_cache_count >= 0);
  if (!_dbus_enable_message_cache()) goto out;
  if ((_dbus_string_get_length(&message->header.data) + _dbus_string_get_length(&message->body)) > MAX_MESSAGE_SIZE_TO_CACHE) goto out;
  if (message_cache_count >= MAX_MESSAGE_CACHE_SIZE) goto out;
  i = 0;
  while(message_cache[i] != NULL) ++i;
  _dbus_assert(i < MAX_MESSAGE_CACHE_SIZE);
  _dbus_assert(message_cache[i] == NULL);
  message_cache[i] = message;
  message_cache_count += 1;
  was_cached = TRUE;
#ifndef DBUS_DISABLE_CHECKS
  message->in_cache = TRUE;
#endif
out:
  _dbus_assert(_dbus_atomic_get(&message->refcount) == 0);
  _DBUS_UNLOCK(message_cache);
  if (!was_cached) dbus_message_finalize(message);
}
static void _dbus_message_real_iter_zero(DBusMessageRealIter *iter) {
  _dbus_assert(iter != NULL);
  _DBUS_ZERO(*iter);
  iter->message = NULL;
}
void dbus_message_iter_init_closed(DBusMessageIter *iter) {
  _dbus_return_if_fail(iter != NULL);
  _dbus_message_real_iter_zero((DBusMessageRealIter*) iter);
}
static dbus_bool_t _dbus_message_real_iter_is_zeroed(DBusMessageRealIter *iter) {
  return (iter != NULL && iter->message == NULL && iter->changed_stamp == 0 && iter->iter_type == 0 && iter->sig_refcount == 0);
}
#if !defined(DBUS_ENABLE_CHECKS) || defined(DBUS_ENABLE_ASSERT)
static dbus_bool_t _dbus_message_iter_check(DBusMessageRealIter *iter) {
  char byte_order;
  if (iter == NULL) {
      _dbus_warn_check_failed("dbus message iterator is NULL");
      return FALSE;
  }
  if (iter->message == NULL || iter->iter_type == 0) {
      _dbus_warn_check_failed("dbus message iterator has already been closed, or is uninitialized or corrupt");
      return FALSE;
  }
  byte_order = _dbus_header_get_byte_order(&iter->message->header);
  if (iter->iter_type == DBUS_MESSAGE_ITER_TYPE_READER) {
      if (iter->u.reader.byte_order != byte_order) {
          _dbus_warn_check_failed("dbus message changed byte order since iterator was created");
          return FALSE;
      }
      _dbus_assert(iter->u.reader.byte_order == DBUS_COMPILER_BYTE_ORDER);
  } else if (iter->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER) {
      if (iter->u.writer.byte_order != byte_order) {
          _dbus_warn_check_failed("dbus message changed byte order since append iterator was created");
          return FALSE;
      }
      _dbus_assert(iter->u.writer.byte_order == DBUS_COMPILER_BYTE_ORDER);
  } else {
      _dbus_warn_check_failed("dbus message iterator looks uninitialized or corrupted");
      return FALSE;
  }
  if (iter->changed_stamp != iter->message->changed_stamp) {
      _dbus_warn_check_failed("dbus message iterator invalid because the message has been modified (or perhaps the iterator is just uninitialized)");
      return FALSE;
  }
  return TRUE;
}
#endif
dbus_bool_t _dbus_message_iter_get_args_valist(DBusMessageIter *iter, DBusError *error, int first_arg_type, va_list var_args) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  int spec_type, msg_type, i, j;
  dbus_bool_t retval;
  va_list copy_args = NULL;
  _dbus_assert(_dbus_message_iter_check(real));
  retval = FALSE;
  spec_type = first_arg_type;
  i = 0;
  //DBUS_VA_COPY(copy_args, var_args);
  while(spec_type != DBUS_TYPE_INVALID) {
      msg_type = dbus_message_iter_get_arg_type(iter);
      if (msg_type != spec_type) {
          dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Argument %d is specified to be of type \"%s\", but is actually of type \"%s\"\n", i,
                         _dbus_type_to_string (spec_type), _dbus_type_to_string (msg_type));
          goto out;
      }
      if (spec_type == DBUS_TYPE_UNIX_FD) {
      #ifdef HAVE_UNIX_FD_PASSING
          DBusBasicValue idx;
          int *pfd, nfd;
          pfd = va_arg(var_args, int*);
          _dbus_assert(pfd);
          _dbus_type_reader_read_basic(&real->u.reader, &idx);
          if (idx.u32 >= real->message->n_unix_fds) {
              dbus_set_error(error, DBUS_ERROR_INCONSISTENT_MESSAGE,"Message refers to file descriptor at index %i,but has only %i descriptors "
                             "attached.\n", idx.u32, real->message->n_unix_fds);
              goto out;
          }
          if ((nfd = _dbus_dup(real->message->unix_fds[idx.u32], error)) < 0) goto out;
          *pfd = nfd;
      #else
          dbus_set_error(error, DBUS_ERROR_NOT_SUPPORTED, "Platform does not support file desciptor passing.\n");
          goto out;
      #endif
      } else if (dbus_type_is_basic(spec_type)) {
          DBusBasicValue *ptr;
          ptr = va_arg(var_args, DBusBasicValue*);
          _dbus_assert(ptr != NULL);
          _dbus_type_reader_read_basic(&real->u.reader, ptr);
      } else if (spec_type == DBUS_TYPE_ARRAY) {
          int element_type;
          int spec_element_type;
          const DBusBasicValue **ptr;
          int *n_elements_p;
          DBusTypeReader array;
          spec_element_type = va_arg(var_args, int);
          element_type = _dbus_type_reader_get_element_type(&real->u.reader);
          if (spec_element_type != element_type) {
              dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Argument %d is specified to be an array of \"%s\", but is actually an array of \"%s\"\n",
                             i, _dbus_type_to_string(spec_element_type), _dbus_type_to_string (element_type));
              goto out;
          }
          if (dbus_type_is_fixed(spec_element_type) && element_type != DBUS_TYPE_UNIX_FD) {
              ptr = va_arg(var_args, const DBusBasicValue**);
              n_elements_p = va_arg(var_args, int*);
              _dbus_assert(ptr != NULL);
              _dbus_assert(n_elements_p != NULL);
              _dbus_type_reader_recurse(&real->u.reader, &array);
              _dbus_type_reader_read_fixed_multi(&array, (void*)ptr, n_elements_p);
          } else if (_DBUS_TYPE_IS_STRINGLIKE(spec_element_type)) {
              char ***str_array_p;
              int n_elements;
              char **str_array;
              str_array_p = va_arg(var_args, char***);
              n_elements_p = va_arg(var_args, int*);
              _dbus_assert(str_array_p != NULL);
              _dbus_assert(n_elements_p != NULL);
              _dbus_type_reader_recurse(&real->u.reader, &array);
              n_elements = 0;
              while(_dbus_type_reader_get_current_type(&array) != DBUS_TYPE_INVALID) {
                  ++n_elements;
                  _dbus_type_reader_next(&array);
              }
              str_array = dbus_new0(char*, n_elements + 1);
              if (str_array == NULL) {
                  _DBUS_SET_OOM(error);
                  goto out;
              }
              _dbus_type_reader_recurse(&real->u.reader, &array);
              j = 0;
              while(j < n_elements) {
                  const char *s;
                  _dbus_type_reader_read_basic(&array, (void*)&s);
                  str_array[j] = _dbus_strdup(s);
                  if (str_array[j] == NULL) {
                      dbus_free_string_array(str_array);
                      _DBUS_SET_OOM(error);
                      goto out;
                  }
                  ++j;
                  if (!_dbus_type_reader_next(&array)) _dbus_assert(j == n_elements);
              }
              _dbus_assert(_dbus_type_reader_get_current_type(&array) == DBUS_TYPE_INVALID);
              _dbus_assert(j == n_elements);
              _dbus_assert(str_array[j] == NULL);
              *str_array_p = str_array;
              *n_elements_p = n_elements;
          }
      #ifdef DBUS_DISABLE_CHECKS
          else {
              _dbus_warn("you can't read arrays of container types (struct, variant, array) with %s for now", _DBUS_FUNCTION_NAME);
              goto out;
          }
      #endif
      }
  #ifdef DBUS_DISABLE_CHECKS
      else {
          _dbus_warn("you can only read arrays and basic types with %s for now", _DBUS_FUNCTION_NAME);
          goto out;
      }
  #endif
      i++;
      spec_type = va_arg(var_args, int);
      if (!_dbus_type_reader_next(&real->u.reader) && spec_type != DBUS_TYPE_INVALID) {
          dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Message has only %d arguments, but more were expected", i);
          goto out;
      }
  }
  retval = TRUE;
out:
  if (!retval) {
      spec_type = first_arg_type;
      j = 0;
      while(j < i) {
          if (spec_type == DBUS_TYPE_UNIX_FD) {
          #ifdef HAVE_UNIX_FD_PASSING
              int *pfd;
              pfd = va_arg(copy_args, int *);
              _dbus_assert(pfd);
              if (*pfd >= 0) {
                  _dbus_close(*pfd, NULL);
                  *pfd = -1;
              }
          #endif
          } else if (dbus_type_is_basic(spec_type)) { va_arg(copy_args, DBusBasicValue *); }
          else if (spec_type == DBUS_TYPE_ARRAY) {
              int spec_element_type;
              spec_element_type = va_arg(copy_args, int);
              if (dbus_type_is_fixed(spec_element_type)) {
                  va_arg(copy_args, const DBusBasicValue**);
                  va_arg(copy_args, int*);
              } else if (_DBUS_TYPE_IS_STRINGLIKE(spec_element_type)) {
                  char ***str_array_p;
                  str_array_p = va_arg(copy_args, char***);
                  va_arg(copy_args, int*);
                  _dbus_assert(str_array_p != NULL);
                  dbus_free_string_array(*str_array_p);
                  *str_array_p = NULL;
              }
          }
          spec_type = va_arg(copy_args, int);
          j++;
      }
  }
  va_end(copy_args);
  return retval;
}
dbus_uint32_t dbus_message_get_serial(DBusMessage *message) {
  _dbus_return_val_if_fail(message != NULL, 0);
  return _dbus_header_get_serial(&message->header);
}
dbus_bool_t dbus_message_set_reply_serial(DBusMessage *message, dbus_uint32_t reply_serial) {
  DBusBasicValue value;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(!message->locked, FALSE);
  _dbus_return_val_if_fail(reply_serial != 0, FALSE);
  value.u32 = reply_serial;
  return _dbus_header_set_field_basic(&message->header, DBUS_HEADER_FIELD_REPLY_SERIAL, DBUS_TYPE_UINT32, &value);
}
dbus_uint32_t dbus_message_get_reply_serial(DBusMessage *message) {
  dbus_uint32_t v_UINT32;
  _dbus_return_val_if_fail(message != NULL, 0);
  if (_dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_REPLY_SERIAL, DBUS_TYPE_UINT32, &v_UINT32)) return v_UINT32;
  else return 0;
}
static void dbus_message_finalize(DBusMessage *message) {
  _dbus_assert(_dbus_atomic_get(&message->refcount) == 0);
  _dbus_data_slot_list_free(&message->slot_list);
  _dbus_list_foreach(&message->counters, free_counter, message);
  _dbus_list_clear(&message->counters);
  _dbus_header_free(&message->header);
  _dbus_string_free(&message->body);
#ifdef HAVE_UNIX_FD_PASSING
  close_unix_fds(message->unix_fds, &message->n_unix_fds);
  dbus_free(message->unix_fds);
#endif
  _dbus_assert(_dbus_atomic_get(&message->refcount) == 0);
  dbus_free(message);
}
static DBusMessage* dbus_message_new_empty_header(void) {
  DBusMessage *message;
  dbus_bool_t from_cache;
  message = dbus_message_get_cached();
  if (message != NULL) from_cache = TRUE;
  else {
      from_cache = FALSE;
      message = dbus_new0(DBusMessage, 1);
      if (message == NULL) return NULL;
  #ifndef DBUS_DISABLE_CHECKS
      message->generation = _dbus_current_generation;
  #endif
  #ifdef HAVE_UNIX_FD_PASSING
      message->unix_fds = NULL;
      message->n_unix_fds_allocated = 0;
#endif
  }
  _dbus_atomic_inc(&message->refcount);
  _dbus_message_trace_ref(message, 0, 1, "new_empty_header");
  message->locked = FALSE;
#ifndef DBUS_DISABLE_CHECKS
  message->in_cache = FALSE;
#endif
  message->counters = NULL;
  message->size_counter_delta = 0;
  message->changed_stamp = 0;
#ifdef HAVE_UNIX_FD_PASSING
  message->n_unix_fds = 0;
  message->n_unix_fds_allocated = 0;
  message->unix_fd_counter_delta = 0;
#endif
  if (!from_cache) _dbus_data_slot_list_init(&message->slot_list);
  if (from_cache) {
      _dbus_header_reinit(&message->header);
      _dbus_string_set_length(&message->body, 0);
  } else {
      if (!_dbus_header_init(&message->header)) {
          dbus_free(message);
          return NULL;
      }
      if (!_dbus_string_init_preallocated(&message->body, 32)) {
          _dbus_header_free(&message->header);
          dbus_free(message);
          return NULL;
      }
  }
  return message;
}
DBusMessage* dbus_message_new(int message_type) {
  DBusMessage *message;
  _dbus_return_val_if_fail(message_type != DBUS_MESSAGE_TYPE_INVALID, NULL);
  message = dbus_message_new_empty_header();
  if (message == NULL) return NULL;
  if (!_dbus_header_create(&message->header, DBUS_COMPILER_BYTE_ORDER, message_type,NULL, NULL, NULL, NULL, NULL)) {
      dbus_message_unref(message);
      return NULL;
  }
  return message;
}
DBusMessage* dbus_message_new_method_call(const char *destination, const char *path, const char *iface, const char *method) {
  DBusMessage *message;
  _dbus_return_val_if_fail(path != NULL, NULL);
  _dbus_return_val_if_fail(method != NULL, NULL);
  _dbus_return_val_if_fail(destination == NULL || _dbus_check_is_valid_bus_name(destination), NULL);
  _dbus_return_val_if_fail(_dbus_check_is_valid_path(path), NULL);
  _dbus_return_val_if_fail(iface == NULL || _dbus_check_is_valid_interface(iface), NULL);
  _dbus_return_val_if_fail(_dbus_check_is_valid_member(method), NULL);
  message = dbus_message_new_empty_header();
  if (message == NULL) return NULL;
  if (!_dbus_header_create(&message->header, DBUS_COMPILER_BYTE_ORDER, DBUS_MESSAGE_TYPE_METHOD_CALL, destination, path, iface, method, NULL)) {
      dbus_message_unref(message);
      return NULL;
  }
  return message;
}
DBusMessage* dbus_message_new_method_return(DBusMessage *method_call) {
  DBusMessage *message;
  const char *sender;
  _dbus_return_val_if_fail(method_call != NULL, NULL);
  sender = dbus_message_get_sender(method_call);
  message = dbus_message_new_empty_header();
  if (message == NULL) return NULL;
  if (!_dbus_header_create(&message->header, DBUS_COMPILER_BYTE_ORDER, DBUS_MESSAGE_TYPE_METHOD_RETURN, sender,NULL,NULL,NULL,NULL)) {
      dbus_message_unref(message);
      return NULL;
  }
  dbus_message_set_no_reply(message, TRUE);
  if (!dbus_message_set_reply_serial(message, dbus_message_get_serial(method_call))) {
      dbus_message_unref(message);
      return NULL;
  }
  return message;
}
DBusMessage* dbus_message_new_signal(const char *path, const char *iface, const char *name) {
  DBusMessage *message;
  _dbus_return_val_if_fail(path != NULL, NULL);
  _dbus_return_val_if_fail(iface != NULL, NULL);
  _dbus_return_val_if_fail(name != NULL, NULL);
  _dbus_return_val_if_fail(_dbus_check_is_valid_path(path), NULL);
  _dbus_return_val_if_fail(_dbus_check_is_valid_interface(iface), NULL);
  _dbus_return_val_if_fail(_dbus_check_is_valid_member(name), NULL);
  message = dbus_message_new_empty_header();
  if (message == NULL) return NULL;
  if (!_dbus_header_create(&message->header, DBUS_COMPILER_BYTE_ORDER, DBUS_MESSAGE_TYPE_SIGNAL,NULL, path, iface, name,NULL)) {
      dbus_message_unref(message);
      return NULL;
  }
  dbus_message_set_no_reply(message, TRUE);
  return message;
}
DBusMessage* dbus_message_new_error(DBusMessage *reply_to, const char *error_name, const char *error_message) {
  DBusMessage *message;
  const char *sender;
  DBusMessageIter iter;
  _dbus_return_val_if_fail(reply_to != NULL, NULL);
  _dbus_return_val_if_fail(error_name != NULL, NULL);
  _dbus_return_val_if_fail(_dbus_check_is_valid_error_name(error_name), NULL);
  sender = dbus_message_get_sender(reply_to);
  message = dbus_message_new_empty_header();
  if (message == NULL) return NULL;
  if (!_dbus_header_create(&message->header, DBUS_COMPILER_BYTE_ORDER, DBUS_MESSAGE_TYPE_ERROR, sender,NULL,NULL,NULL, error_name)) {
      dbus_message_unref(message);
      return NULL;
  }
  dbus_message_set_no_reply(message, TRUE);
  if (!dbus_message_set_reply_serial(message, dbus_message_get_serial(reply_to))) {
      dbus_message_unref(message);
      return NULL;
  }
  if (error_message != NULL) {
      dbus_message_iter_init_append(message, &iter);
      if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &error_message)) {
          dbus_message_unref(message);
          return NULL;
      }
  }
  return message;
}
DBusMessage* dbus_message_new_error_printf(DBusMessage *reply_to, const char *error_name, const char *error_format, ...) {
  va_list args;
  DBusString str;
  DBusMessage *message;
  _dbus_return_val_if_fail(reply_to != NULL, NULL);
  _dbus_return_val_if_fail(error_name != NULL, NULL);
  _dbus_return_val_if_fail(_dbus_check_is_valid_error_name(error_name), NULL);
  if (!_dbus_string_init (&str)) return NULL;
  va_start(args, error_format);
  if (_dbus_string_append_printf_valist(&str, error_format, args)) message = dbus_message_new_error(reply_to, error_name, _dbus_string_get_const_data(&str));
  else message = NULL;
  _dbus_string_free(&str);
  va_end(args);
  return message;
}
DBusMessage *dbus_message_copy(const DBusMessage *message) {
  DBusMessage *retval;
  _dbus_return_val_if_fail(message != NULL, NULL);
  retval = dbus_new0(DBusMessage, 1);
  if (retval == NULL) return NULL;
  _dbus_atomic_inc(&retval->refcount);
  retval->locked = FALSE;
#ifndef DBUS_DISABLE_CHECKS
  retval->generation = message->generation;
#endif
  if (!_dbus_header_copy(&message->header, &retval->header)) {
      dbus_free(retval);
      return NULL;
  }
  if (!_dbus_string_init_preallocated(&retval->body, _dbus_string_get_length(&message->body))) {
      _dbus_header_free(&retval->header);
      dbus_free(retval);
      return NULL;
  }
  if (!_dbus_string_copy(&message->body, 0, &retval->body, 0)) goto failed_copy;
#ifdef HAVE_UNIX_FD_PASSING
  retval->unix_fds = dbus_new(int, message->n_unix_fds);
  if (retval->unix_fds == NULL && message->n_unix_fds > 0) goto failed_copy;
  retval->n_unix_fds_allocated = message->n_unix_fds;
  for (retval->n_unix_fds = 0; retval->n_unix_fds < message->n_unix_fds; retval->n_unix_fds++) {
      retval->unix_fds[retval->n_unix_fds] = _dbus_dup(message->unix_fds[retval->n_unix_fds], NULL);
      if (retval->unix_fds[retval->n_unix_fds] < 0) goto failed_copy;
  }
#endif
  _dbus_message_trace_ref(retval, 0, 1, "copy");
  return retval;
failed_copy:
  _dbus_header_free(&retval->header);
  _dbus_string_free(&retval->body);
#ifdef HAVE_UNIX_FD_PASSING
  close_unix_fds(retval->unix_fds, &retval->n_unix_fds);
  dbus_free(retval->unix_fds);
#endif
  dbus_free(retval);
  return NULL;
}
DBusMessage *dbus_message_ref(DBusMessage *message) {
  dbus_int32_t old_refcount;
  _dbus_return_val_if_fail(message != NULL, NULL);
  _dbus_return_val_if_fail(message->generation == _dbus_current_generation, NULL);
  _dbus_return_val_if_fail(!message->in_cache, NULL);
  old_refcount = _dbus_atomic_inc(&message->refcount);
  _dbus_assert(old_refcount >= 1);
  _dbus_message_trace_ref(message, old_refcount, old_refcount + 1, "ref");
  return message;
}
void dbus_message_unref (DBusMessage *message) {
  dbus_int32_t old_refcount;
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(message->generation == _dbus_current_generation);
  _dbus_return_if_fail(!message->in_cache);
  old_refcount = _dbus_atomic_dec(&message->refcount);
  _dbus_assert(old_refcount >= 1);
  _dbus_message_trace_ref(message, old_refcount, old_refcount - 1, "unref");
  if (old_refcount == 1) dbus_message_cache_or_finalize(message);
}
int dbus_message_get_type(DBusMessage *message) {
  _dbus_return_val_if_fail(message != NULL, DBUS_MESSAGE_TYPE_INVALID);
  return _dbus_header_get_message_type(&message->header);
}
dbus_bool_t dbus_message_append_args(DBusMessage *message, int first_arg_type, ...) {
  dbus_bool_t retval;
  va_list var_args;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  va_start(var_args, first_arg_type);
  retval = dbus_message_append_args_valist(message, first_arg_type, var_args);
  va_end(var_args);
  return retval;
}
dbus_bool_t dbus_message_append_args_valist(DBusMessage *message, int first_arg_type, va_list var_args) {
  int type;
  DBusMessageIter iter;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  type = first_arg_type;
  dbus_message_iter_init_append(message, &iter);
  while(type != DBUS_TYPE_INVALID) {
      if (dbus_type_is_basic(type)) {
          const DBusBasicValue *value;
          value = va_arg(var_args, const DBusBasicValue*);
          if (!dbus_message_iter_append_basic(&iter, type, value)) goto failed;
      } else if (type == DBUS_TYPE_ARRAY) {
          int element_type;
          DBusMessageIter array;
          char buf[2];
          element_type = va_arg(var_args, int);
          buf[0] = element_type;
          buf[1] = '\0';
          if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, buf, &array)) goto failed;
          if (dbus_type_is_fixed(element_type) && element_type != DBUS_TYPE_UNIX_FD) {
              const DBusBasicValue **value;
              int n_elements;
              value = va_arg(var_args, const DBusBasicValue**);
              n_elements = va_arg(var_args, int);
              if (!dbus_message_iter_append_fixed_array(&array, element_type, value, n_elements)) {
                  dbus_message_iter_abandon_container(&iter, &array);
                  goto failed;
              }
          } else if (_DBUS_TYPE_IS_STRINGLIKE(element_type)) {
              const char ***value_p;
              const char **value;
              int n_elements;
              int i;
              value_p = va_arg(var_args, const char***);
              n_elements = va_arg(var_args, int);
              value = *value_p;
              i = 0;
              while(i < n_elements) {
                  if (!dbus_message_iter_append_basic(&array, element_type, &value[i])) {
                      dbus_message_iter_abandon_container(&iter, &array);
                      goto failed;
                  }
                  ++i;
              }
          } else {
              _dbus_warn("arrays of %s can't be appended with %s for now", _dbus_type_to_string(element_type), _DBUS_FUNCTION_NAME);
              dbus_message_iter_abandon_container(&iter, &array);
              goto failed;
          }
          if (!dbus_message_iter_close_container(&iter, &array)) goto failed;
      }
  #ifdef DBUS_DISABLE_CHECKS
      else {
          _dbus_warn("type %s isn't supported yet in %s", _dbus_type_to_string(type), _DBUS_FUNCTION_NAME);
          goto failed;
      }
  #endif
      type = va_arg(var_args, int);
  }
  return TRUE;
failed:
  return FALSE;
}
dbus_bool_t dbus_message_get_args(DBusMessage *message, DBusError *error, int first_arg_type, ...) {
  dbus_bool_t retval;
  va_list var_args;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_error_is_set(error, FALSE);
  va_start(var_args, first_arg_type);
  retval = dbus_message_get_args_valist(message, error, first_arg_type, var_args);
  va_end(var_args);
  return retval;
}
dbus_bool_t dbus_message_get_args_valist(DBusMessage *message, DBusError *error, int first_arg_type, va_list var_args) {
  DBusMessageIter iter;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_error_is_set(error, FALSE);
  dbus_message_iter_init(message, &iter);
  return _dbus_message_iter_get_args_valist(&iter, error, first_arg_type, var_args);
}
static void _dbus_message_iter_init_common(DBusMessage *message, DBusMessageRealIter *real, int iter_type) {
  _DBUS_STATIC_ASSERT(sizeof(DBusMessageRealIter) <= sizeof(DBusMessageIter));
  _DBUS_STATIC_ASSERT(_DBUS_ALIGNOF(DBusMessageRealIter) <= _DBUS_ALIGNOF(DBusMessageIter));
  _DBUS_STATIC_ASSERT(sizeof(DBusMessageIter_1_10_0) == sizeof(DBusMessageIter));
  _DBUS_STATIC_ASSERT(_DBUS_ALIGNOF(DBusMessageIter_1_10_0) == _DBUS_ALIGNOF(DBusMessageIter));
  _DBUS_STATIC_ASSERT(sizeof(DBusMessageIter) == 4 * sizeof(void*) + sizeof(dbus_uint32_t) + 9 * sizeof(int));
  ensure_byte_order(message);
  real->message = message;
  real->changed_stamp = message->changed_stamp;
  real->iter_type = iter_type;
  real->sig_refcount = 0;
}
dbus_bool_t dbus_message_iter_init(DBusMessage *message, DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  const DBusString *type_str;
  int type_pos;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(iter != NULL, FALSE);
  get_const_signature (&message->header, &type_str, &type_pos);
  _dbus_message_iter_init_common(message, real,DBUS_MESSAGE_ITER_TYPE_READER);
  _dbus_type_reader_init(&real->u.reader, _dbus_header_get_byte_order(&message->header), type_str, type_pos, &message->body,0);
  return _dbus_type_reader_get_current_type(&real->u.reader) != DBUS_TYPE_INVALID;
}
dbus_bool_t dbus_message_iter_has_next(DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_val_if_fail(_dbus_message_iter_check(real), FALSE);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_READER, FALSE);
  return _dbus_type_reader_has_next(&real->u.reader);
}
dbus_bool_t dbus_message_iter_next(DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_val_if_fail(_dbus_message_iter_check(real), FALSE);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_READER, FALSE);
  return _dbus_type_reader_next(&real->u.reader);
}
int dbus_message_iter_get_arg_type(DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_val_if_fail(_dbus_message_iter_check(real), DBUS_TYPE_INVALID);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_READER, FALSE);
  return _dbus_type_reader_get_current_type(&real->u.reader);
}
int dbus_message_iter_get_element_type(DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_val_if_fail(_dbus_message_iter_check(real), DBUS_TYPE_INVALID);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_READER, DBUS_TYPE_INVALID);
  _dbus_return_val_if_fail(dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_ARRAY, DBUS_TYPE_INVALID);
  return _dbus_type_reader_get_element_type(&real->u.reader);
}
void dbus_message_iter_recurse(DBusMessageIter *iter, DBusMessageIter *sub) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  DBusMessageRealIter *real_sub = (DBusMessageRealIter*)sub;
  _dbus_return_if_fail(_dbus_message_iter_check(real));
  _dbus_return_if_fail(sub != NULL);
  *real_sub = *real;
  _dbus_type_reader_recurse(&real->u.reader, &real_sub->u.reader);
}
char *dbus_message_iter_get_signature(DBusMessageIter *iter) {
  const DBusString *sig;
  DBusString retstr;
  char *ret;
  int start, len;
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_val_if_fail(_dbus_message_iter_check(real), NULL);
  if (!_dbus_string_init(&retstr)) return NULL;
  _dbus_type_reader_get_signature(&real->u.reader, &sig, &start, &len);
  if (!_dbus_string_append_len(&retstr,_dbus_string_get_const_data(sig) + start, len)) return NULL;
  if (!_dbus_string_steal_data(&retstr, &ret)) return NULL;
  _dbus_string_free(&retstr);
  return ret;
}
void dbus_message_iter_get_basic(DBusMessageIter *iter, void *value) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_if_fail(_dbus_message_iter_check(real));
  _dbus_return_if_fail(value != NULL);
  if (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_UNIX_FD) {
  #ifdef HAVE_UNIX_FD_PASSING
      DBusBasicValue idx;
      _dbus_type_reader_read_basic(&real->u.reader, &idx);
      if (idx.u32 >= real->message->n_unix_fds) {
          *((int*)value) = -1;
          return;
      }
      *((int*)value) = _dbus_dup(real->message->unix_fds[idx.u32], NULL);
  #else
      *((int*)value) = -1;
  #endif
  } else _dbus_type_reader_read_basic(&real->u.reader, value);
}
int dbus_message_iter_get_element_count(DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  DBusTypeReader array;
  int element_type;
  int n_elements = 0;
  _dbus_return_val_if_fail(_dbus_message_iter_check(real), 0);
  _dbus_return_val_if_fail(_dbus_type_reader_get_current_type(&real->u.reader) == DBUS_TYPE_ARRAY, 0);
  element_type = _dbus_type_reader_get_element_type(&real->u.reader);
  _dbus_type_reader_recurse(&real->u.reader, &array);
  if (dbus_type_is_fixed(element_type)) {
      int alignment = _dbus_type_get_alignment(element_type);
      int total_len = _dbus_type_reader_get_array_length(&array);
      n_elements = total_len / alignment;
  } else {
      while(_dbus_type_reader_get_current_type(&array) != DBUS_TYPE_INVALID) {
          ++n_elements;
          _dbus_type_reader_next(&array);
      }
  }
   return n_elements;
}
int dbus_message_iter_get_array_len(DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_val_if_fail(_dbus_message_iter_check(real), 0);
  return _dbus_type_reader_get_array_length(&real->u.reader);
}
void dbus_message_iter_get_fixed_array(DBusMessageIter *iter, void *value, int *n_elements) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
#ifdef DBUS_DISABLE_CHECKS
  int subtype = _dbus_type_reader_get_current_type(&real->u.reader);
  _dbus_return_if_fail(_dbus_message_iter_check(real));
  _dbus_return_if_fail(value != NULL);
  _dbus_return_if_fail((subtype == DBUS_TYPE_INVALID) || (dbus_type_is_fixed(subtype) && subtype != DBUS_TYPE_UNIX_FD));
#endif
  _dbus_type_reader_read_fixed_multi(&real->u.reader, value, n_elements);
}
void dbus_message_iter_init_append(DBusMessage *message, DBusMessageIter *iter) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(iter != NULL);
  _dbus_message_iter_init_common(message, real,DBUS_MESSAGE_ITER_TYPE_WRITER);
  _dbus_type_writer_init_types_delayed(&real->u.writer, _dbus_header_get_byte_order(&message->header), &message->body, _dbus_string_get_length(&message->body));
}
static dbus_bool_t _dbus_message_iter_open_signature(DBusMessageRealIter *real) {
  DBusString *str;
  const DBusString *current_sig;
  int current_sig_pos;
  _dbus_assert(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
  if (real->u.writer.type_str != NULL) {
      _dbus_assert(real->sig_refcount > 0);
      real->sig_refcount += 1;
      return TRUE;
  }
  str = dbus_new(DBusString, 1);
  if (str == NULL) return FALSE;
  if (!_dbus_header_get_field_raw(&real->message->header, DBUS_HEADER_FIELD_SIGNATURE, &current_sig, &current_sig_pos)) current_sig = NULL;
  if (current_sig) {
      int current_len;
      current_len = _dbus_string_get_byte(current_sig, current_sig_pos);
      current_sig_pos += 1;
      if (!_dbus_string_init_preallocated(str, current_len + 4)) {
          dbus_free(str);
          return FALSE;
      }
      if (!_dbus_string_copy_len(current_sig, current_sig_pos, current_len, str, 0)) {
          _dbus_string_free(str);
          dbus_free(str);
          return FALSE;
      }
  } else {
      if (!_dbus_string_init_preallocated(str, 4)) {
          dbus_free(str);
          return FALSE;
      }
  }
  real->sig_refcount = 1;
  _dbus_assert(real->u.writer.type_str == NULL);
  _dbus_type_writer_add_types(&real->u.writer, str, _dbus_string_get_length(str));
  return TRUE;
}
static dbus_bool_t _dbus_message_iter_close_signature(DBusMessageRealIter *real) {
  DBusString *str;
  const char *v_STRING;
  dbus_bool_t retval;
  _dbus_assert(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
  _dbus_assert(real->u.writer.type_str != NULL);
  _dbus_assert(real->sig_refcount > 0);
  real->sig_refcount -= 1;
  if (real->sig_refcount > 0) return TRUE;
  _dbus_assert(real->sig_refcount == 0);
  retval = TRUE;
  str = real->u.writer.type_str;
  v_STRING = _dbus_string_get_const_data(str);
  if (!_dbus_header_set_field_basic(&real->message->header, DBUS_HEADER_FIELD_SIGNATURE, DBUS_TYPE_SIGNATURE, &v_STRING)) retval = FALSE;
  _dbus_type_writer_remove_types(&real->u.writer);
  _dbus_string_free(str);
  dbus_free(str);
  return retval;
}
static void _dbus_message_iter_abandon_signature(DBusMessageRealIter *real) {
  DBusString *str;
  _dbus_assert(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
  _dbus_assert(real->u.writer.type_str != NULL);
  _dbus_assert(real->sig_refcount > 0);
  real->sig_refcount -= 1;
  if (real->sig_refcount > 0) return;
  _dbus_assert(real->sig_refcount == 0);
  str = real->u.writer.type_str;
  _dbus_type_writer_remove_types(&real->u.writer);
  _dbus_string_free(str);
  dbus_free(str);
}
#ifdef DBUS_DISABLE_CHECKS
static dbus_bool_t _dbus_message_iter_append_check(DBusMessageRealIter *iter) {
  if (!_dbus_message_iter_check(iter)) return FALSE;
  if (iter->message->locked) {
      _dbus_warn_check_failed("dbus append iterator can't be used: message is locked (has already been sent)");
      return FALSE;
  }
  return TRUE;
}
#endif
#ifdef HAVE_UNIX_FD_PASSING
static int *expand_fd_array(DBusMessage *m, unsigned n) {
  _dbus_assert(m);
  if (m->n_unix_fds + n > m->n_unix_fds_allocated) {
      unsigned k;
      int *p;
      k = (m->n_unix_fds + n) * 2;
      if (k < 4) k = 4;
      p = dbus_realloc(m->unix_fds, k * sizeof(int));
      if (p == NULL) return NULL;
      m->unix_fds = p;
      m->n_unix_fds_allocated = k;
  }
  return m->unix_fds + m->n_unix_fds;
}
#endif
dbus_bool_t dbus_message_iter_append_basic(DBusMessageIter *iter, int type, const void *value) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  dbus_bool_t ret;
  _dbus_return_val_if_fail(_dbus_message_iter_append_check(real), FALSE);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER, FALSE);
  _dbus_return_val_if_fail(dbus_type_is_basic(type), FALSE);
  _dbus_return_val_if_fail(value != NULL, FALSE);
#ifdef DBUS_DISABLE_CHECKS
  DBusString str;
  DBusValidity signature_validity;
  const char * const *string_p;
  const dbus_bool_t *bool_p;
  switch(type) {
      case DBUS_TYPE_STRING:
          string_p = value;
          _dbus_return_val_if_fail(_dbus_check_is_valid_utf8(*string_p), FALSE);
          break;
      case DBUS_TYPE_OBJECT_PATH:
          string_p = value;
          _dbus_return_val_if_fail(_dbus_check_is_valid_path(*string_p), FALSE);
          break;
      case DBUS_TYPE_SIGNATURE:
          string_p = value;
          _dbus_string_init_const(&str, *string_p);
          signature_validity = _dbus_validate_signature_with_reason(&str,0, _dbus_string_get_length(&str));
          if (signature_validity == DBUS_VALIDITY_UNKNOWN_OOM_ERROR) return FALSE;
          _dbus_return_val_if_fail(signature_validity == DBUS_VALID, FALSE);
          break;
      case DBUS_TYPE_BOOLEAN:
          bool_p = value;
          _dbus_return_val_if_fail(*bool_p == 0 || *bool_p == 1, FALSE);
          break;
  }
#endif
  if (!_dbus_message_iter_open_signature(real)) return FALSE;
  if (type == DBUS_TYPE_UNIX_FD) {
#ifdef HAVE_UNIX_FD_PASSING
      int *fds;
      dbus_uint32_t u;
      ret = FALSE;
      if (!(fds = expand_fd_array(real->message, 1))) goto out;
      *fds = _dbus_dup(*(int*)value, NULL);
      if (*fds < 0) goto out;
      u = real->message->n_unix_fds;
      if (!(ret = _dbus_type_writer_write_basic(&real->u.writer, DBUS_TYPE_UNIX_FD, &u))) {
          _dbus_close(*fds, NULL);
          goto out;
      }
      real->message->n_unix_fds += 1;
      u += 1;
      ret = _dbus_header_set_field_basic(&real->message->header, DBUS_HEADER_FIELD_UNIX_FDS, DBUS_TYPE_UINT32, &u);
  #else
      ret = FALSE;
      goto out;
  #endif
  } else ret = _dbus_type_writer_write_basic(&real->u.writer, type, value);
out:
  if (!_dbus_message_iter_close_signature(real)) ret = FALSE;
  return ret;
}
dbus_bool_t dbus_message_iter_append_fixed_array(DBusMessageIter *iter, int element_type, const void *value, int n_elements) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  dbus_bool_t ret;
  _dbus_return_val_if_fail(_dbus_message_iter_append_check(real), FALSE);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER, FALSE);
  _dbus_return_val_if_fail(dbus_type_is_fixed(element_type) && element_type != DBUS_TYPE_UNIX_FD, FALSE);
  _dbus_return_val_if_fail(real->u.writer.container_type == DBUS_TYPE_ARRAY, FALSE);
  _dbus_return_val_if_fail(value != NULL, FALSE);
  _dbus_return_val_if_fail(n_elements >= 0, FALSE);
  _dbus_return_val_if_fail(n_elements <= DBUS_MAXIMUM_ARRAY_LENGTH / _dbus_type_get_alignment(element_type), FALSE);
#ifdef DBUS_DISABLE_CHECKS
  if (element_type == DBUS_TYPE_BOOLEAN) {
      const dbus_bool_t * const *bools = value;
      int i;
      for (i = 0; i < n_elements; i++) _dbus_return_val_if_fail((*bools)[i] == 0 || (*bools)[i] == 1, FALSE);
  }
#endif
  ret = _dbus_type_writer_write_fixed_multi(&real->u.writer, element_type, value, n_elements);
  return ret;
}
dbus_bool_t dbus_message_iter_open_container(DBusMessageIter *iter, int type, const char *contained_signature, DBusMessageIter *sub) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  DBusMessageRealIter *real_sub = (DBusMessageRealIter*)sub;
  DBusString contained_str;
  DBusValidity contained_signature_validity;
  dbus_bool_t ret;
  _dbus_return_val_if_fail(sub != NULL, FALSE);
  _dbus_message_real_iter_zero(real_sub);
  _dbus_return_val_if_fail(_dbus_message_iter_append_check(real), FALSE);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER, FALSE);
  _dbus_return_val_if_fail(dbus_type_is_container(type), FALSE);
  _dbus_return_val_if_fail((type == DBUS_TYPE_STRUCT && contained_signature == NULL) ||(type == DBUS_TYPE_DICT_ENTRY &&
                           contained_signature == NULL) || (type == DBUS_TYPE_VARIANT && contained_signature != NULL) || (type == DBUS_TYPE_ARRAY &&
                           contained_signature != NULL), FALSE);
  if (contained_signature != NULL) {
      _dbus_string_init_const(&contained_str, contained_signature);
      contained_signature_validity = _dbus_validate_signature_with_reason(&contained_str,0, _dbus_string_get_length(&contained_str));
      if (contained_signature_validity == DBUS_VALIDITY_UNKNOWN_OOM_ERROR) return FALSE;
  } else contained_signature_validity = DBUS_VALID_BUT_INCOMPLETE;
  _dbus_return_val_if_fail((type == DBUS_TYPE_ARRAY && contained_signature && *contained_signature == DBUS_DICT_ENTRY_BEGIN_CHAR) ||
                            contained_signature == NULL || contained_signature_validity == DBUS_VALID, FALSE);
  if (!_dbus_message_iter_open_signature(real)) return FALSE;
  ret = FALSE;
  *real_sub = *real;
  if (contained_signature != NULL) {
      _dbus_string_init_const(&contained_str, contained_signature);
      ret = _dbus_type_writer_recurse(&real->u.writer, type, &contained_str, 0, &real_sub->u.writer);
  } else ret = _dbus_type_writer_recurse(&real->u.writer, type,NULL, 0, &real_sub->u.writer);
  if (!ret) _dbus_message_iter_abandon_signature(real);
  return ret;
}
dbus_bool_t dbus_message_iter_close_container(DBusMessageIter *iter, DBusMessageIter *sub) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  DBusMessageRealIter *real_sub = (DBusMessageRealIter*)sub;
  dbus_bool_t ret;
  _dbus_return_val_if_fail(_dbus_message_iter_append_check(real), FALSE);
  _dbus_return_val_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER, FALSE);
  _dbus_return_val_if_fail(_dbus_message_iter_append_check(real_sub), FALSE);
  _dbus_return_val_if_fail(real_sub->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER, FALSE);
  ret = _dbus_type_writer_unrecurse(&real->u.writer, &real_sub->u.writer);
  _dbus_message_real_iter_zero(real_sub);
  if (!_dbus_message_iter_close_signature(real)) ret = FALSE;
  return ret;
}
void dbus_message_iter_abandon_container(DBusMessageIter *iter, DBusMessageIter *sub) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  DBusMessageRealIter *real_sub = (DBusMessageRealIter*)sub;
#ifndef DBUS_DISABLE_CHECKS
  _dbus_return_if_fail(_dbus_message_iter_append_check(real));
  _dbus_return_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
  _dbus_return_if_fail(_dbus_message_iter_append_check(real_sub));
  _dbus_return_if_fail(real_sub->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
#endif
  _dbus_message_iter_abandon_signature(real);
  _dbus_message_real_iter_zero(real_sub);
}
void dbus_message_iter_abandon_container_if_open(DBusMessageIter *iter, DBusMessageIter *sub) {
  DBusMessageRealIter *real = (DBusMessageRealIter*)iter;
  DBusMessageRealIter *real_sub = (DBusMessageRealIter*)sub;
  if (_dbus_message_real_iter_is_zeroed(real) && _dbus_message_real_iter_is_zeroed(real_sub)) return;
#ifdef DBUS_DISABLE_CHECKS
  _dbus_return_if_fail(_dbus_message_iter_append_check(real));
  _dbus_return_if_fail(real->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
#endif
  if (_dbus_message_real_iter_is_zeroed(real_sub)) return;
#ifdef DBUS_DISABLE_CHECKS
  _dbus_return_if_fail(_dbus_message_iter_append_check(real_sub));
  _dbus_return_if_fail(real_sub->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
#endif
  _dbus_message_iter_abandon_signature(real);
  _dbus_message_real_iter_zero(real_sub);
}
void dbus_message_set_no_reply(DBusMessage *message, dbus_bool_t no_reply) {
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(!message->locked);
  _dbus_header_toggle_flag(&message->header, DBUS_HEADER_FLAG_NO_REPLY_EXPECTED, no_reply);
}
dbus_bool_t dbus_message_get_no_reply(DBusMessage *message) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  return _dbus_header_get_flag(&message->header, DBUS_HEADER_FLAG_NO_REPLY_EXPECTED);
}
void dbus_message_set_auto_start(DBusMessage *message, dbus_bool_t auto_start) {
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(!message->locked);
  _dbus_header_toggle_flag(&message->header, DBUS_HEADER_FLAG_NO_AUTO_START, !auto_start);
}
dbus_bool_t dbus_message_get_auto_start(DBusMessage *message) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  return !_dbus_header_get_flag(&message->header, DBUS_HEADER_FLAG_NO_AUTO_START);
}
dbus_bool_t dbus_message_set_path(DBusMessage *message, const char *object_path) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(!message->locked, FALSE);
  _dbus_return_val_if_fail(object_path == NULL || _dbus_check_is_valid_path(object_path), FALSE);
  return set_or_delete_string_field(message, DBUS_HEADER_FIELD_PATH, DBUS_TYPE_OBJECT_PATH, object_path);
}
const char* dbus_message_get_path(DBusMessage *message) {
  const char *v;
  _dbus_return_val_if_fail(message != NULL, NULL);
  v = NULL;
  _dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_PATH, DBUS_TYPE_OBJECT_PATH, (void*)&v);
  return v;
}
dbus_bool_t dbus_message_has_path(DBusMessage *message, const char *path) {
  const char *msg_path;
  msg_path = dbus_message_get_path(message);
  if (msg_path == NULL) {
      if (path == NULL) return TRUE;
      else return FALSE;
  }
  if (path == NULL) return FALSE;
  if (strcmp (msg_path, path) == 0) return TRUE;
  return FALSE;
}
dbus_bool_t dbus_message_get_path_decomposed(DBusMessage *message, char ***path) {
  const char *v;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(path != NULL, FALSE);
  *path = NULL;
  v = dbus_message_get_path(message);
  if (v != NULL) {
      if (!_dbus_decompose_path(v, strlen(v), path, NULL)) return FALSE;
  }
  return TRUE;
}
dbus_bool_t dbus_message_set_interface(DBusMessage *message, const char *iface) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(!message->locked, FALSE);
  _dbus_return_val_if_fail(iface == NULL || _dbus_check_is_valid_interface(iface), FALSE);
  return set_or_delete_string_field(message, DBUS_HEADER_FIELD_INTERFACE, DBUS_TYPE_STRING, iface);
}
const char* dbus_message_get_interface(DBusMessage *message) {
  const char *v;
  _dbus_return_val_if_fail(message != NULL, NULL);
  v = NULL;
  _dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_INTERFACE, DBUS_TYPE_STRING, (void*)&v);
  return v;
}
dbus_bool_t dbus_message_has_interface(DBusMessage *message, const char *iface) {
  const char *msg_interface;
  msg_interface = dbus_message_get_interface(message);
  if (msg_interface == NULL) {
      if (iface == NULL) return TRUE;
      else return FALSE;
  }
  if (iface == NULL) return FALSE;
  if (strcmp(msg_interface, iface) == 0) return TRUE;
  return FALSE;

}
dbus_bool_t dbus_message_set_member(DBusMessage *message, const char *member) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(!message->locked, FALSE);
  _dbus_return_val_if_fail(member == NULL || _dbus_check_is_valid_member(member), FALSE);
  return set_or_delete_string_field(message, DBUS_HEADER_FIELD_MEMBER, DBUS_TYPE_STRING, member);
}
const char* dbus_message_get_member(DBusMessage *message) {
  const char *v;
  _dbus_return_val_if_fail(message != NULL, NULL);
  v = NULL;
  _dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_MEMBER, DBUS_TYPE_STRING, (void*)&v);
  return v;
}
dbus_bool_t dbus_message_has_member(DBusMessage *message, const char *member) {
  const char *msg_member;
  msg_member = dbus_message_get_member(message);
  if (msg_member == NULL) {
      if (member == NULL) return TRUE;
      else return FALSE;
  }
  if (member == NULL) return FALSE;
  if (strcmp(msg_member, member) == 0) return TRUE;
  return FALSE;
}
dbus_bool_t dbus_message_set_error_name(DBusMessage *message, const char *error_name) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(!message->locked, FALSE);
  _dbus_return_val_if_fail(error_name == NULL || _dbus_check_is_valid_error_name(error_name), FALSE);
  return set_or_delete_string_field(message, DBUS_HEADER_FIELD_ERROR_NAME, DBUS_TYPE_STRING, error_name);
}
const char* dbus_message_get_error_name(DBusMessage *message) {
  const char *v;
  _dbus_return_val_if_fail(message != NULL, NULL);
  v = NULL;
  _dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_ERROR_NAME, DBUS_TYPE_STRING, (void*)&v);
  return v;
}
dbus_bool_t dbus_message_set_destination(DBusMessage *message, const char *destination) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(!message->locked, FALSE);
  _dbus_return_val_if_fail(destination == NULL || _dbus_check_is_valid_bus_name(destination), FALSE);
  return set_or_delete_string_field(message, DBUS_HEADER_FIELD_DESTINATION, DBUS_TYPE_STRING, destination);
}
const char* dbus_message_get_destination(DBusMessage *message) {
  const char *v;
  _dbus_return_val_if_fail(message != NULL, NULL);
  v = NULL;
  _dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_DESTINATION, DBUS_TYPE_STRING, (void*)&v);
  return v;
}
dbus_bool_t dbus_message_set_sender(DBusMessage *message, const char *sender) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(!message->locked, FALSE);
  _dbus_return_val_if_fail(sender == NULL || _dbus_check_is_valid_bus_name(sender), FALSE);
  return set_or_delete_string_field (message, DBUS_HEADER_FIELD_SENDER, DBUS_TYPE_STRING, sender);
}
const char* dbus_message_get_sender(DBusMessage *message) {
  const char *v;
  _dbus_return_val_if_fail(message != NULL, NULL);
  v = NULL;
  _dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_SENDER, DBUS_TYPE_STRING, (void*)&v);
  return v;
}
const char* dbus_message_get_signature(DBusMessage *message) {
  const DBusString *type_str;
  int type_pos;
  _dbus_return_val_if_fail(message != NULL, NULL);
  get_const_signature(&message->header, &type_str, &type_pos);
  return _dbus_string_get_const_data_len(type_str, type_pos, 0);
}
static dbus_bool_t _dbus_message_has_type_interface_member(DBusMessage *message, int type, const char *iface, const char *member) {
  const char *n;
  _dbus_assert(message != NULL);
  _dbus_assert(iface != NULL);
  _dbus_assert(member != NULL);
  if (dbus_message_get_type(message) != type) return FALSE;
  n = dbus_message_get_member(message);
  if (n && strcmp(n, member) == 0) {
      n = dbus_message_get_interface(message);
      if (n == NULL || strcmp(n, iface) == 0) return TRUE;
  }
  return FALSE;
}
dbus_bool_t dbus_message_is_method_call(DBusMessage *message, const char *iface, const char *method) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(iface != NULL, FALSE);
  _dbus_return_val_if_fail(method != NULL, FALSE);
  return _dbus_message_has_type_interface_member(message, DBUS_MESSAGE_TYPE_METHOD_CALL, iface, method);
}
dbus_bool_t dbus_message_is_signal(DBusMessage *message, const char *iface, const char *signal_name) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(iface != NULL, FALSE);
  _dbus_return_val_if_fail(signal_name != NULL, FALSE);
  return _dbus_message_has_type_interface_member(message, DBUS_MESSAGE_TYPE_SIGNAL, iface, signal_name);
}
dbus_bool_t dbus_message_is_error(DBusMessage *message, const char *error_name) {
  const char *n;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(error_name != NULL, FALSE);
  if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_ERROR) return FALSE;
  n = dbus_message_get_error_name(message);
  if (n && strcmp(n, error_name) == 0) return TRUE;
  else return FALSE;
}
dbus_bool_t dbus_message_has_destination(DBusMessage *message, const char *name) {
  const char *s;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(name != NULL, FALSE);
  s = dbus_message_get_destination(message);
  if (s && strcmp(s, name) == 0) return TRUE;
  else return FALSE;
}
dbus_bool_t dbus_message_has_sender(DBusMessage *message, const char *name) {
  const char *s;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(name != NULL, FALSE);
  s = dbus_message_get_sender(message);
  if (s && strcmp(s, name) == 0) return TRUE;
  else return FALSE;
}
dbus_bool_t dbus_message_has_signature(DBusMessage *message, const char *signature) {
  const char *s;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(signature != NULL, FALSE);
  s = dbus_message_get_signature(message);
  if (s && strcmp(s, signature) == 0) return TRUE;
  else return FALSE;
}
dbus_bool_t dbus_set_error_from_message(DBusError *error, DBusMessage *message) {
  const char *str;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_error_is_set(error, FALSE);
  if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_ERROR) return FALSE;
  str = NULL;
  dbus_message_get_args(message, NULL, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
  dbus_set_error(error, dbus_message_get_error_name(message),str ? "%s" : NULL, str);
  return TRUE;
}
dbus_bool_t dbus_message_contains_unix_fds(DBusMessage *message) {
#ifdef HAVE_UNIX_FD_PASSING
  _dbus_assert(message);
  return message->n_unix_fds > 0;
#else
  return FALSE;
#endif
}
#define INITIAL_LOADER_DATA_LEN 32
DBusMessageLoader* _dbus_message_loader_new(void) {
  DBusMessageLoader *loader;
  loader = dbus_new0(DBusMessageLoader, 1);
  if (loader == NULL) return NULL;
  loader->refcount = 1;
  loader->corrupted = FALSE;
  loader->corruption_reason = DBUS_VALID;
  loader->max_message_size = DBUS_MAXIMUM_MESSAGE_LENGTH;
  loader->max_message_unix_fds = DBUS_DEFAULT_MESSAGE_UNIX_FDS;
  if (!_dbus_string_init(&loader->data)) {
      dbus_free(loader);
      return NULL;
  }
  _dbus_string_set_length(&loader->data, INITIAL_LOADER_DATA_LEN);
  _dbus_string_set_length(&loader->data, 0);
#ifdef HAVE_UNIX_FD_PASSING
  loader->unix_fds = NULL;
  loader->n_unix_fds = loader->n_unix_fds_allocated = 0;
  loader->unix_fds_outstanding = FALSE;
#endif
  return loader;
}
DBusMessageLoader *_dbus_message_loader_ref(DBusMessageLoader *loader) {
  loader->refcount += 1;
  return loader;
}
void _dbus_message_loader_unref(DBusMessageLoader *loader) {
  loader->refcount -= 1;
  if (loader->refcount == 0) {
  #ifdef HAVE_UNIX_FD_PASSING
      close_unix_fds(loader->unix_fds, &loader->n_unix_fds);
      dbus_free(loader->unix_fds);
  #endif
      _dbus_list_foreach(&loader->messages, (DBusForeachFunction)dbus_message_unref, NULL);
      _dbus_list_clear(&loader->messages);
      _dbus_string_free(&loader->data);
      dbus_free(loader);
    }
}
void _dbus_message_loader_get_buffer(DBusMessageLoader *loader, DBusString **buffer, int *max_to_read, dbus_bool_t *may_read_fds) {
  _dbus_assert(!loader->buffer_outstanding);
  *buffer = &loader->data;
  loader->buffer_outstanding = TRUE;
  if (max_to_read != NULL) {
  #ifdef HAVE_UNIX_FD_PASSING
      int offset = 0;
      int remain;
      int byte_order;
      int fields_array_len;
      int header_len;
      int body_len;
  #endif
      *max_to_read = DBUS_MAXIMUM_MESSAGE_LENGTH;
      *may_read_fds = TRUE;
  #ifdef HAVE_UNIX_FD_PASSING
      if (loader->n_unix_fds == 0) return;
      remain = _dbus_string_get_length(&loader->data);
      while(remain > 0) {
          DBusValidity validity = DBUS_VALIDITY_UNKNOWN;
          int needed;
          if (remain < DBUS_MINIMUM_HEADER_SIZE) {
              *max_to_read = DBUS_MINIMUM_HEADER_SIZE - remain;
              *may_read_fds = FALSE;
              return;
          }
          if (!_dbus_header_have_message_untrusted(loader->max_message_size, &validity, &byte_order, &fields_array_len, &header_len, &body_len, &loader->data,
              offset, remain)) {
              if (validity != DBUS_VALID) return;
              needed = header_len + body_len;
              _dbus_assert(needed > remain);
              *max_to_read = needed - remain;
              *may_read_fds = FALSE;
              return;
          }
          needed = header_len + body_len;
          _dbus_assert(needed > DBUS_MINIMUM_HEADER_SIZE);
          _dbus_assert(remain >= needed);
          remain -= needed;
          offset += needed;
      }
#endif
  }
}
void
_dbus_message_loader_return_buffer(DBusMessageLoader *loader, DBusString *buffer) {
  _dbus_assert(loader->buffer_outstanding);
  _dbus_assert(buffer == &loader->data);
  loader->buffer_outstanding = FALSE;
}
#ifdef HAVE_UNIX_FD_PASSING
dbus_bool_t _dbus_message_loader_get_unix_fds(DBusMessageLoader *loader, int **fds, unsigned *max_n_fds) {
  _dbus_assert(!loader->unix_fds_outstanding);
  if (loader->n_unix_fds_allocated < loader->max_message_unix_fds) {
      int *a = dbus_realloc(loader->unix_fds,loader->max_message_unix_fds * sizeof(loader->unix_fds[0]));
      if (!a) return FALSE;
      loader->unix_fds = a;
      loader->n_unix_fds_allocated = loader->max_message_unix_fds;
  }
  *fds = loader->unix_fds + loader->n_unix_fds;
  *max_n_fds = loader->n_unix_fds_allocated - loader->n_unix_fds;
  loader->unix_fds_outstanding = TRUE;
  return TRUE;
}
void _dbus_message_loader_return_unix_fds(DBusMessageLoader *loader, int *fds, unsigned n_fds) {
  _dbus_assert(loader->unix_fds_outstanding);
  _dbus_assert(loader->unix_fds + loader->n_unix_fds == fds);
  _dbus_assert(loader->n_unix_fds + n_fds <= loader->n_unix_fds_allocated);
  loader->n_unix_fds += n_fds;
  loader->unix_fds_outstanding = FALSE;
  if (n_fds && loader->unix_fds_change) loader->unix_fds_change(loader->unix_fds_change_data);
}
#endif
static dbus_bool_t load_message(DBusMessageLoader *loader, DBusMessage *message, int byte_order, int fields_array_len, int header_len, int body_len) {
  dbus_bool_t oom;
  DBusValidity validity;
  const DBusString *type_str;
  int type_pos;
  DBusValidationMode mode;
  dbus_uint32_t n_unix_fds = 0;
  mode = DBUS_VALIDATION_MODE_DATA_IS_UNTRUSTED;
  oom = FALSE;
#if 0
  _dbus_verbose_bytes_of_string(&loader->data, 0, header_len /* + body_len */);
#endif
  _dbus_assert(_dbus_string_get_length(&message->header.data) == 0);
  _dbus_assert((header_len + body_len) <= _dbus_string_get_length(&loader->data));
  if (!_dbus_header_load(&message->header, mode, &validity, byte_order, fields_array_len, header_len, body_len, &loader->data)) {
      _dbus_verbose("Failed to load header for new message code %d\n", validity);
      _dbus_assert(validity != DBUS_VALID);
      if (validity == DBUS_VALIDITY_UNKNOWN_OOM_ERROR) oom = TRUE;
      else {
          loader->corrupted = TRUE;
          loader->corruption_reason = validity;
      }
      goto failed;
  }
  _dbus_assert(validity == DBUS_VALID);
  if (mode != DBUS_VALIDATION_MODE_WE_TRUST_THIS_DATA_ABSOLUTELY) {
      get_const_signature(&message->header, &type_str, &type_pos);
      validity = _dbus_validate_body_with_reason(type_str, type_pos, byte_order,NULL, &loader->data, header_len, body_len);
      if (validity != DBUS_VALID) {
          _dbus_verbose("Failed to validate message body code %d\n", validity);
          loader->corrupted = TRUE;
          loader->corruption_reason = validity;
          goto failed;
      }
  }
  _dbus_header_get_field_basic(&message->header, DBUS_HEADER_FIELD_UNIX_FDS, DBUS_TYPE_UINT32, &n_unix_fds);
#ifdef HAVE_UNIX_FD_PASSING
  if (n_unix_fds > loader->n_unix_fds) {
      _dbus_verbose("Message contains references to more unix fds than were sent %u != %u\n", n_unix_fds, loader->n_unix_fds);
      loader->corrupted = TRUE;
      loader->corruption_reason = DBUS_INVALID_MISSING_UNIX_FDS;
      goto failed;
  }
  dbus_free(message->unix_fds);
  if (n_unix_fds > 0) {
      message->unix_fds = _dbus_memdup(loader->unix_fds, n_unix_fds * sizeof(message->unix_fds[0]));
      if (message->unix_fds == NULL) {
          _dbus_verbose("Failed to allocate file descriptor array\n");
          oom = TRUE;
          goto failed;
      }
      message->n_unix_fds_allocated = message->n_unix_fds = n_unix_fds;
      loader->n_unix_fds -= n_unix_fds;
      memmove(loader->unix_fds, loader->unix_fds + n_unix_fds, loader->n_unix_fds * sizeof(loader->unix_fds[0]));
      if (loader->unix_fds_change) loader->unix_fds_change(loader->unix_fds_change_data);
  } else message->unix_fds = NULL;
#else
  if (n_unix_fds > 0) {
      _dbus_verbose("Hmm, message claims to come with file descriptors but that's not supported on our platform, disconnecting.\n");
      loader->corrupted = TRUE;
      loader->corruption_reason = DBUS_INVALID_MISSING_UNIX_FDS;
      goto failed;
  }
#endif
  if (!_dbus_list_append(&loader->messages, message)) {
      _dbus_verbose("Failed to append new message to loader queue\n");
      oom = TRUE;
      goto failed;
  }
  _dbus_assert(_dbus_string_get_length(&message->body) == 0);
  _dbus_assert(_dbus_string_get_length(&loader->data) >= (header_len + body_len));
  if (!_dbus_string_copy_len(&loader->data, header_len, body_len, &message->body, 0)) {
      _dbus_verbose("Failed to move body into new message\n");
      oom = TRUE;
      goto failed;
  }
  _dbus_string_delete(&loader->data, 0, header_len + body_len);
  _dbus_string_compact(&loader->data, 2048);
  _dbus_assert(_dbus_string_get_length(&message->header.data) == header_len);
  _dbus_assert(_dbus_string_get_length (&message->body) == body_len);
  _dbus_verbose("Loaded message %p\n", message);
  _dbus_assert(!oom);
  _dbus_assert(!loader->corrupted);
  _dbus_assert(loader->messages != NULL);
  _dbus_assert(_dbus_list_find_last(&loader->messages, message) != NULL);
  return TRUE;
failed:
  _dbus_list_remove_last(&loader->messages, message);
  if (oom) { _dbus_assert(!loader->corrupted); }
  else { _dbus_assert(loader->corrupted); }
  _dbus_verbose_bytes_of_string(&loader->data, 0, _dbus_string_get_length(&loader->data));
  return FALSE;
}
dbus_bool_t _dbus_message_loader_queue_messages(DBusMessageLoader *loader) {
  while(!loader->corrupted && _dbus_string_get_length(&loader->data) >= DBUS_MINIMUM_HEADER_SIZE) {
      DBusValidity validity;
      int byte_order, fields_array_len, header_len, body_len;
      if (_dbus_header_have_message_untrusted(loader->max_message_size, &validity, &byte_order, &fields_array_len, &header_len, &body_len, &loader->data,
          0, _dbus_string_get_length(&loader->data))) {
          DBusMessage *message;
          _dbus_assert(validity == DBUS_VALID);
          message = dbus_message_new_empty_header();
          if (message == NULL) return FALSE;
          if (!load_message(loader, message, byte_order, fields_array_len, header_len, body_len)) {
              dbus_message_unref(message);
              return loader->corrupted;
          }
          _dbus_assert(loader->messages != NULL);
          _dbus_assert(_dbus_list_find_last(&loader->messages, message) != NULL);
      } else {
          _dbus_verbose("Initial peek at header says we don't have a whole message yet, or data broken with invalid code %d\n", validity);
          if (validity != DBUS_VALID) {
              loader->corrupted = TRUE;
              loader->corruption_reason = validity;
          }
          return TRUE;
      }
  }
  return TRUE;
}
DBusMessage* _dbus_message_loader_peek_message(DBusMessageLoader *loader) {
  if (loader->messages) return loader->messages->data;
  else return NULL;
}
DBusMessage* _dbus_message_loader_pop_message(DBusMessageLoader *loader) {
  return _dbus_list_pop_first(&loader->messages);
}
DBusList* _dbus_message_loader_pop_message_link(DBusMessageLoader *loader) {
  return _dbus_list_pop_first_link(&loader->messages);
}
void _dbus_message_loader_putback_message_link(DBusMessageLoader *loader, DBusList *link) {
  _dbus_list_prepend_link(&loader->messages, link);
}
dbus_bool_t _dbus_message_loader_get_is_corrupted(DBusMessageLoader *loader) {
  _dbus_assert((loader->corrupted && loader->corruption_reason != DBUS_VALID) || (!loader->corrupted && loader->corruption_reason == DBUS_VALID));
  return loader->corrupted;
}
DBusValidity _dbus_message_loader_get_corruption_reason(DBusMessageLoader *loader) {
  _dbus_assert((loader->corrupted && loader->corruption_reason != DBUS_VALID) || (!loader->corrupted && loader->corruption_reason == DBUS_VALID));
  return loader->corruption_reason;
}
void _dbus_message_loader_set_max_message_size(DBusMessageLoader *loader, long size) {
  if (size > DBUS_MAXIMUM_MESSAGE_LENGTH) {
      _dbus_verbose("clamping requested max message size %ld to %d\n", size, DBUS_MAXIMUM_MESSAGE_LENGTH);
      size = DBUS_MAXIMUM_MESSAGE_LENGTH;
  }
  loader->max_message_size = size;
}
long _dbus_message_loader_get_max_message_size(DBusMessageLoader *loader) {
  return loader->max_message_size;
}
void _dbus_message_loader_set_max_message_unix_fds(DBusMessageLoader *loader, long n) {
  if (n > DBUS_MAXIMUM_MESSAGE_UNIX_FDS) {
      _dbus_verbose("clamping requested max message unix_fds %ld to %d\n", n, DBUS_MAXIMUM_MESSAGE_UNIX_FDS);
      n = DBUS_MAXIMUM_MESSAGE_UNIX_FDS;
  }
  loader->max_message_unix_fds = n;
}
long _dbus_message_loader_get_max_message_unix_fds(DBusMessageLoader *loader) {
  return loader->max_message_unix_fds;
}
int _dbus_message_loader_get_pending_fds_count(DBusMessageLoader *loader) {
#ifdef HAVE_UNIX_FD_PASSING
  return loader->n_unix_fds;
#else
  return 0;
#endif
}
void _dbus_message_loader_set_pending_fds_function(DBusMessageLoader *loader, void (*callback)(void*), void *data) {
#ifdef HAVE_UNIX_FD_PASSING
  loader->unix_fds_change = callback;
  loader->unix_fds_change_data = data;
#endif
}
static DBusDataSlotAllocator slot_allocator = _DBUS_DATA_SLOT_ALLOCATOR_INIT (_DBUS_LOCK_NAME (message_slots));
dbus_bool_t dbus_message_allocate_data_slot(dbus_int32_t *slot_p) {
  return _dbus_data_slot_allocator_alloc(&slot_allocator, slot_p);
}
void dbus_message_free_data_slot(dbus_int32_t *slot_p) {
  _dbus_return_if_fail(*slot_p >= 0);
  _dbus_data_slot_allocator_free(&slot_allocator, slot_p);
}
dbus_bool_t dbus_message_set_data(DBusMessage *message, dbus_int32_t slot, void *data, DBusFreeFunction free_data_func) {
  DBusFreeFunction old_free_func;
  void *old_data;
  dbus_bool_t retval;
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(slot >= 0, FALSE);
  retval = _dbus_data_slot_list_set (&slot_allocator, &message->slot_list, slot, data, free_data_func, &old_free_func, &old_data);
  if (retval) {
      if (old_free_func) (*old_free_func)(old_data);
  }
  return retval;
}
void* dbus_message_get_data(DBusMessage *message, dbus_int32_t slot) {
  void *res;
  _dbus_return_val_if_fail(message != NULL, NULL);
  res = _dbus_data_slot_list_get(&slot_allocator, &message->slot_list, slot);
  return res;
}
int dbus_message_type_from_string(const char *type_str) {
  if (strcmp(type_str, "method_call") == 0) return DBUS_MESSAGE_TYPE_METHOD_CALL;
  if (strcmp(type_str, "method_return") == 0) return DBUS_MESSAGE_TYPE_METHOD_RETURN;
  else if (strcmp(type_str, "signal") == 0) return DBUS_MESSAGE_TYPE_SIGNAL;
  else if (strcmp(type_str, "error") == 0) return DBUS_MESSAGE_TYPE_ERROR;
  else return DBUS_MESSAGE_TYPE_INVALID;
}
const char *dbus_message_type_to_string(int type) {
  switch(type) {
      case DBUS_MESSAGE_TYPE_METHOD_CALL: return "method_call";
      case DBUS_MESSAGE_TYPE_METHOD_RETURN: return "method_return";
      case DBUS_MESSAGE_TYPE_SIGNAL: return "signal";
      case DBUS_MESSAGE_TYPE_ERROR: return "error";
      default: return "invalid";
  }
}
dbus_bool_t dbus_message_marshal(DBusMessage *msg, char **marshalled_data_p, int *len_p) {
  DBusString tmp;
  dbus_bool_t was_locked;
  _dbus_return_val_if_fail(msg != NULL, FALSE);
  _dbus_return_val_if_fail(marshalled_data_p != NULL, FALSE);
  _dbus_return_val_if_fail(len_p != NULL, FALSE);
  if (!_dbus_string_init(&tmp)) return FALSE;
  was_locked = msg->locked;
  if (!was_locked) dbus_message_lock(msg);
  if (!_dbus_string_copy(&(msg->header.data), 0, &tmp, 0)) goto fail;
  *len_p = _dbus_string_get_length(&tmp);
  if (!_dbus_string_copy(&(msg->body), 0, &tmp, *len_p)) goto fail;
  *len_p = _dbus_string_get_length(&tmp);
  if (!_dbus_string_steal_data(&tmp, marshalled_data_p)) goto fail;
  _dbus_string_free(&tmp);
  if (!was_locked) msg->locked = FALSE;
  return TRUE;
fail:
  _dbus_string_free(&tmp);
  if (!was_locked) msg->locked = FALSE;
  return FALSE;
}
DBusMessage *dbus_message_demarshal(const char *str, int len, DBusError *error) {
  DBusMessageLoader *loader = NULL;
  DBusString *buffer;
  DBusMessage *msg;
  _dbus_return_val_if_fail(str != NULL, NULL);
  loader = _dbus_message_loader_new();
  if (loader == NULL) goto fail_oom;
  _dbus_message_loader_get_buffer(loader, &buffer, NULL, NULL);
  if (!_dbus_string_append_len(buffer, str, len)) goto fail_oom;
  _dbus_message_loader_return_buffer(loader, buffer);
  if (!_dbus_message_loader_queue_messages(loader)) goto fail_oom;
  if (_dbus_message_loader_get_is_corrupted(loader)) goto fail_corrupt;
  msg = _dbus_message_loader_pop_message(loader);
  if (!msg) goto fail_oom;
  _dbus_message_loader_unref(loader);
  return msg;
fail_corrupt:
  dbus_set_error(error, DBUS_ERROR_INVALID_ARGS, "Message is corrupted (%s)", _dbus_validity_to_error_message(loader->corruption_reason));
  _dbus_message_loader_unref(loader);
  return NULL;
fail_oom:
  _DBUS_SET_OOM(error);
  if (loader != NULL) _dbus_message_loader_unref(loader);
  return NULL;
}
int dbus_message_demarshal_bytes_needed(const char *buf, int len) {
  DBusString str;
  int byte_order, fields_array_len, header_len, body_len;
  DBusValidity validity = DBUS_VALID;
  int have_message;
  if (!buf || len < DBUS_MINIMUM_HEADER_SIZE) return 0;
  if (len > DBUS_MAXIMUM_MESSAGE_LENGTH) len = DBUS_MAXIMUM_MESSAGE_LENGTH;
  _dbus_string_init_const_len(&str, buf, len);
  validity = DBUS_VALID;
  have_message = _dbus_header_have_message_untrusted(DBUS_MAXIMUM_MESSAGE_LENGTH, &validity, &byte_order, &fields_array_len, &header_len,
                                                     &body_len, &str, 0, len);
  _dbus_string_free (&str);
  if (validity == DBUS_VALID) {
      _dbus_assert(have_message || (header_len + body_len) > len);
      (void)have_message;
      return header_len + body_len;
  } else return -1;
}
void dbus_message_set_allow_interactive_authorization(DBusMessage *message, dbus_bool_t allow) {
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(!message->locked);
  _dbus_header_toggle_flag(&message->header, DBUS_HEADER_FLAG_ALLOW_INTERACTIVE_AUTHORIZATION, allow);
}
dbus_bool_t dbus_message_get_allow_interactive_authorization(DBusMessage *message) {
  _dbus_return_val_if_fail(message != NULL, FALSE);
  return _dbus_header_get_flag(&message->header, DBUS_HEADER_FLAG_ALLOW_INTERACTIVE_AUTHORIZATION);
}
struct DBusVariant {
  DBusString data;
};
DBusVariant *_dbus_variant_read(DBusMessageIter *reader) {
  DBusVariant *self = NULL;
  DBusMessageRealIter *real_reader = (DBusMessageRealIter*)reader;
  DBusTypeWriter items_writer;
  DBusTypeWriter variant_writer;
  DBusString variant_signature;
  DBusString contained_signature;
  dbus_bool_t data_inited = FALSE;
  int type;
  const DBusString *sig;
  int start, len;
  _dbus_assert(_dbus_message_iter_check(real_reader));
  _dbus_assert(real_reader->iter_type == DBUS_MESSAGE_ITER_TYPE_READER);
  _dbus_string_init_const(&variant_signature, DBUS_TYPE_VARIANT_AS_STRING);
  type = dbus_message_iter_get_arg_type(reader);
  _dbus_type_reader_get_signature(&real_reader->u.reader, &sig, &start, &len);
  if (!_dbus_string_init(&contained_signature)) return NULL;
  if (!_dbus_string_copy_len (sig, start, len, &contained_signature, 0)) goto oom;
  self = dbus_new0(DBusVariant, 1);
  if (self == NULL) goto oom;
  if (!_dbus_string_init(&self->data)) goto oom;
  data_inited = TRUE;
  _dbus_type_writer_init_values_only(&items_writer, DBUS_COMPILER_BYTE_ORDER, &variant_signature, 0, &self->data, 0);
  if (!_dbus_type_writer_recurse(&items_writer, DBUS_TYPE_VARIANT, &contained_signature, 0, &variant_writer)) goto oom;
  if (type == DBUS_TYPE_ARRAY) {
      DBusMessageIter array_reader;
      DBusMessageRealIter *real_array_reader = (DBusMessageRealIter*)&array_reader;
      DBusTypeWriter array_writer;
      dbus_message_iter_recurse(reader, &array_reader);
      if (!_dbus_type_writer_recurse(&variant_writer, type, &contained_signature, 1, &array_writer)) goto oom;
      if (!_dbus_type_writer_write_reader(&array_writer, &real_array_reader->u.reader)) goto oom;
      if (!_dbus_type_writer_unrecurse(&variant_writer, &array_writer)) goto oom;
  } else if (type == DBUS_TYPE_DICT_ENTRY || type == DBUS_TYPE_VARIANT || type == DBUS_TYPE_STRUCT) {
      DBusMessageIter inner_reader;
      DBusMessageRealIter *real_inner_reader = (DBusMessageRealIter*)&inner_reader;
      DBusTypeWriter inner_writer;
      dbus_message_iter_recurse(reader, &inner_reader);
      if (!_dbus_type_writer_recurse(&variant_writer, type, NULL, 0, &inner_writer)) goto oom;
      if (!_dbus_type_writer_write_reader(&inner_writer, &real_inner_reader->u.reader)) goto oom;
      if (!_dbus_type_writer_unrecurse(&variant_writer, &inner_writer)) goto oom;
  } else {
      DBusBasicValue value;
      _dbus_assert(dbus_type_is_basic(type));
      dbus_message_iter_get_basic(reader, &value);
      if (!_dbus_type_writer_write_basic(&variant_writer, type, &value)) goto oom;
  }
  _dbus_string_free(&contained_signature);
  return self;
oom:
  if (self != NULL) {
      if (data_inited) _dbus_string_free(&self->data);
      dbus_free (self);
  }
  _dbus_string_free(&contained_signature);
  return NULL;
}
const char *_dbus_variant_get_signature(DBusVariant *self) {
  unsigned char len;
  const char *ret;
  _dbus_assert(self != NULL);
  len = _dbus_string_get_byte(&self->data, 0);
  ret = _dbus_string_get_const_data_len(&self->data, 1, len);
  _dbus_assert(strlen(ret) == len);
  return ret;
}
dbus_bool_t _dbus_variant_write (DBusVariant *self, DBusMessageIter *writer) {
  DBusString variant_signature;
  DBusTypeReader variant_reader;
  DBusTypeReader reader;
  DBusMessageRealIter *real_writer = (DBusMessageRealIter*)writer;
  dbus_bool_t ret;
  _dbus_assert(self != NULL);
  _dbus_assert(_dbus_message_iter_append_check(real_writer));
  _dbus_assert(real_writer->iter_type == DBUS_MESSAGE_ITER_TYPE_WRITER);
  _dbus_string_init_const(&variant_signature, DBUS_TYPE_VARIANT_AS_STRING);
  _dbus_type_reader_init(&reader, DBUS_COMPILER_BYTE_ORDER, &variant_signature, 0, &self->data, 0);
  _dbus_type_reader_recurse(&reader, &variant_reader);
  if (!_dbus_message_iter_open_signature(real_writer)) return FALSE;
  ret = _dbus_type_writer_write_reader(&real_writer->u.writer, &variant_reader);
  if (!_dbus_message_iter_close_signature(real_writer)) return FALSE;
  return ret;
}
int _dbus_variant_get_length(DBusVariant *self) {
  _dbus_assert(self != NULL);
  return _dbus_string_get_length(&self->data);
}
const DBusString *_dbus_variant_peek(DBusVariant *self) {
  _dbus_assert(self != NULL);
  return &self->data;
}
void _dbus_variant_free(DBusVariant *self) {
  _dbus_assert(self != NULL);
  _dbus_string_free(&self->data);
  dbus_free(self);
}
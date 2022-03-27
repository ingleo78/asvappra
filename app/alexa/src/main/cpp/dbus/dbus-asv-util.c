#include "config.h"
#include "dbus.h"
#include "dbus-asv-util.h"

DBusMessage *_dbus_asv_new_method_return(DBusMessage *message, DBusMessageIter *iter, DBusMessageIter *arr_iter) {
  DBusMessage *reply = dbus_message_new_method_return(message);
  if (reply == NULL) return NULL;
  dbus_message_iter_init_append(reply, iter);
  if (!dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", arr_iter)) {
      dbus_message_unref(reply);
      return NULL;
  }
  return reply;
}
dbus_bool_t _dbus_asv_open_entry(DBusMessageIter *arr_iter, DBusMessageIter *entry_iter, const char *key, const char *type, DBusMessageIter *var_iter) {
  if (!dbus_message_iter_open_container(arr_iter, DBUS_TYPE_DICT_ENTRY,NULL, entry_iter)) return FALSE;
  if (!dbus_message_iter_append_basic(entry_iter, DBUS_TYPE_STRING, &key)) {
      dbus_message_iter_abandon_container(arr_iter, entry_iter);
      return FALSE;
  }
  if (!dbus_message_iter_open_container(entry_iter, DBUS_TYPE_VARIANT, type, var_iter)) {
      dbus_message_iter_abandon_container(arr_iter, entry_iter);
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_asv_close_entry(DBusMessageIter *arr_iter, DBusMessageIter *entry_iter, DBusMessageIter *var_iter) {
  if (!dbus_message_iter_close_container(entry_iter, var_iter)) {
      dbus_message_iter_abandon_container(arr_iter, entry_iter);
      return FALSE;
  }
  if (!dbus_message_iter_close_container(arr_iter, entry_iter)) return FALSE;
  return TRUE;
}
dbus_bool_t _dbus_asv_close(DBusMessageIter *iter, DBusMessageIter *arr_iter) {
  return dbus_message_iter_close_container(iter, arr_iter);
}
void _dbus_asv_abandon_entry(DBusMessageIter *arr_iter, DBusMessageIter *entry_iter, DBusMessageIter *var_iter) {
  dbus_message_iter_abandon_container(entry_iter, var_iter);
  dbus_message_iter_abandon_container(arr_iter, entry_iter);
}
void _dbus_asv_abandon(DBusMessageIter *iter, DBusMessageIter *arr_iter) {
  dbus_message_iter_abandon_container(iter, arr_iter);
}
dbus_bool_t _dbus_asv_add_uint32(DBusMessageIter *arr_iter, const char *key, dbus_uint32_t value) {
  DBusMessageIter entry_iter, var_iter;
  if (!_dbus_asv_open_entry(arr_iter, &entry_iter, key, DBUS_TYPE_UINT32_AS_STRING, &var_iter)) return FALSE;
  if (!dbus_message_iter_append_basic(&var_iter, DBUS_TYPE_UINT32, &value)) {
      _dbus_asv_abandon_entry(arr_iter, &entry_iter, &var_iter);
      return FALSE;
  }
  if (!_dbus_asv_close_entry(arr_iter, &entry_iter, &var_iter)) return FALSE;
  return TRUE;
}
dbus_bool_t _dbus_asv_add_string(DBusMessageIter *arr_iter, const char *key, const char *value) {
  DBusMessageIter entry_iter, var_iter;
  if (!_dbus_asv_open_entry(arr_iter, &entry_iter, key, DBUS_TYPE_STRING_AS_STRING, &var_iter)) return FALSE;
  if (!dbus_message_iter_append_basic(&var_iter, DBUS_TYPE_STRING, &value)) {
      _dbus_asv_abandon_entry(arr_iter, &entry_iter, &var_iter);
      return FALSE;
  }
  if (!_dbus_asv_close_entry(arr_iter, &entry_iter, &var_iter)) return FALSE;
  return TRUE;
}
dbus_bool_t _dbus_asv_add_object_path(DBusMessageIter *arr_iter, const char *key, const char *value) {
  DBusMessageIter entry_iter, var_iter;
  if (!_dbus_asv_open_entry(arr_iter, &entry_iter, key, DBUS_TYPE_OBJECT_PATH_AS_STRING, &var_iter)) return FALSE;
  if (!dbus_message_iter_append_basic(&var_iter, DBUS_TYPE_OBJECT_PATH, &value)) {
      _dbus_asv_abandon_entry(arr_iter, &entry_iter, &var_iter);
      return FALSE;
  }
  if (!_dbus_asv_close_entry(arr_iter, &entry_iter, &var_iter)) return FALSE;
  return TRUE;
}
dbus_bool_t _dbus_asv_add_byte_array(DBusMessageIter *arr_iter, const char *key, const void *value, int n_elements) {
  DBusMessageIter entry_iter;
  DBusMessageIter var_iter;
  DBusMessageIter byte_array_iter;
  if (!_dbus_asv_open_entry(arr_iter, &entry_iter, key, "ay", &var_iter)) return FALSE;
  if (!dbus_message_iter_open_container(&var_iter, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &byte_array_iter)) {
      _dbus_asv_abandon_entry(arr_iter, &entry_iter, &var_iter);
      return FALSE;
  }
  if (!dbus_message_iter_append_fixed_array(&byte_array_iter, DBUS_TYPE_BYTE, &value, n_elements)) {
      dbus_message_iter_abandon_container(&var_iter, &byte_array_iter);
      _dbus_asv_abandon_entry(arr_iter, &entry_iter, &var_iter);
      return FALSE;
  }
  if (!dbus_message_iter_close_container (&var_iter, &byte_array_iter)) {
      _dbus_asv_abandon_entry(arr_iter, &entry_iter, &var_iter);
      return FALSE;
  }
  if (!_dbus_asv_close_entry(arr_iter, &entry_iter, &var_iter)) return FALSE;
  return TRUE;
}
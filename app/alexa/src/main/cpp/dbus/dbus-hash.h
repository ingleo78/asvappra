#ifndef DBUS_HASH_H
#define DBUS_HASH_H

#include <stdint.h>
#include <inttypes.h>
#include "dbus-memory.h"
#include "dbus-types.h"
#include "dbus-sysdeps.h"

DBUS_BEGIN_DECLS
struct DBusHashIter {
  void *dummy1;
  void *dummy2;
  void *dummy3;
  void *dummy4;
  int dummy5;
  int dummy6;
};
typedef struct DBusHashTable DBusHashTable;
typedef struct DBusHashIter DBusHashIter;
typedef enum {
  DBUS_HASH_STRING,
  DBUS_HASH_INT,
  DBUS_HASH_UINTPTR
} DBusHashType;
DBUS_PRIVATE_EXPORT DBusHashTable* _dbus_hash_table_new(DBusHashType type, DBusFreeFunction key_free_function, DBusFreeFunction value_free_function);
DBusHashTable* _dbus_hash_table_ref(DBusHashTable *table);
DBUS_PRIVATE_EXPORT void _dbus_hash_table_unref(DBusHashTable *table);
void _dbus_hash_table_remove_all(DBusHashTable *table);
DBUS_PRIVATE_EXPORT void _dbus_hash_iter_init(DBusHashTable *table, DBusHashIter *iter);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_iter_next(DBusHashIter *iter);
DBUS_PRIVATE_EXPORT void _dbus_hash_iter_remove_entry(DBusHashIter *iter);
DBUS_PRIVATE_EXPORT void* _dbus_hash_iter_get_value(DBusHashIter *iter);
DBUS_PRIVATE_EXPORT void _dbus_hash_iter_set_value(DBusHashIter *iter, void *value);
DBUS_PRIVATE_EXPORT int _dbus_hash_iter_get_int_key(DBusHashIter *iter);
DBUS_PRIVATE_EXPORT const char* _dbus_hash_iter_get_string_key(DBusHashIter *iter);
DBUS_PRIVATE_EXPORT uintptr_t _dbus_hash_iter_get_uintptr_key(DBusHashIter *iter);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_iter_lookup(DBusHashTable *table, void *key, dbus_bool_t create_if_not_found, DBusHashIter *iter);
DBUS_PRIVATE_EXPORT void* _dbus_hash_table_lookup_string(DBusHashTable *table, const char *key);
DBUS_PRIVATE_EXPORT void* _dbus_hash_table_lookup_int(DBusHashTable *table, int key);
DBUS_PRIVATE_EXPORT void* _dbus_hash_table_lookup_uintptr(DBusHashTable *table, uintptr_t key);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_table_remove_string(DBusHashTable *table, const char *key);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_table_remove_int(DBusHashTable *table, int key);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_table_remove_uintptr(DBusHashTable *table, uintptr_t key);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_table_insert_string(DBusHashTable *table, char *key, void *value);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_table_insert_int(DBusHashTable *table, int key, void *value);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_table_insert_uintptr(DBusHashTable *table, uintptr_t key, void *value);
DBUS_PRIVATE_EXPORT int _dbus_hash_table_get_n_entries(DBusHashTable *table);
DBUS_PRIVATE_EXPORT char **_dbus_hash_table_to_array(DBusHashTable *table, char delimiter);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_hash_table_from_array(DBusHashTable *table, char **array, char delimiter);
typedef struct DBusPreallocatedHash DBusPreallocatedHash;
DBUS_PRIVATE_EXPORT DBusPreallocatedHash *_dbus_hash_table_preallocate_entry(DBusHashTable *table);
DBUS_PRIVATE_EXPORT void _dbus_hash_table_free_preallocated_entry(DBusHashTable *table, DBusPreallocatedHash *preallocated);
DBUS_PRIVATE_EXPORT void _dbus_hash_table_insert_string_preallocated(DBusHashTable *table, DBusPreallocatedHash *preallocated, char *key, void *value);
#ifdef DBUS_WIN
#define DBUS_HASH_POLLABLE DBUS_HASH_UINTPTR
#else
#define DBUS_HASH_POLLABLE DBUS_HASH_INT
#endif
static inline DBusPollable _dbus_hash_iter_get_pollable_key(DBusHashIter *iter) {
#ifdef DBUS_WIN
  DBusSocket s;
  s.sock = _dbus_hash_iter_get_uintptr_key(iter);
  return s;
#else
  return _dbus_hash_iter_get_int_key(iter);
#endif
}
static inline void *_dbus_hash_table_lookup_pollable(DBusHashTable *table, DBusPollable key) {
#ifdef DBUS_WIN
  return _dbus_hash_table_lookup_uintptr(table, key.sock);
#else
  return _dbus_hash_table_lookup_int(table, key);
#endif
}
static inline dbus_bool_t _dbus_hash_table_remove_pollable(DBusHashTable *table, DBusPollable key) {
#ifdef DBUS_WIN
  return _dbus_hash_table_remove_uintptr(table, key.sock);
#else
  return _dbus_hash_table_remove_int(table, key);
#endif
}
static inline dbus_bool_t _dbus_hash_table_insert_pollable(DBusHashTable *table, DBusPollable key, void *value) {
#ifdef DBUS_WIN
  return _dbus_hash_table_insert_uintptr(table, key.sock, value);
#else
  return _dbus_hash_table_insert_int(table, key, value);
#endif
}
static inline void _dbus_clear_hash_table(DBusHashTable **table_p) {
  _dbus_clear_pointer_impl(DBusHashTable, table_p, _dbus_hash_table_unref);
}
DBUS_END_DECLS

#endif
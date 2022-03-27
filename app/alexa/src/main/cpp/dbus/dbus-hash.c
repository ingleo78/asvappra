#include "config.h"
#include "dbus-hash.h"
#include "dbus-internals.h"
#include "dbus-mempool.h"
#include "dbus-test-tap.h"

#define REBUILD_MULTIPLIER  3
#define RANDOM_INDEX(table, i)  (((((intptr_t)(i))*1103515245) >> (table)->down_shift) & (table)->mask)
#define DBUS_SMALL_HASH_TABLE 4
typedef struct DBusHashEntry DBusHashEntry;
struct DBusHashEntry {
  DBusHashEntry *next;
  void *key;
  void *value;
};
typedef DBusHashEntry* (*DBusFindEntryFunction)(DBusHashTable *table, void *key, dbus_bool_t create_if_not_found, DBusHashEntry ***bucket,
                                                DBusPreallocatedHash *preallocated);
struct DBusHashTable {
  int refcount;
  DBusHashEntry **buckets;
  DBusHashEntry *static_buckets[DBUS_SMALL_HASH_TABLE];
  int n_buckets;
  int n_entries;
  int hi_rebuild_size;
  int lo_rebuild_size;
  int down_shift;
  int mask;
  DBusHashType key_type;
  DBusFindEntryFunction find_function;
  DBusFreeFunction free_key_function;
  DBusFreeFunction free_value_function;
  DBusMemPool *entry_pool;
};
typedef struct {
  DBusHashTable *table;
  DBusHashEntry **bucket;
  DBusHashEntry *entry;
  DBusHashEntry *next_entry;
  int next_bucket;
  int n_entries_on_init;
} DBusRealHashIter;
_DBUS_STATIC_ASSERT(sizeof(DBusRealHashIter) == sizeof(DBusHashIter));
static DBusHashEntry* find_direct_function(DBusHashTable *table, void *key, dbus_bool_t create_if_not_found, DBusHashEntry ***bucket, DBusPreallocatedHash *preallocated);
static DBusHashEntry* find_string_function(DBusHashTable *table, void *key, dbus_bool_t create_if_not_found, DBusHashEntry ***bucket, DBusPreallocatedHash *preallocated);
static unsigned int string_hash(const char  *str);
static void rebuild_table(DBusHashTable *table);
static DBusHashEntry* alloc_entry(DBusHashTable *table);
static void remove_entry(DBusHashTable *table, DBusHashEntry **bucket, DBusHashEntry *entry);
static void free_entry(DBusHashTable *table, DBusHashEntry *entry);
static void free_entry_data(DBusHashTable *table, DBusHashEntry *entry);
DBusHashTable* _dbus_hash_table_new(DBusHashType type, DBusFreeFunction key_free_function, DBusFreeFunction value_free_function) {
  DBusHashTable *table;
  DBusMemPool *entry_pool;
  table = dbus_new0(DBusHashTable, 1);
  if (table == NULL) return NULL;
  entry_pool = _dbus_mem_pool_new(sizeof(DBusHashEntry), TRUE);
  if (entry_pool == NULL) {
      dbus_free(table);
      return NULL;
  }
  table->refcount = 1;
  table->entry_pool = entry_pool;
  _dbus_assert(DBUS_SMALL_HASH_TABLE == _DBUS_N_ELEMENTS(table->static_buckets));
  table->buckets = table->static_buckets;  
  table->n_buckets = DBUS_SMALL_HASH_TABLE;
  table->n_entries = 0;
  table->hi_rebuild_size = DBUS_SMALL_HASH_TABLE * REBUILD_MULTIPLIER;
  table->lo_rebuild_size = 0;
  table->down_shift = 28;
  table->mask = 3;
  table->key_type = type;
  _dbus_assert(table->mask < table->n_buckets);
  switch(table->key_type) {
    case DBUS_HASH_INT: case DBUS_HASH_UINTPTR: table->find_function = find_direct_function; break;
    case DBUS_HASH_STRING: table->find_function = find_string_function; break;
    default: _dbus_assert_not_reached("Unknown hash table type");
  }
  table->free_key_function = key_free_function;
  table->free_value_function = value_free_function;
  return table;
}
DBusHashTable *_dbus_hash_table_ref(DBusHashTable *table) {
  table->refcount += 1;
  return table;
}
void _dbus_hash_table_unref(DBusHashTable *table) {
  table->refcount -= 1;
  if (table->refcount == 0) {
  #if 0
      DBusHashEntry *entry;
      DBusHashEntry *next;
      int i;
      for (i = 0; i < table->n_buckets; i++) {
          entry = table->buckets[i];
          while (entry != NULL) {
              next = entry->next;
              free_entry (table, entry);
              entry = next;
          }
      }
  #else
      DBusHashEntry *entry;
      int i;
      for (i = 0; i < table->n_buckets; i++) {
          entry = table->buckets[i];
          while(entry != NULL) {
              free_entry_data(table, entry);
              entry = entry->next;
          }
      }
      _dbus_mem_pool_free(table->entry_pool);
  #endif
      if (table->buckets != table->static_buckets) dbus_free(table->buckets);
      dbus_free(table);
  }
}
void _dbus_hash_table_remove_all(DBusHashTable *table) {
  DBusHashIter iter;
  _dbus_hash_iter_init(table, &iter);
  while(_dbus_hash_iter_next(&iter)) _dbus_hash_iter_remove_entry(&iter);
}
static DBusHashEntry* alloc_entry(DBusHashTable *table) {
  DBusHashEntry *entry;
  entry = _dbus_mem_pool_alloc(table->entry_pool);
  return entry;
}
static void free_entry_data(DBusHashTable *table, DBusHashEntry *entry) {
  if (table->free_key_function) (*table->free_key_function)(entry->key);
  if (table->free_value_function) (*table->free_value_function)(entry->value);
}
static void free_entry(DBusHashTable *table, DBusHashEntry *entry) {
  free_entry_data(table, entry);
  _dbus_mem_pool_dealloc(table->entry_pool, entry);
}
static void remove_entry(DBusHashTable *table, DBusHashEntry **bucket, DBusHashEntry *entry) {
  _dbus_assert(table != NULL);
  _dbus_assert(bucket != NULL);
  _dbus_assert(*bucket != NULL);
  _dbus_assert(entry != NULL);
  if (*bucket == entry) *bucket = entry->next;
  else {
      DBusHashEntry *prev;
      prev = *bucket;
      while(prev->next != entry) prev = prev->next;
      _dbus_assert(prev != NULL);
      prev->next = entry->next;
  }
  table->n_entries -= 1;
  free_entry(table, entry);
}
void _dbus_hash_iter_init(DBusHashTable *table, DBusHashIter *iter) {
  DBusRealHashIter *real;
  _DBUS_STATIC_ASSERT(sizeof(DBusHashIter) == sizeof(DBusRealHashIter));
  real = (DBusRealHashIter*)iter;
  real->table = table;
  real->bucket = NULL;
  real->entry = NULL;
  real->next_entry = NULL;
  real->next_bucket = 0;
  real->n_entries_on_init = table->n_entries;
}
dbus_bool_t _dbus_hash_iter_next(DBusHashIter *iter) {
  DBusRealHashIter *real;
  _DBUS_STATIC_ASSERT(sizeof(DBusHashIter) == sizeof(DBusRealHashIter));
  real = (DBusRealHashIter*)iter;
  _dbus_assert(real->n_entries_on_init >= real->table->n_entries);
  while(real->next_entry == NULL) {
      if (real->next_bucket >= real->table->n_buckets) {
          real->entry = NULL;
          real->table = NULL;
          real->bucket = NULL;
          return FALSE;
      }
      real->bucket = &(real->table->buckets[real->next_bucket]);
      real->next_entry = *(real->bucket);
      real->next_bucket += 1;
  }
  _dbus_assert(real->next_entry != NULL);
  _dbus_assert(real->bucket != NULL);
  real->entry = real->next_entry;
  real->next_entry = real->entry->next;
  return TRUE;
}
void _dbus_hash_iter_remove_entry(DBusHashIter *iter) {
  DBusRealHashIter *real;
  real = (DBusRealHashIter*)iter;
  _dbus_assert(real->table != NULL);
  _dbus_assert(real->entry != NULL);
  _dbus_assert(real->bucket != NULL);
  remove_entry(real->table, real->bucket, real->entry);
  real->entry = NULL;
}
void* _dbus_hash_iter_get_value(DBusHashIter *iter) {
  DBusRealHashIter *real;
  real = (DBusRealHashIter*)iter;
  _dbus_assert(real->table != NULL);
  _dbus_assert(real->entry != NULL);
  return real->entry->value;
}
void _dbus_hash_iter_set_value(DBusHashIter *iter, void *value) {
  DBusRealHashIter *real;
  real = (DBusRealHashIter*)iter;
  _dbus_assert(real->table != NULL);
  _dbus_assert(real->entry != NULL);
  if (real->table->free_value_function && value != real->entry->value) (*real->table->free_value_function)(real->entry->value);
  real->entry->value = value;
}
int _dbus_hash_iter_get_int_key(DBusHashIter *iter) {
  DBusRealHashIter *real;
  real = (DBusRealHashIter*)iter;
  _dbus_assert(real->table != NULL);
  _dbus_assert(real->entry != NULL);
  return _DBUS_POINTER_TO_INT(real->entry->key);
}
uintptr_t _dbus_hash_iter_get_uintptr_key(DBusHashIter *iter) {
  DBusRealHashIter *real;
  real = (DBusRealHashIter*)iter;
  _dbus_assert(real->table != NULL);
  _dbus_assert(real->entry != NULL);
  return (uintptr_t)real->entry->key;
}
const char* _dbus_hash_iter_get_string_key(DBusHashIter *iter) {
  DBusRealHashIter *real;
  real = (DBusRealHashIter*)iter;
  _dbus_assert(real->table != NULL);
  _dbus_assert(real->entry != NULL);
  return real->entry->key;
}
dbus_bool_t _dbus_hash_iter_lookup (DBusHashTable *table, void *key, dbus_bool_t create_if_not_found, DBusHashIter *iter) {
  DBusRealHashIter *real;
  DBusHashEntry *entry;
  DBusHashEntry **bucket;
  _DBUS_STATIC_ASSERT(sizeof(DBusHashIter) == sizeof(DBusRealHashIter));
  real = (DBusRealHashIter*)iter;
  entry = (*table->find_function)(table, key, create_if_not_found, &bucket, NULL);
  if (entry == NULL) return FALSE;
  if (create_if_not_found) {
      if (table->free_key_function && entry->key != key) (*table->free_key_function)(entry->key);
      entry->key = key;
  }
  real->table = table;
  real->bucket = bucket;
  real->entry = entry;
  real->next_entry = entry->next;
  real->next_bucket = (bucket - table->buckets) + 1;
  real->n_entries_on_init = table->n_entries;
  _dbus_assert(&(table->buckets[real->next_bucket-1]) == real->bucket);
  return TRUE;
}
static void add_allocated_entry(DBusHashTable *table, DBusHashEntry *entry, unsigned int idx, void *key, DBusHashEntry ***bucket) {
  DBusHashEntry **b;
  entry->key = key;
  b = &(table->buckets[idx]);
  entry->next = *b;
  *b = entry;
  if (bucket) *bucket = b;
  table->n_entries += 1;
  if (table->n_entries >= table->hi_rebuild_size || table->n_entries < table->lo_rebuild_size) rebuild_table(table);
}
static DBusHashEntry* add_entry(DBusHashTable *table, unsigned int idx, void *key, DBusHashEntry ***bucket, DBusPreallocatedHash *preallocated) {
  DBusHashEntry  *entry;
  if (preallocated == NULL) {
      entry = alloc_entry(table);
      if (entry == NULL) {
          if (bucket) *bucket = NULL;
          return NULL;
      }
  } else entry = (DBusHashEntry*)preallocated;
  add_allocated_entry(table, entry, idx, key, bucket);
  return entry;
}
static unsigned int string_hash(const char *str) {
  const char *p = str;
  unsigned int h = *p;
  if (h)
      for (p += 1; *p != '\0'; p++) h = (h << 5) - h + *p;
  return h;
}
typedef int (*KeyCompareFunc)(const void *key_a, const void *key_b);
static DBusHashEntry* find_generic_function(DBusHashTable *table, void *key, unsigned int idx, KeyCompareFunc compare_func, dbus_bool_t create_if_not_found,
                                            DBusHashEntry ***bucket, DBusPreallocatedHash *preallocated) {
  DBusHashEntry *entry;
  if (bucket) *bucket = NULL;
  entry = table->buckets[idx];
  while (entry != NULL) {
      if ((compare_func == NULL && key == entry->key) || (compare_func != NULL && (*compare_func)(key, entry->key) == 0)) {
          if (bucket) *bucket = &(table->buckets[idx]);
          if (preallocated) _dbus_hash_table_free_preallocated_entry(table, preallocated);
          return entry;
      }
      entry = entry->next;
  }
  if (create_if_not_found) entry = add_entry(table, idx, key, bucket, preallocated);
  else if (preallocated) _dbus_hash_table_free_preallocated_entry(table, preallocated);
  return entry;
}
static DBusHashEntry* find_string_function(DBusHashTable *table, void *key, dbus_bool_t create_if_not_found, DBusHashEntry ***bucket,
                                           DBusPreallocatedHash *preallocated) {
  unsigned int idx;
  idx = string_hash(key) & table->mask;
  return find_generic_function(table, key, idx, (KeyCompareFunc)strcmp, create_if_not_found, bucket, preallocated);
}
static DBusHashEntry* find_direct_function(DBusHashTable *table, void *key, dbus_bool_t create_if_not_found, DBusHashEntry ***bucket,
                                           DBusPreallocatedHash *preallocated) {
  unsigned int idx;
  idx = RANDOM_INDEX(table, key) & table->mask;
  return find_generic_function(table, key, idx,NULL, create_if_not_found, bucket, preallocated);
}
static void rebuild_table(DBusHashTable *table) {
  int old_size;
  int new_buckets;
  DBusHashEntry **old_buckets;
  DBusHashEntry **old_chain;
  DBusHashEntry *entry;
  dbus_bool_t growing;
  growing = table->n_entries >= table->hi_rebuild_size;
  old_size = table->n_buckets;
  old_buckets = table->buckets;
  if (growing) {
      if (table->n_buckets < _DBUS_INT_MAX / 4 && table->down_shift >= 2) new_buckets = table->n_buckets * 4;
      else return;
  } else {
      new_buckets = table->n_buckets / 4;
      if (new_buckets < DBUS_SMALL_HASH_TABLE) return;
  }
  table->buckets = dbus_new0(DBusHashEntry*, new_buckets);
  if (table->buckets == NULL) {
      table->buckets = old_buckets;
      return;
  }
  table->n_buckets = new_buckets;
  if (growing) {
      table->lo_rebuild_size = table->hi_rebuild_size;
      table->hi_rebuild_size *= 4;
      table->down_shift -= 2;
      table->mask = (table->mask << 2) + 3;
  } else {
      table->hi_rebuild_size = table->lo_rebuild_size;
      table->lo_rebuild_size /= 4;
      table->down_shift += 2;
      table->mask = table->mask >> 2;
  }
#if 0
  printf("%s table to lo = %d hi = %d downshift = %d mask = 0x%x\n", growing ? "GROW" : "SHRINK", table->lo_rebuild_size, table->hi_rebuild_size,
         table->down_shift, table->mask);
#endif
  _dbus_assert(table->lo_rebuild_size >= 0);
  _dbus_assert(table->hi_rebuild_size > table->lo_rebuild_size);
  _dbus_assert(table->down_shift >= 0);
  _dbus_assert(table->mask != 0);
  _dbus_assert(table->mask < table->n_buckets);
  for (old_chain = old_buckets; old_size > 0; old_size--, old_chain++) {
      for (entry = *old_chain; entry != NULL; entry = *old_chain) {
          unsigned int idx;
          DBusHashEntry **bucket;
          *old_chain = entry->next;
          switch(table->key_type) {
              case DBUS_HASH_STRING: idx = string_hash (entry->key) & table->mask; break;
              case DBUS_HASH_INT: case DBUS_HASH_UINTPTR: idx = RANDOM_INDEX(table, entry->key); break;
              default:
                  idx = 0;
                  _dbus_assert_not_reached("Unknown hash table type");
                  break;
          }
          bucket = &(table->buckets[idx]);
          entry->next = *bucket;
          *bucket = entry;
      }
  }
  if (old_buckets != table->static_buckets) dbus_free(old_buckets);
}
void* _dbus_hash_table_lookup_string(DBusHashTable *table, const char *key) {
  DBusHashEntry *entry;
  _dbus_assert(table->key_type == DBUS_HASH_STRING);
  entry = (*table->find_function) (table, (char*)key, FALSE, NULL, NULL);
  if (entry) return entry->value;
  else return NULL;
}
void* _dbus_hash_table_lookup_int(DBusHashTable *table, int key) {
  DBusHashEntry *entry;
  _dbus_assert(table->key_type == DBUS_HASH_INT);
  entry = (*table->find_function)(table, _DBUS_INT_TO_POINTER(key), FALSE, NULL, NULL);
  if (entry) return entry->value;
  else return NULL;
}
void* _dbus_hash_table_lookup_uintptr(DBusHashTable *table, uintptr_t key) {
  DBusHashEntry *entry;
  _dbus_assert(table->key_type == DBUS_HASH_UINTPTR);
  entry = (*table->find_function)(table, (void*)key, FALSE, NULL, NULL);
  if (entry) return entry->value;
  else return NULL;
}
dbus_bool_t _dbus_hash_table_remove_string(DBusHashTable *table, const char *key) {
  DBusHashEntry *entry;
  DBusHashEntry **bucket;
  _dbus_assert(table->key_type == DBUS_HASH_STRING);
  entry = (*table->find_function)(table, (char*)key, FALSE, &bucket, NULL);
  if (entry) {
      remove_entry(table, bucket, entry);
      return TRUE;
  } else return FALSE;
}
dbus_bool_t _dbus_hash_table_remove_int(DBusHashTable *table, int key) {
  DBusHashEntry *entry;
  DBusHashEntry **bucket;
  _dbus_assert(table->key_type == DBUS_HASH_INT);
  entry = (*table->find_function)(table, _DBUS_INT_TO_POINTER(key), FALSE, &bucket, NULL);
  if (entry) {
      remove_entry(table, bucket, entry);
      return TRUE;
  } else return FALSE;
}
dbus_bool_t _dbus_hash_table_remove_uintptr(DBusHashTable *table, uintptr_t key) {
  DBusHashEntry *entry;
  DBusHashEntry **bucket;
  _dbus_assert(table->key_type == DBUS_HASH_UINTPTR);
  entry = (*table->find_function)(table, (void*)key, FALSE, &bucket, NULL);
  if (entry) {
      remove_entry(table, bucket, entry);
      return TRUE;
  } else return FALSE;
}
dbus_bool_t _dbus_hash_table_insert_string(DBusHashTable *table, char *key, void *value) {
  DBusPreallocatedHash *preallocated;
  _dbus_assert(table->key_type == DBUS_HASH_STRING);
  preallocated = _dbus_hash_table_preallocate_entry(table);
  if (preallocated == NULL) return FALSE;
  _dbus_hash_table_insert_string_preallocated(table, preallocated, key, value);
  return TRUE;
}
dbus_bool_t _dbus_hash_table_insert_int(DBusHashTable *table, int key, void *value) {
  DBusHashEntry *entry;
  _dbus_assert(table->key_type == DBUS_HASH_INT);
  entry = (*table->find_function)(table, _DBUS_INT_TO_POINTER(key), TRUE, NULL, NULL);
  if (entry == NULL) return FALSE;
  if (table->free_key_function && entry->key != _DBUS_INT_TO_POINTER(key)) (*table->free_key_function)(entry->key);
  if (table->free_value_function && entry->value != value) (*table->free_value_function)(entry->value);
  entry->key = _DBUS_INT_TO_POINTER(key);
  entry->value = value;
  return TRUE;
}
dbus_bool_t _dbus_hash_table_insert_uintptr(DBusHashTable *table, uintptr_t key, void *value) {
  DBusHashEntry *entry;
  _dbus_assert(table->key_type == DBUS_HASH_UINTPTR);
  entry = (*table->find_function)(table, (void*)key, TRUE, NULL, NULL);
  if (entry == NULL) return FALSE;
  if (table->free_key_function && entry->key != (void*)key) (*table->free_key_function)(entry->key);
  if (table->free_value_function && entry->value != value) (*table->free_value_function)(entry->value);
  entry->key = (void*)key;
  entry->value = value;
  return TRUE;
}
DBusPreallocatedHash* _dbus_hash_table_preallocate_entry(DBusHashTable *table) {
  DBusHashEntry *entry;
  entry = alloc_entry(table);
  return (DBusPreallocatedHash*)entry;
}
void _dbus_hash_table_free_preallocated_entry(DBusHashTable *table, DBusPreallocatedHash *preallocated) {
  DBusHashEntry *entry;
  _dbus_assert(preallocated != NULL);
  entry = (DBusHashEntry*)preallocated;
  _dbus_mem_pool_dealloc(table->entry_pool, entry);
}
void _dbus_hash_table_insert_string_preallocated(DBusHashTable *table, DBusPreallocatedHash *preallocated, char *key, void *value) {
  DBusHashEntry *entry;
  _dbus_assert(table->key_type == DBUS_HASH_STRING);
  _dbus_assert(preallocated != NULL);
  entry = (*table->find_function)(table, key, TRUE, NULL, preallocated);
  _dbus_assert(entry != NULL);
  if (table->free_key_function && entry->key != key) (*table->free_key_function)(entry->key);
  if (table->free_value_function && entry->value != value) (*table->free_value_function)(entry->value);
  entry->key = key;
  entry->value = value;
}
int _dbus_hash_table_get_n_entries(DBusHashTable *table) {
  return table->n_entries;
}
dbus_bool_t _dbus_hash_table_from_array(DBusHashTable *table, char **array, char delimiter) {
  DBusString key;
  DBusString value;
  int i;
  dbus_bool_t retval = FALSE;
  _dbus_assert(table != NULL);
  _dbus_assert(array != NULL);
  if (!_dbus_string_init(&key)) return FALSE;
  if (!_dbus_string_init(&value)) {
      _dbus_string_free(&key);
      return FALSE;
  }
  for (i = 0; array[i] != NULL; i++) {
      if (!_dbus_string_append(&key, array[i])) break;
      if (_dbus_string_split_on_byte(&key, delimiter, &value)) {
          char *hash_key, *hash_value;
          if (!_dbus_string_steal_data(&key, &hash_key)) break;
          if (!_dbus_string_steal_data(&value, &hash_value)) break;
          if (!_dbus_hash_table_insert_string(table, hash_key, hash_value)) break;
      }
      _dbus_string_set_length(&key, 0);
      _dbus_string_set_length(&value, 0);
  }
  if (array[i] != NULL) goto out;
  retval = TRUE;
out:
  _dbus_string_free(&key);
  _dbus_string_free(&value);
  return retval;
}
char **_dbus_hash_table_to_array(DBusHashTable *table, char delimiter) {
  int i, length;
  DBusString entry;
  DBusHashIter iter;
  char **array;
  _dbus_assert(table != NULL);
  length = _dbus_hash_table_get_n_entries(table);
  array = dbus_new0(char*, length + 1);
  if (array == NULL) return NULL;
  i = 0;
  _dbus_hash_iter_init(table, &iter);
  if (!_dbus_string_init(&entry)) {
      dbus_free_string_array(array);
      return NULL;
  }
  while(_dbus_hash_iter_next(&iter)) {
      const char *key, *value;
      key = (const char*)_dbus_hash_iter_get_string_key(&iter);
      value = (const char*)_dbus_hash_iter_get_value(&iter);
      if (!_dbus_string_append_printf(&entry, "%s%c%s", key, delimiter, value)) break;
      if (!_dbus_string_steal_data(&entry, array + i)) break;
      i++;
  }
  _dbus_string_free(&entry);
  if (i != length) {
      dbus_free_string_array(array);
      array = NULL;
  }
  return array;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus-test.h"
#include <stdio.h>

static int count_entries(DBusHashTable *table) {
  DBusHashIter iter;
  int count;
  count = 0;
  _dbus_hash_iter_init(table, &iter);
  while(_dbus_hash_iter_next(&iter)) ++count;
  _dbus_assert(count == _dbus_hash_table_get_n_entries(table));
  return count;
}
static inline void *steal(void *ptr) {
  void **_ptr = (void**)ptr;
  void *val;
  val = *_ptr;
  *_ptr = NULL;
  return val;
}
dbus_bool_t _dbus_hash_test(void) {
  int i;
  DBusHashTable *table1;
  DBusHashTable *table2;
  DBusHashTable *table3;
  DBusHashIter iter;
#define N_HASH_KEYS 5000
  char **keys;
  dbus_bool_t ret = FALSE;
  char *str_key = NULL;
  char *str_value = NULL;
  keys = dbus_new(char*, N_HASH_KEYS);
  if (keys == NULL) _dbus_test_fatal("no memory");
  for (i = 0; i < N_HASH_KEYS; i++) {
      keys[i] = dbus_malloc(128);
      if (keys[i] == NULL) _dbus_test_fatal("no memory");
  }
  _dbus_test_diag("Computing test hash keys...");
  i = 0;
  while(i < N_HASH_KEYS) {
      int len;
      len = sprintf(keys[i], "Hash key %d", i);
      _dbus_assert(*(keys[i] + len) == '\0');
      ++i;
  }
  _dbus_test_diag("... done.");
  table1 = _dbus_hash_table_new(DBUS_HASH_STRING, dbus_free, dbus_free);
  if (table1 == NULL) goto out;
  table2 = _dbus_hash_table_new(DBUS_HASH_INT,NULL, dbus_free);
  if (table2 == NULL) goto out;
  table3 = _dbus_hash_table_new(DBUS_HASH_UINTPTR,NULL, dbus_free);
  if (table3 == NULL) goto out;
  i = 0;
  while(i < 3000) {
      const void *out_value;
      str_key = _dbus_strdup(keys[i]);
      if (str_key == NULL) goto out;
      str_value = _dbus_strdup("Value!");
      if (str_value == NULL) goto out;
      if (!_dbus_hash_table_insert_string(table1, steal(&str_key), steal(&str_value))) goto out;
      str_value = _dbus_strdup(keys[i]);
      if (str_value == NULL) goto out;
      if (!_dbus_hash_table_insert_int(table2, i, steal (&str_value))) goto out;
      str_value = _dbus_strdup(keys[i]);
      if (str_value == NULL) goto out;
      if (!_dbus_hash_table_insert_uintptr(table3, i, steal (&str_value))) goto out;
      _dbus_assert(count_entries(table1) == i + 1);
      _dbus_assert(count_entries(table2) == i + 1);
      _dbus_assert(count_entries(table3) == i + 1);
      out_value = _dbus_hash_table_lookup_string(table1, keys[i]);
      _dbus_assert(out_value != NULL);
      _dbus_assert(strcmp(out_value, "Value!") == 0);
      out_value = _dbus_hash_table_lookup_int(table2, i);
      _dbus_assert(out_value != NULL);
      _dbus_assert(strcmp(out_value, keys[i]) == 0);
      out_value = _dbus_hash_table_lookup_uintptr(table3, i);
      _dbus_assert(out_value != NULL);
      _dbus_assert(strcmp(out_value, keys[i]) == 0);
      ++i;
  }
  --i;
  while(i >= 0) {
      _dbus_hash_table_remove_string(table1, keys[i]);
      _dbus_hash_table_remove_int(table2, i);
      _dbus_hash_table_remove_uintptr(table3, i);
      _dbus_assert(count_entries(table1) == i);
      _dbus_assert(count_entries(table2) == i);
      _dbus_assert(count_entries(table3) == i);
      --i;
  }
  _dbus_hash_table_ref(table1);
  _dbus_hash_table_ref(table2);
  _dbus_hash_table_ref(table3);
  _dbus_hash_table_unref(table1);
  _dbus_hash_table_unref(table2);
  _dbus_hash_table_unref(table3);
  _dbus_hash_table_unref(table1);
  _dbus_hash_table_unref(table2);
  _dbus_hash_table_unref(table3);
  table3 = NULL;
  table1 = _dbus_hash_table_new(DBUS_HASH_STRING, dbus_free, dbus_free);
  if (table1 == NULL) goto out;
  table2 = _dbus_hash_table_new(DBUS_HASH_INT,NULL, dbus_free);
  if (table2 == NULL) goto out;
  i = 0;
  while(i < 5000) {
      str_key = _dbus_strdup(keys[i]);
      if (str_key == NULL) goto out;
      str_value = _dbus_strdup("Value!");
      if (str_value == NULL) goto out;
      if (!_dbus_hash_table_insert_string(table1, steal(&str_key), steal(&str_value))) goto out;
      str_value = _dbus_strdup(keys[i]);
      if (str_value == NULL) goto out;
      if (!_dbus_hash_table_insert_int(table2, i, steal(&str_value))) goto out;
      _dbus_assert(count_entries(table1) == i + 1);
      _dbus_assert(count_entries(table2) == i + 1);
      ++i;
  }
  _dbus_hash_iter_init(table1, &iter);
  while(_dbus_hash_iter_next(&iter)) {
      const char *key;
      const void *value;
      key = _dbus_hash_iter_get_string_key(&iter);
      value = _dbus_hash_iter_get_value(&iter);
      _dbus_assert(_dbus_hash_table_lookup_string(table1, key) == value);
      str_value = _dbus_strdup("Different value!");
      if (str_value == NULL) goto out;
      value = str_value;
      _dbus_hash_iter_set_value(&iter, steal(&str_value));
      _dbus_assert(_dbus_hash_table_lookup_string(table1, key) == value);
  }
  _dbus_hash_iter_init(table1, &iter);
  while(_dbus_hash_iter_next(&iter)) {
      _dbus_hash_iter_remove_entry(&iter);
      _dbus_assert(count_entries(table1) == i - 1);
      --i;
  }
  _dbus_hash_iter_init(table2, &iter);
  while(_dbus_hash_iter_next(&iter)) {
      int key;
      const void *value;
      key = _dbus_hash_iter_get_int_key(&iter);
      value = _dbus_hash_iter_get_value(&iter);
      _dbus_assert(_dbus_hash_table_lookup_int(table2, key) == value);
      str_value = _dbus_strdup("Different value!");
      if (str_value == NULL) goto out;
      value = str_value;
      _dbus_hash_iter_set_value(&iter, steal(&str_value));
      _dbus_assert(_dbus_hash_table_lookup_int(table2, key) == value);
  }
  i = count_entries(table2);
  _dbus_hash_iter_init(table2, &iter);
  while(_dbus_hash_iter_next(&iter)) {
      _dbus_hash_iter_remove_entry(&iter);
      _dbus_assert(count_entries(table2) + 1 == i);
      --i;
  }
  i = 0;
  while(i < 1000) {
      str_key = _dbus_strdup(keys[i]);
      if (str_key == NULL) goto out;
      str_value = _dbus_strdup("Value!");
      if (str_value == NULL) goto out;
      if (!_dbus_hash_table_insert_string(table1, steal(&str_key), steal(&str_value))) goto out;
      ++i;
  }
  --i;
  while(i >= 0) {
      str_key = _dbus_strdup(keys[i]);
      if (str_key == NULL) goto out;
      str_value = _dbus_strdup("Value!");
      if (str_value == NULL) goto out;
      if (!_dbus_hash_table_remove_string(table1, keys[i])) goto out;
      if (!_dbus_hash_table_insert_string(table1, steal(&str_key), steal(&str_value))) goto out;
      if (!_dbus_hash_table_remove_string(table1, keys[i])) goto out;
      _dbus_assert(_dbus_hash_table_get_n_entries(table1) == i);
      --i;
  }
  _dbus_hash_table_unref(table1);
  _dbus_hash_table_unref(table2);
  table1 = _dbus_hash_table_new(DBUS_HASH_STRING, dbus_free, dbus_free);
  if (table1 == NULL) goto out;
  table2 = _dbus_hash_table_new(DBUS_HASH_INT,NULL, dbus_free);
  if (table2 == NULL) goto out;
  i = 0;
  while(i < 3000) {
      const void *out_value;
      str_key = _dbus_strdup(keys[i]);
      if (str_key == NULL) goto out;
      str_value = _dbus_strdup("Value!");
      if (str_value == NULL) goto out;
      if (!_dbus_hash_iter_lookup(table1, steal(&str_key), TRUE, &iter)) goto out;
      _dbus_assert(_dbus_hash_iter_get_value(&iter) == NULL);
      _dbus_hash_iter_set_value (&iter, steal(&str_value));
      str_value = _dbus_strdup(keys[i]);
      if (str_value == NULL) goto out;
      if (!_dbus_hash_iter_lookup(table2, _DBUS_INT_TO_POINTER(i), TRUE, &iter)) goto out;
      _dbus_assert(_dbus_hash_iter_get_value(&iter) == NULL);
      _dbus_hash_iter_set_value(&iter, steal(&str_value));
      _dbus_assert(count_entries(table1) == i + 1);
      _dbus_assert(count_entries(table2) == i + 1);
      if (!_dbus_hash_iter_lookup(table1, keys[i], FALSE, &iter)) goto out;
      out_value = _dbus_hash_iter_get_value(&iter);
      _dbus_assert(out_value != NULL);
      _dbus_assert(strcmp(out_value, "Value!") == 0);
      while(_dbus_hash_iter_next(&iter));
      if (!_dbus_hash_iter_lookup(table2, _DBUS_INT_TO_POINTER(i), FALSE, &iter)) goto out;
      out_value = _dbus_hash_iter_get_value(&iter);
      _dbus_assert(out_value != NULL);
      _dbus_assert(strcmp(out_value, keys[i]) == 0);
      while(_dbus_hash_iter_next(&iter));
      ++i;
  }
  --i;
  while(i >= 0) {
      if (!_dbus_hash_iter_lookup(table1, keys[i], FALSE, &iter)) _dbus_test_fatal("hash entry should have existed");
      _dbus_hash_iter_remove_entry(&iter);
      if (!_dbus_hash_iter_lookup(table2, _DBUS_INT_TO_POINTER(i), FALSE, &iter)) _dbus_test_fatal("hash entry should have existed");
      _dbus_hash_iter_remove_entry(&iter);
      _dbus_assert(count_entries(table1) == i);
      _dbus_assert(count_entries(table2) == i);
      --i;
  }
  _dbus_hash_table_unref(table1);
  _dbus_hash_table_unref(table2);
  ret = TRUE;
out:
  for (i = 0; i < N_HASH_KEYS; i++) dbus_free(keys[i]);
  dbus_free(keys);
  dbus_free(str_key);
  dbus_free(str_value);
  return ret;
}
#endif
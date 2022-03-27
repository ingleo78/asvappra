#include <string.h>
#include <unistd.h>
#include "config.h"
#define DBUS_USERDB_INCLUDES_PRIVATE 1
#include "dbus-userdb.h"
#include "dbus-test.h"
#include "dbus-internals.h"
#include "dbus-protocol.h"
#include "dbus-test-tap.h"

#if defined(DBUS_WIN) || !defined(DBUS_UNIX)
#error "This file only makes sense on Unix OSs"
#endif

dbus_bool_t _dbus_is_console_user(dbus_uid_t uid, DBusError *error) {
  DBusUserDatabase *db;
  const DBusUserInfo *info;
  dbus_bool_t result = FALSE;
#ifdef HAVE_SYSTEMD
  if (access("/run/systemd/seats/", F_OK) >= 0) {
      int r;
      r = sd_uid_get_seats(uid, 0, NULL);
      if (r < 0) {
          dbus_set_error(error, _dbus_error_from_errno(-r), "Failed to determine seats of user \"" DBUS_UID_FORMAT "\": %s", uid, _dbus_strerror(-r));
          return FALSE;
      }
      return (r > 0);
  }
#endif
#ifdef HAVE_CONSOLE_OWNER_FILE
  DBusString f;
  DBusStat st;
  if (!_dbus_string_init(&f)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_string_append(&f, DBUS_CONSOLE_OWNER_FILE)) {
      _dbus_string_free(&f);
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (_dbus_stat(&f, &st, NULL) && (st.uid == uid)) {
      _dbus_string_free(&f);
      return TRUE;
  }
  _dbus_string_free(&f);
#endif
  if (!_dbus_user_database_lock_system()) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  db = _dbus_user_database_get_system();
  if (db == NULL) {
      dbus_set_error(error, DBUS_ERROR_FAILED, "Could not get system database.");
      _dbus_user_database_unlock_system();
      return FALSE;
  }
  info = _dbus_user_database_lookup(db, uid, NULL, error);
  if (info == NULL) {
      _dbus_user_database_unlock_system();
       return FALSE;
  }
  result = _dbus_user_at_console(info->username, error);
  _dbus_user_database_unlock_system();
  return result;
}
dbus_bool_t _dbus_get_user_id(const DBusString *username, dbus_uid_t *uid) {
  return _dbus_get_user_id_and_primary_group(username, uid, NULL);
}
dbus_bool_t _dbus_get_group_id(const DBusString *groupname, dbus_gid_t *gid) {
  DBusUserDatabase *db;
  const DBusGroupInfo *info;
  if (!_dbus_user_database_lock_system()) return FALSE;
  db = _dbus_user_database_get_system();
  if (db == NULL) {
      _dbus_user_database_unlock_system();
      return FALSE;
  }
  if (!_dbus_user_database_get_groupname(db, groupname, &info, NULL)) {
      _dbus_user_database_unlock_system();
      return FALSE;
  }
  *gid = info->gid;
  _dbus_user_database_unlock_system();
  return TRUE;
}
dbus_bool_t _dbus_get_user_id_and_primary_group(const DBusString *username, dbus_uid_t *uid_p, dbus_gid_t *gid_p) {
  DBusUserDatabase *db;
  const DBusUserInfo *info;
  if (!_dbus_user_database_lock_system()) return FALSE;
  db = _dbus_user_database_get_system();
  if (db == NULL) {
      _dbus_user_database_unlock_system();
      return FALSE;
  }
  if (!_dbus_user_database_get_username(db, username, &info, NULL)) {
      _dbus_user_database_unlock_system();
      return FALSE;
  }
  if (uid_p) *uid_p = info->uid;
  if (gid_p) *gid_p = info->primary_gid;
  _dbus_user_database_unlock_system();
  return TRUE;
}
DBusGroupInfo* _dbus_user_database_lookup_group(DBusUserDatabase *db, dbus_gid_t gid, const DBusString *groupname, DBusError *error) {
  DBusGroupInfo *info;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
   if (gid == DBUS_UID_UNSET) {
       unsigned long n;
       if (_dbus_is_a_number(groupname, &n)) gid = n;
   }
  if (gid != DBUS_GID_UNSET) info = _dbus_hash_table_lookup_uintptr(db->groups, gid);
  else info = _dbus_hash_table_lookup_string(db->groups_by_name, _dbus_string_get_const_data(groupname));
  if (info) {
      _dbus_verbose("Using cache for GID "DBUS_GID_FORMAT" information\n", info->gid);
      return info;
  } else {
      if (gid != DBUS_GID_UNSET) _dbus_verbose("No cache for GID "DBUS_GID_FORMAT"\n", gid);
      else _dbus_verbose("No cache for groupname \"%s\"\n", _dbus_string_get_const_data(groupname));
      info = dbus_new0(DBusGroupInfo, 1);
      if (info == NULL) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          return NULL;
      }
      if (gid != DBUS_GID_UNSET) {
          if (!_dbus_group_info_fill_gid(info, gid, error)) {
              _DBUS_ASSERT_ERROR_IS_SET(error);
              _dbus_group_info_free_allocated(info);
              return NULL;
          }
      } else {
          if (!_dbus_group_info_fill(info, groupname, error)) {
              _DBUS_ASSERT_ERROR_IS_SET(error);
              _dbus_group_info_free_allocated(info);
              return NULL;
          }
      }
      gid = DBUS_GID_UNSET;
      groupname = NULL;
      if (!_dbus_hash_table_insert_uintptr(db->groups, info->gid, info)) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          _dbus_group_info_free_allocated(info);
          return NULL;
      }
      if (!_dbus_hash_table_insert_string(db->groups_by_name, info->groupname, info)) {
          _dbus_hash_table_remove_uintptr(db->groups, info->gid);
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          return NULL;
      }
      return info;
  }
}
dbus_bool_t _dbus_user_database_get_groupname(DBusUserDatabase *db, const DBusString *groupname, const DBusGroupInfo **info, DBusError *error) {
  *info = _dbus_user_database_lookup_group(db, DBUS_GID_UNSET, groupname, error);
  return *info != NULL;
}
dbus_bool_t _dbus_user_database_get_gid(DBusUserDatabase *db, dbus_gid_t gid, const DBusGroupInfo **info, DBusError *error) {
  *info = _dbus_user_database_lookup_group(db, gid, NULL, error);
  return *info != NULL;
}
dbus_bool_t _dbus_groups_from_uid(dbus_uid_t uid, dbus_gid_t **group_ids, int *n_group_ids) {
  DBusUserDatabase *db;
  const DBusUserInfo *info;
  *group_ids = NULL;
  *n_group_ids = 0;
  if (!_dbus_user_database_lock_system()) return FALSE;
  db = _dbus_user_database_get_system();
  if (db == NULL) {
      _dbus_user_database_unlock_system();
      return FALSE;
  }
  if (!_dbus_user_database_get_uid(db, uid, &info, NULL)) {
      _dbus_user_database_unlock_system();
      return FALSE;
  }
  _dbus_assert(info->uid == uid);
  if (info->n_group_ids > 0) {
      *group_ids = dbus_new(dbus_gid_t, info->n_group_ids);
      if (*group_ids == NULL) {
	  _dbus_user_database_unlock_system();
          return FALSE;
      }
      *n_group_ids = info->n_group_ids;
      memcpy(*group_ids, info->group_ids, info->n_group_ids * sizeof(dbus_gid_t));
  }
  _dbus_user_database_unlock_system();
  return TRUE;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>

dbus_bool_t _dbus_userdb_test(const char *test_data_dir) {
  const DBusString *username;
  const DBusString *homedir;
  dbus_uid_t uid;
  unsigned long *group_ids;
  int n_group_ids, i;
  DBusError error;
  if (!_dbus_username_from_current_process(&username)) _dbus_test_fatal("didn't get username");
  if (!_dbus_homedir_from_current_process(&homedir)) _dbus_test_fatal("didn't get homedir");
  if (!_dbus_get_user_id(username, &uid)) _dbus_test_fatal("didn't get uid");
  if (!_dbus_groups_from_uid(uid, &group_ids, &n_group_ids)) _dbus_test_fatal("didn't get groups");
  _dbus_test_diag("    Current user: %s homedir: %s gids:", _dbus_string_get_const_data(username), _dbus_string_get_const_data(homedir));
  for (i=0; i<n_group_ids; i++) _dbus_test_diag("- %ld", group_ids[i]);
  dbus_error_init(&error);
  _dbus_test_diag("Is Console user: %i", _dbus_is_console_user(uid, &error));
  _dbus_test_diag("Invocation was OK: %s", error.message ? error.message : "yes");
  dbus_error_free(&error);
  _dbus_test_diag("Is Console user 4711: %i", _dbus_is_console_user(4711, &error));
  _dbus_test_diag("Invocation was OK: %s", error.message ? error.message : "yes");
  dbus_error_free(&error);
  dbus_free(group_ids);
  return TRUE;
}
#endif
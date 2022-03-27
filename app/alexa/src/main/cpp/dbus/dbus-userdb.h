#ifndef DBUS_USERDB_H
#define DBUS_USERDB_H

#include "dbus-sysdeps-unix.h"

#ifdef DBUS_WIN
#error "Don't include this on Windows"
#endif

DBUS_BEGIN_DECLS
typedef struct DBusUserDatabase DBusUserDatabase;
#ifdef DBUS_USERDB_INCLUDES_PRIVATE
#include "dbus-hash.h"

struct DBusUserDatabase {
  int refcount;
  DBusHashTable *users;
  DBusHashTable *groups;
  DBusHashTable *users_by_name;
  DBusHashTable *groups_by_name;

};
DBusUserDatabase* _dbus_user_database_new(void);
DBusUserDatabase* _dbus_user_database_ref(DBusUserDatabase *db);
void _dbus_user_database_flush(DBusUserDatabase *db);
void _dbus_user_database_unref(DBusUserDatabase *db);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_user_database_get_uid(DBusUserDatabase *db, dbus_uid_t uid, const DBusUserInfo **info, DBusError *error);
dbus_bool_t _dbus_user_database_get_gid(DBusUserDatabase *db, dbus_gid_t gid, const DBusGroupInfo **info, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_user_database_get_username(DBusUserDatabase *db, const DBusString *username, const DBusUserInfo **info, DBusError *error);
dbus_bool_t _dbus_user_database_get_groupname(DBusUserDatabase *db, const DBusString *groupname, const DBusGroupInfo **info, DBusError *error);
DBUS_PRIVATE_EXPORT DBusUserInfo* _dbus_user_database_lookup(DBusUserDatabase *db, dbus_uid_t uid, const DBusString *username, DBusError *error);
DBUS_PRIVATE_EXPORT DBusGroupInfo* _dbus_user_database_lookup_group(DBusUserDatabase *db, dbus_gid_t gid, const DBusString *groupname, DBusError *error);
DBUS_PRIVATE_EXPORT void _dbus_user_info_free_allocated(DBusUserInfo *info);
DBUS_PRIVATE_EXPORT void _dbus_group_info_free_allocated(DBusGroupInfo *info);
#endif
DBUS_PRIVATE_EXPORT DBusUserDatabase* _dbus_user_database_get_system(void);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_user_database_lock_system(void) _DBUS_GNUC_WARN_UNUSED_RESULT;
DBUS_PRIVATE_EXPORT void _dbus_user_database_unlock_system(void);
void _dbus_user_database_flush_system(void);
dbus_bool_t _dbus_get_user_id(const DBusString *username, dbus_uid_t *uid);
dbus_bool_t _dbus_get_group_id(const DBusString *group_name, dbus_gid_t *gid);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_get_user_id_and_primary_group (const DBusString *username, dbus_uid_t *uid_p, dbus_gid_t *gid_p);
dbus_bool_t _dbus_credentials_from_uid(dbus_uid_t user_id, DBusCredentials *credentials);
dbus_bool_t _dbus_groups_from_uid(dbus_uid_t uid, dbus_gid_t **group_ids, int *n_group_ids);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_is_console_user(dbus_uid_t uid, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_is_a_number(const DBusString *str, unsigned long *num);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_username_from_current_process(const DBusString **username);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_homedir_from_current_process(const DBusString **homedir);
dbus_bool_t _dbus_homedir_from_username(const DBusString *username, DBusString *homedir);
dbus_bool_t _dbus_homedir_from_uid(dbus_uid_t uid, DBusString *homedir);
DBUS_END_DECLS

#endif
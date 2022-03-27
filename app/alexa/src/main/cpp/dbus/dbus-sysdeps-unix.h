#ifndef DBUS_SYSDEPS_UNIX_H
#define DBUS_SYSDEPS_UNIX_H

#include "dbus-sysdeps.h"

#ifdef DBUS_WIN
#error "Don't include this on Windows"
#endif

DBUS_BEGIN_DECLS
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_close(int fd, DBusError *error);
DBUS_PRIVATE_EXPORT int _dbus_dup(int fd, DBusError *error);
DBUS_PRIVATE_EXPORT int _dbus_read(int fd, DBusString *buffer, int count);
int _dbus_write(int fd, const DBusString *buffer, int start, int len);
int _dbus_write_two(int fd, const DBusString *buffer1, int start1, int len1, const DBusString *buffer2, int start2, int len2);
int _dbus_connect_unix_socket(const char *path, dbus_bool_t abstract, DBusError *error);
int _dbus_listen_unix_socket (const char *path, dbus_bool_t abstract, DBusError *error);
int _dbus_connect_exec(const char *path, char *const argv[], DBusError *error);
int _dbus_listen_systemd_sockets(DBusSocket **fd, DBusError *error);
dbus_bool_t _dbus_read_credentials(int client_fd, DBusCredentials *credentials, DBusError *error);
dbus_bool_t _dbus_send_credentials(int server_fd, DBusError *error);
dbus_bool_t _dbus_lookup_launchd_socket(DBusString *socket_path, const char *launchd_env_var, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_lookup_user_bus(dbus_bool_t *supported, DBusString *address, DBusError *error);
typedef struct DBusUserInfo  DBusUserInfo;
typedef struct DBusGroupInfo DBusGroupInfo;
struct DBusUserInfo {
  dbus_uid_t uid;
  dbus_gid_t primary_gid;
  dbus_gid_t *group_ids;
  int n_group_ids;
  char *username;
  char *homedir;
};
struct DBusGroupInfo {
  dbus_gid_t  gid;
  char *groupname;
};
dbus_bool_t _dbus_user_info_fill(DBusUserInfo *info, const DBusString *username, DBusError *error);
dbus_bool_t _dbus_user_info_fill_uid(DBusUserInfo *info, dbus_uid_t uid, DBusError *error);
void _dbus_user_info_free(DBusUserInfo *info);
dbus_bool_t _dbus_group_info_fill(DBusGroupInfo *info, const DBusString *groupname, DBusError *error);
dbus_bool_t _dbus_group_info_fill_gid(DBusGroupInfo *info, dbus_gid_t gid, DBusError *error);
void _dbus_group_info_free(DBusGroupInfo *info);
DBUS_PRIVATE_EXPORT dbus_uid_t _dbus_geteuid(void);
dbus_bool_t _dbus_parse_uid(const DBusString *uid_str, dbus_uid_t *uid);
DBUS_PRIVATE_EXPORT void _dbus_close_all(void);
dbus_bool_t _dbus_append_address_from_socket(DBusSocket fd, DBusString *address, DBusError *error);
DBUS_PRIVATE_EXPORT void _dbus_fd_set_close_on_exec(int fd);
typedef enum {
  DBUS_FORCE_STDIN_NULL = (1 << 0),
  DBUS_FORCE_STDOUT_NULL = (1 << 1),
  DBUS_FORCE_STDERR_NULL = (1 << 2)
} DBusEnsureStandardFdsFlags;
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_ensure_standard_fds(DBusEnsureStandardFdsFlags flags, const char **error_str_p);
typedef void (*DBusSignalHandler)(int sig);
void _dbus_set_signal_handler(int sig, DBusSignalHandler handler);
DBUS_END_DECLS

#endif
#ifndef DBUS_SYSDEPS_H
#define DBUS_SYSDEPS_H

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/poll.h>
#include <stdarg.h>
#include "config.h"
#include "dbus-errors.h"
#include "dbus-file.h"
#include "dbus-string.h"
#ifdef DBUS_WINCE
#include "dbus-sysdeps-wince-glue.h"
#endif
#ifdef DBUS_WIN
#include <ws2tcpip.h>
#endif

DBUS_BEGIN_DECLS
#ifdef DBUS_WIN
#define _DBUS_PATH_SEPARATOR ";"
#else
#define _DBUS_PATH_SEPARATOR ":"
#endif
typedef struct DBusList DBusList;
typedef struct DBusCredentials DBusCredentials;
typedef struct DBusPipe DBusPipe;
DBUS_PRIVATE_EXPORT void _dbus_abort(void) _DBUS_GNUC_NORETURN;
dbus_bool_t _dbus_check_setuid(void);
DBUS_PRIVATE_EXPORT const char* _dbus_getenv(const char *varname);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_clearenv(void);
char **_dbus_get_environment (void);
typedef unsigned long dbus_pid_t;
typedef unsigned long dbus_uid_t;
typedef unsigned long dbus_gid_t;
#define DBUS_PID_UNSET  ((dbus_pid_t) -1)
#define DBUS_UID_UNSET  ((dbus_uid_t) -1)
#define DBUS_GID_UNSET  ((dbus_gid_t) -1)
#define DBUS_PID_FORMAT  "%lu"
#define DBUS_UID_FORMAT  "%lu"
#define DBUS_GID_FORMAT  "%lu"
#ifdef DBUS_WIN
typedef struct { SOCKET sock; } DBusSocket;
#define DBUS_SOCKET_FORMAT  "Iu"
#define DBUS_SOCKET_INIT  { INVALID_SOCKET }
static inline SOCKET _dbus_socket_printable(DBusSocket s) { return s.sock; }
static inline dbus_bool_t _dbus_socket_is_valid(DBusSocket s) { return s.sock != INVALID_SOCKET; }
static inline void _dbus_socket_invalidate(DBusSocket *s) { s->sock = INVALID_SOCKET; }
static inline int _dbus_socket_get_int(DBusSocket s) { return (int)s.sock; }
#else
typedef struct { int fd; } DBusSocket;
#define DBUS_SOCKET_FORMAT  "d"
#define DBUS_SOCKET_INIT  { -1 }
static inline int _dbus_socket_printable(DBusSocket s) { return s.fd; }
static inline dbus_bool_t _dbus_socket_is_valid(DBusSocket s) { return s.fd >= 0; }
static inline void _dbus_socket_invalidate(DBusSocket *s) { s->fd = -1; }
static inline int _dbus_socket_get_int(DBusSocket s) { return s.fd; }
#endif
static inline DBusSocket _dbus_socket_get_invalid(void) {
  DBusSocket s = DBUS_SOCKET_INIT;
  return s;
}
dbus_bool_t _dbus_set_socket_nonblocking(DBusSocket fd, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_close_socket(DBusSocket fd, DBusError *error);
DBUS_PRIVATE_EXPORT int _dbus_read_socket(DBusSocket fd, DBusString *buffer, int count);
DBUS_PRIVATE_EXPORT int _dbus_write_socket(DBusSocket fd, const DBusString *buffer, int start, int len);
int _dbus_write_socket_two(DBusSocket fd, const DBusString *buffer1, int start1, int len1, const DBusString *buffer2, int start2, int len2);
int _dbus_read_socket_with_unix_fds(DBusSocket fd, DBusString *buffer, int count, int *fds, unsigned int *n_fds);
DBUS_PRIVATE_EXPORT int _dbus_write_socket_with_unix_fds(DBusSocket fd, const DBusString *buffer, int start, int len, const int *fds, int n_fds);
int _dbus_write_socket_with_unix_fds_two(DBusSocket fd, const DBusString *buffer1, int start1, int len1, const DBusString *buffer2, int start2, int len2,
                                         const int *fds, int n_fds);
DBusSocket _dbus_connect_tcp_socket(const char *host, const char *port, const char *family, DBusError *error);
DBusSocket _dbus_connect_tcp_socket_with_nonce(const char *host, const char *port, const char *family, const char *noncefile, DBusError *error);
int _dbus_listen_tcp_socket(const char *host, const char *port, const char *family, DBusString *retport, DBusSocket **fds_p, DBusError *error);
DBusSocket _dbus_accept(DBusSocket listen_fd);
dbus_bool_t _dbus_read_credentials_socket(DBusSocket client_fd, DBusCredentials *credentials, DBusError *error);
dbus_bool_t _dbus_send_credentials_socket(DBusSocket server_fd, DBusError *error);
typedef enum {
  DBUS_CREDENTIALS_ADD_FLAGS_USER_DATABASE = (1 << 0),
  DBUS_CREDENTIALS_ADD_FLAGS_NONE = 0
} DBusCredentialsAddFlags;
dbus_bool_t _dbus_credentials_add_from_user(DBusCredentials *credentials, const DBusString *username, DBusCredentialsAddFlags flags, DBusError *error);
dbus_bool_t _dbus_credentials_add_from_current_process (DBusCredentials *credentials);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_append_user_from_current_process(DBusString *str);
dbus_bool_t _dbus_parse_unix_user_from_config(const DBusString *username, dbus_uid_t *uid_p);
dbus_bool_t _dbus_parse_unix_group_from_config(const DBusString *groupname, dbus_gid_t *gid_p);
dbus_bool_t _dbus_unix_groups_from_uid(dbus_uid_t uid, dbus_gid_t **group_ids, int *n_group_ids);
dbus_bool_t _dbus_unix_user_is_at_console(dbus_uid_t uid, DBusError *error);
dbus_bool_t _dbus_unix_user_is_process_owner(dbus_uid_t uid);
dbus_bool_t _dbus_windows_user_is_process_owner(const char *windows_sid);
dbus_bool_t _dbus_append_keyring_directory_for_credentials(DBusString *directory, DBusCredentials *credentials);
dbus_bool_t _dbus_daemon_is_session_bus_address_published(const char *scope);
dbus_bool_t _dbus_daemon_publish_session_bus_address(const char* address, const char* shm_name);
void _dbus_daemon_unpublish_session_bus_address(void);
dbus_bool_t _dbus_socket_can_pass_unix_fd(DBusSocket fd);
typedef struct DBusAtomic DBusAtomic;
struct DBusAtomic {
#ifdef DBUS_WIN
  volatile long value;
#else
  volatile dbus_int32_t value;
#endif
};
DBUS_PRIVATE_EXPORT dbus_int32_t _dbus_atomic_inc(DBusAtomic *atomic);
DBUS_PRIVATE_EXPORT dbus_int32_t _dbus_atomic_dec(DBusAtomic *atomic);
DBUS_PRIVATE_EXPORT dbus_int32_t _dbus_atomic_get(DBusAtomic *atomic);
#ifdef DBUS_WIN
typedef DBusSocket DBusPollable;
#define DBUS_POLLABLE_FORMAT  "Iu"
static inline DBusPollable _dbus_socket_get_pollable(DBusSocket s) { return s; }
static inline SOCKET _dbus_pollable_printable(DBusPollable p) { return p.sock; }
static inline dbus_bool_t _dbus_pollable_is_valid(DBusPollable p) { return _dbus_socket_is_valid(p); }
static inline void _dbus_pollable_invalidate(DBusPollable *p) { _dbus_socket_invalidate(p); }
static inline dbus_bool_t _dbus_pollable_equals(DBusPollable a, DBusPollable b) { return a.sock == b.sock; }
#else
typedef int DBusPollable;
#define DBUS_POLLABLE_FORMAT  "d"
static inline DBusPollable _dbus_socket_get_pollable(DBusSocket s) { return s.fd; }
static inline int _dbus_pollable_printable(DBusPollable p) { return p; }
static inline dbus_bool_t _dbus_pollable_is_valid(DBusPollable p) { return p >= 0; }
static inline void _dbus_pollable_invalidate (DBusPollable *p) { *p = -1; }
static inline dbus_bool_t _dbus_pollable_equals (DBusPollable a, DBusPollable b) { return a == b; }
#endif
#if defined(HAVE_POLL) && !defined(BROKEN_POLL)
typedef struct pollfd DBusPollFD;
#define _DBUS_POLLIN  POLLIN
#define _DBUS_POLLPRI  POLLPRI
#define _DBUS_POLLOUT  POLLOUT
#define _DBUS_POLLERR  POLLERR
#define _DBUS_POLLHUP  POLLHUP
#define _DBUS_POLLNVAL  POLLNVAL
#else
typedef struct {
  DBusPollable fd;
  short events;
  short revents;
} DBusPollFD;
#define _DBUS_POLLIN  0x0001
#define _DBUS_POLLPRI  0x0002
#define _DBUS_POLLOUT  0x0004
#define _DBUS_POLLERR  0x0008
#define _DBUS_POLLHUP  0x0010
#define _DBUS_POLLNVAL  0x0020
#endif
DBUS_PRIVATE_EXPORT int _dbus_poll(DBusPollFD *fds, int n_fds, int timeout_milliseconds);
DBUS_PRIVATE_EXPORT void _dbus_sleep_milliseconds(int milliseconds);
DBUS_PRIVATE_EXPORT void _dbus_get_monotonic_time(long *tv_sec, long *tv_usec);
DBUS_PRIVATE_EXPORT void _dbus_get_real_time(long *tv_sec, long *tv_usec);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_create_directory(const DBusString *filename, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_ensure_directory(const DBusString *filename, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_delete_directory(const DBusString *filename, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_concat_dir_and_file (DBusString *dir, const DBusString *next_component);
dbus_bool_t _dbus_string_get_dirname(const DBusString *filename, DBusString *dirname);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_path_is_absolute(const DBusString *filename);
dbus_bool_t _dbus_get_standard_session_servicedirs(DBusList **dirs);
dbus_bool_t _dbus_get_standard_system_servicedirs(DBusList **dirs);
dbus_bool_t _dbus_set_up_transient_session_servicedirs(DBusList **dirs, DBusError *error);
dbus_bool_t _dbus_get_system_config_file(DBusString *str);
dbus_bool_t _dbus_get_session_config_file(DBusString *str);
typedef struct DBusDirIter DBusDirIter;
DBusDirIter* _dbus_directory_open(const DBusString *filename, DBusError *error);
dbus_bool_t  _dbus_directory_get_next_file(DBusDirIter *iter, DBusString *filename, DBusError *error);
void _dbus_directory_close(DBusDirIter *iter);
dbus_bool_t _dbus_check_dir_is_private_to_user(DBusString *dir, DBusError *error);
DBUS_PRIVATE_EXPORT const char* _dbus_get_tmpdir(void);
_DBUS_GNUC_WARN_UNUSED_RESULT dbus_bool_t _dbus_generate_random_bytes_buffer(char *buffer, int n_bytes, DBusError *error);
dbus_bool_t _dbus_generate_random_bytes(DBusString *str, int n_bytes, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_generate_random_ascii(DBusString *str, int n_bytes, DBusError  *error);
DBUS_PRIVATE_EXPORT const char* _dbus_error_from_errno(int error_number);
DBUS_PRIVATE_EXPORT const char* _dbus_error_from_system_errno(void);
int _dbus_save_socket_errno(void);
void _dbus_restore_socket_errno(int saved_errno);
void _dbus_set_errno_to_zero(void);
dbus_bool_t _dbus_get_is_errno_eagain_or_ewouldblock(int e);
dbus_bool_t _dbus_get_is_errno_enomem(int e);
dbus_bool_t _dbus_get_is_errno_eintr(int e);
dbus_bool_t _dbus_get_is_errno_epipe(int e);
dbus_bool_t _dbus_get_is_errno_etoomanyrefs(int e);
DBUS_PRIVATE_EXPORT const char* _dbus_strerror_from_errno(void);
void _dbus_disable_sigpipe(void);
DBUS_PRIVATE_EXPORT void _dbus_exit(int code) _DBUS_GNUC_NORETURN;
DBUS_PRIVATE_EXPORT int _dbus_printf_string_upper_bound(const char *format, va_list args) _DBUS_GNUC_PRINTF(1, 0);
#ifdef DBUS_ENABLE_VERBOSE_MODE
DBUS_PRIVATE_EXPORT void _dbus_print_thread(void);
#endif
typedef struct {
  unsigned long mode;
  unsigned long nlink;
  dbus_uid_t uid;
  dbus_gid_t gid;
  unsigned long size;
  unsigned long atime;
  unsigned long mtime;
  unsigned long ctime;
} DBusStat;
dbus_bool_t _dbus_stat(const DBusString *filename, DBusStat *statbuf, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_socketpair(DBusSocket *fd1, DBusSocket *fd2, dbus_bool_t blocking, DBusError *error);
DBUS_PRIVATE_EXPORT void _dbus_print_backtrace(void);
dbus_bool_t _dbus_become_daemon(const DBusString *pidfile, DBusPipe *print_pid_pipe, DBusError *error, dbus_bool_t keep_umask);
dbus_bool_t _dbus_verify_daemon_user(const char *user);
dbus_bool_t _dbus_change_to_daemon_user(const char *user, DBusError *error);
dbus_bool_t _dbus_write_pid_to_file_and_pipe(const DBusString *pidfile, DBusPipe *print_pid_pipe, dbus_pid_t pid_to_write, DBusError *error);
dbus_bool_t _dbus_command_for_pid(unsigned long  pid, DBusString *str, int max_len, DBusError *error);
dbus_bool_t _dbus_user_at_console(const char *username, DBusError *error);
typedef enum {
  DBUS_LOG_FLAGS_STDERR = (1 << 0),
  DBUS_LOG_FLAGS_SYSTEM_LOG = (1 << 1)
} DBusLogFlags;
DBUS_PRIVATE_EXPORT void _dbus_init_system_log(const char *tag, DBusLogFlags flags);
typedef enum {
  DBUS_SYSTEM_LOG_INFO,
  DBUS_SYSTEM_LOG_WARNING,
  DBUS_SYSTEM_LOG_SECURITY,
  DBUS_SYSTEM_LOG_ERROR
} DBusSystemLogSeverity;
DBUS_PRIVATE_EXPORT void _dbus_log(DBusSystemLogSeverity severity, const char *msg, ...) _DBUS_GNUC_PRINTF(2, 3);
DBUS_PRIVATE_EXPORT void _dbus_logv(DBusSystemLogSeverity severity, const char *msg, va_list args) _DBUS_GNUC_PRINTF(2, 0);
#if !defined (DBUS_VA_COPY)
#if defined(__GNUC__) && defined(__PPC__) && (defined(_CALL_SYSV) || defined(_WIN32))
#define DBUS_VA_COPY(ap1, ap2)   (*(ap1) = *(ap2))
#elif defined(DBUS_VA_COPY_AS_ARRAY)
#define DBUS_VA_COPY(ap1, ap2)  memcpy((ap1), (ap2), sizeof(va_list))
#else
#define DBUS_VA_COPY(ap1, ap2)  ((ap1) = (ap2))
#endif
#endif
#define _DBUS_BYTE_OF_PRIMITIVE(p, i)  (((const char*)&(p))[(i)])
#define _DBUS_DOUBLES_BITWISE_EQUAL(a, b) \
  (_DBUS_BYTE_OF_PRIMITIVE(a, 0) == _DBUS_BYTE_OF_PRIMITIVE(b, 0) && _DBUS_BYTE_OF_PRIMITIVE(a, 1) == _DBUS_BYTE_OF_PRIMITIVE(b, 1) &&  \
   _DBUS_BYTE_OF_PRIMITIVE(a, 2) == _DBUS_BYTE_OF_PRIMITIVE(b, 2) && _DBUS_BYTE_OF_PRIMITIVE(a, 3) == _DBUS_BYTE_OF_PRIMITIVE(b, 3) &&  \
   _DBUS_BYTE_OF_PRIMITIVE(a, 4) == _DBUS_BYTE_OF_PRIMITIVE(b, 4) && _DBUS_BYTE_OF_PRIMITIVE(a, 5) == _DBUS_BYTE_OF_PRIMITIVE(b, 5) &&  \
   _DBUS_BYTE_OF_PRIMITIVE(a, 6) == _DBUS_BYTE_OF_PRIMITIVE(b, 6) && _DBUS_BYTE_OF_PRIMITIVE(a, 7) == _DBUS_BYTE_OF_PRIMITIVE(b, 7))
dbus_bool_t _dbus_get_autolaunch_address(const char *scope, DBusString *address, DBusError *error);
dbus_bool_t _dbus_lookup_session_address(dbus_bool_t *supported, DBusString *address, DBusError *error);
typedef union DBusGUID DBusGUID;
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_read_local_machine_uuid(DBusGUID *machine_id, dbus_bool_t create_if_not_found, DBusError *error);
dbus_bool_t _dbus_threads_init_platform_specific(void);
void _dbus_threads_lock_platform_specific(void);
void _dbus_threads_unlock_platform_specific(void);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_split_paths_and_append(DBusString *dirs, const char *suffix, DBusList **dir_list);
unsigned long _dbus_pid_for_log(void);
DBUS_PRIVATE_EXPORT dbus_pid_t _dbus_getpid(void);
DBUS_PRIVATE_EXPORT dbus_uid_t _dbus_getuid(void);
DBUS_PRIVATE_EXPORT void _dbus_flush_caches(void);
dbus_bool_t _dbus_replace_install_prefix(DBusString *path);
#define DBUS_DEFAULT_MESSAGE_UNIX_FDS 16
typedef struct DBusRLimit DBusRLimit;
DBusRLimit *_dbus_rlimit_save_fd_limit(DBusError *error);
dbus_bool_t _dbus_rlimit_raise_fd_limit_if_privileged(unsigned int desired, DBusError *error);
dbus_bool_t _dbus_rlimit_restore_fd_limit(DBusRLimit *saved, DBusError *error);
void _dbus_rlimit_free(DBusRLimit *lim);
DBUS_END_DECLS
#ifdef DBUS_WIN
#include "dbus-sysdeps-win.h"
#endif

#endif
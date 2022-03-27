#include "dbus-userdb.h"
#include "dbus-test.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <grp.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/un.h>
#include "config.h"
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-unix.h"
#include "dbus-internals.h"
#include "dbus-list.h"
#include "dbus-pipe.h"
#include "dbus-protocol.h"
#include "dbus-string.h"

#define DBUS_USERDB_INCLUDES_PRIVATE 1
#ifndef O_BINARY
#define O_BINARY 0
#endif
dbus_bool_t _dbus_become_daemon(const DBusString *pidfile, DBusPipe *print_pid_pipe, DBusError *error, dbus_bool_t keep_umask) {
  const char *s;
  pid_t child_pid;
  DBusEnsureStandardFdsFlags flags;
  _dbus_verbose("Becoming a daemon...\n");
  _dbus_verbose("chdir to /\n");
  if (chdir("/") < 0) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Could not chdir() to root directory");
      return FALSE;
  }
  _dbus_verbose("forking...\n");
  fflush(stdout);
  fflush(stderr);
  switch((child_pid = fork())) {
      case -1:
          _dbus_verbose("fork failed\n");
          dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to fork daemon: %s", _dbus_strerror(errno));
          return FALSE;
          break;
      case 0:
          _dbus_verbose("in child, closing std file descriptors\n");
          flags = DBUS_FORCE_STDIN_NULL | DBUS_FORCE_STDOUT_NULL;
          s = _dbus_getenv("DBUS_DEBUG_OUTPUT");
          if (s == NULL || *s == '\0') flags |= DBUS_FORCE_STDERR_NULL;
          else _dbus_verbose("keeping stderr open due to DBUS_DEBUG_OUTPUT\n");
          if (!_dbus_ensure_standard_fds(flags, &s)) {
              _dbus_warn("%s: %s", s, _dbus_strerror(errno));
              _exit(1);
          }
          if (!keep_umask) {
              _dbus_verbose("setting umask\n");
              umask(022);
          }
          _dbus_verbose("calling setsid()\n");
          if (setsid() == -1) _dbus_assert_not_reached("setsid() failed");
          break;
      default:
          if (!_dbus_write_pid_to_file_and_pipe(pidfile, print_pid_pipe, child_pid, error)) {
              _dbus_verbose("pid file or pipe write failed: %s\n", error->message);
              kill(child_pid, SIGTERM);
              return FALSE;
          }
          _dbus_verbose("parent exiting\n");
          _exit(0);
  }
  return TRUE;
}
static dbus_bool_t _dbus_write_pid_file(const DBusString *filename, unsigned long pid, DBusError *error) {
  const char *cfilename;
  int fd;
  FILE *f;
  cfilename = _dbus_string_get_const_data(filename);
  fd = open (cfilename, O_WRONLY | O_CREAT | O_EXCL | O_BINARY, 0644);
  if (fd < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to open \"%s\": %s", cfilename, _dbus_strerror(errno));
      return FALSE;
  }
  if ((f = fdopen(fd, "w")) == NULL) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to fdopen fd %d: %s", fd, _dbus_strerror(errno));
      _dbus_close(fd, NULL);
      return FALSE;
  }
  if (fprintf(f, "%lu\n", pid) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to write to \"%s\": %s", cfilename, _dbus_strerror(errno));
      fclose(f);
      return FALSE;
  }
  if (fclose(f) == EOF) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to close \"%s\": %s", cfilename, _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_write_pid_to_file_and_pipe(const DBusString *pidfile, DBusPipe *print_pid_pipe, dbus_pid_t pid_to_write, DBusError *error) {
  if (pidfile) {
      _dbus_verbose("writing pid file %s\n", _dbus_string_get_const_data(pidfile));
      if (!_dbus_write_pid_file(pidfile, pid_to_write, error)) {
          _dbus_verbose("pid file write failed\n");
          _DBUS_ASSERT_ERROR_IS_SET(error);
          return FALSE;
      }
  } else { _dbus_verbose("No pid file requested\n"); }
  if (print_pid_pipe != NULL && _dbus_pipe_is_valid(print_pid_pipe)) {
      DBusString pid;
      int bytes;
      _dbus_verbose("writing our pid to pipe %d\n", print_pid_pipe->fd);
      if (!_dbus_string_init(&pid)) {
          _DBUS_SET_OOM(error);
          return FALSE;
      }
      if (!_dbus_string_append_int(&pid, pid_to_write) || !_dbus_string_append(&pid, "\n")) {
          _dbus_string_free(&pid);
          _DBUS_SET_OOM(error);
          return FALSE;
      }
      bytes = _dbus_string_get_length(&pid);
      if (_dbus_pipe_write(print_pid_pipe, &pid, 0, bytes, error) != bytes) {
          if (error != NULL && !dbus_error_is_set(error)) dbus_set_error(error, DBUS_ERROR_FAILED,"Printing message bus PID: did not write enough bytes\n");
          _dbus_string_free(&pid);
          return FALSE;
      }
      _dbus_string_free(&pid);
  } else { _dbus_verbose("No pid pipe to write to\n"); }
  return TRUE;
}
dbus_bool_t _dbus_verify_daemon_user(const char *user) {
  DBusString u;
  _dbus_string_init_const(&u, user);
  return _dbus_get_user_id_and_primary_group(&u, NULL, NULL);
}
#ifndef HAVE_LIBAUDIT
dbus_bool_t _dbus_change_to_daemon_user(const char *user, DBusError *error) {
  dbus_uid_t uid;
  dbus_gid_t gid;
  DBusString u;
  _dbus_string_init_const(&u, user);
  if (!_dbus_get_user_id_and_primary_group(&u, &uid, &gid)) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"User '%s' does not appear to exist?", user);
      return FALSE;
  }
  if (setgroups(0, NULL) < 0) _dbus_warn("Failed to drop supplementary groups: %s", _dbus_strerror(errno));
  if (setgid (gid) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to set GID to %lu: %s", gid, _dbus_strerror(errno));
      return FALSE;
  }
  if (setuid(uid) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to set UID to %lu: %s", uid, _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
#endif
#ifdef HAVE_SETRLIMIT
struct DBusRLimit {
    struct rlimit lim;
};
DBusRLimit *_dbus_rlimit_save_fd_limit(DBusError *error) {
  DBusRLimit *self;
  self = dbus_new0(DBusRLimit, 1);
  if (self == NULL) {
      _DBUS_SET_OOM(error);
      return NULL;
  }
  if (getrlimit(RLIMIT_NOFILE, &self->lim) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno), "Failed to get fd limit: %s", _dbus_strerror(errno));
      dbus_free(self);
      return NULL;
  }
  return self;
}
dbus_bool_t _dbus_rlimit_raise_fd_limit_if_privileged(unsigned int desired, DBusError *error) {
  struct rlimit lim;
  if (getuid() != 0) return TRUE;
  if (getrlimit(RLIMIT_NOFILE, &lim) < 0) {
      dbus_set_error(error, _dbus_error_from_errno (errno), "Failed to get fd limit: %s", _dbus_strerror(errno));
      return FALSE;
  }
  if (lim.rlim_cur == RLIM_INFINITY || lim.rlim_cur >= desired) return TRUE;
  lim.rlim_cur = lim.rlim_max = desired;
  if (setrlimit(RLIMIT_NOFILE, &lim) < 0) {
      dbus_set_error(error, _dbus_error_from_errno (errno), "Failed to set fd limit to %u: %s", desired, _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_rlimit_restore_fd_limit(DBusRLimit *saved, DBusError *error) {
  if (setrlimit(RLIMIT_NOFILE, &saved->lim) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno), "Failed to restore old fd limit: %s", _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
#else
static void fd_limit_not_supported(DBusError *error) {
  dbus_set_error(error, DBUS_ERROR_NOT_SUPPORTED,"cannot change fd limit on this platform");
}
DBusRLimit *_dbus_rlimit_save_fd_limit(DBusError *error) {
  fd_limit_not_supported(error);
  return NULL;
}
dbus_bool_t _dbus_rlimit_raise_fd_limit_if_privileged(unsigned int desired, DBusError *error) {
  fd_limit_not_supported(error);
  return FALSE;
}
dbus_bool_t _dbus_rlimit_restore_fd_limit(DBusRLimit *saved, DBusError *error) {
  fd_limit_not_supported(error);
  return FALSE;
}
#endif
void _dbus_rlimit_free(DBusRLimit *lim) {
  dbus_free (lim);
}
void _dbus_set_signal_handler(int sig, DBusSignalHandler handler) {
  struct sigaction act;
  sigset_t empty_mask;
  sigemptyset(&empty_mask);
  act.sa_handler = handler;
  act.sa_mask = empty_mask;
  act.sa_flags = 0;
  sigaction(sig,  &act, NULL);
}
dbus_bool_t _dbus_file_exists(const char *file) {
  return (access (file, F_OK) == 0);
}
dbus_bool_t _dbus_user_at_console(const char *username, DBusError *error) {
#ifdef DBUS_CONSOLE_AUTH_DIR
  DBusString u, f;
  dbus_bool_t result;
  result = FALSE;
  if (!_dbus_string_init(&f)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_string_append(&f, DBUS_CONSOLE_AUTH_DIR)) {
      _DBUS_SET_OOM(error);
      goto out;
  }
  _dbus_string_init_const(&u, username);
  if (!_dbus_concat_dir_and_file(&f, &u)) {
      _DBUS_SET_OOM(error);
      goto out;
  }
  result = _dbus_file_exists(_dbus_string_get_const_data(&f));
out:
  _dbus_string_free(&f);
  return result;
#else
  return FALSE;
#endif
}
dbus_bool_t _dbus_path_is_absolute(const DBusString *filename) {
  if (_dbus_string_get_length(filename) > 0) return _dbus_string_get_byte(filename, 0) == '/';
  else return FALSE;
}
dbus_bool_t _dbus_stat(const DBusString *filename, DBusStat *statbuf, DBusError *error) {
  const char *filename_c;
  struct stat sb;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  if (stat(filename_c, &sb) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"%s", _dbus_strerror(errno));
      return FALSE;
  }
  statbuf->mode = sb.st_mode;
  statbuf->nlink = sb.st_nlink;
  statbuf->uid = sb.st_uid;
  statbuf->gid = sb.st_gid;
  statbuf->size = sb.st_size;
  statbuf->atime = sb.st_atime;
  statbuf->mtime = sb.st_mtime;
  statbuf->ctime = sb.st_ctime;
  return TRUE;
}
struct DBusDirIter {
  DIR *d;
};
DBusDirIter* _dbus_directory_open(const DBusString *filename, DBusError *error) {
  DIR *d;
  DBusDirIter *iter;
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  d = opendir(filename_c);
  if (d == NULL) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to read directory \"%s\": %s", filename_c, _dbus_strerror(errno));
      return NULL;
  }
  iter = dbus_new0(DBusDirIter, 1);
  if (iter == NULL) {
      closedir(d);
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY,"Could not allocate memory for directory iterator");
      return NULL;
  }
  iter->d = d;
  return iter;
}
dbus_bool_t _dbus_directory_get_next_file(DBusDirIter *iter, DBusString *filename, DBusError *error) {
  struct dirent *ent;
  int err;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
 again:  errno = 0;
  ent = readdir(iter->d);
  if (!ent) {
      err = errno;
      if (err != 0) dbus_set_error(error, _dbus_error_from_errno(err),"%s", _dbus_strerror(err));
      return FALSE;
  } else if (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0'))) goto again;
  else {
      _dbus_string_set_length(filename, 0);
      if (!_dbus_string_append(filename, ent->d_name)) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY,"No memory to read directory entry");
          return FALSE;
      } else return TRUE;
  }
}
void _dbus_directory_close(DBusDirIter *iter) {
  closedir(iter->d);
  dbus_free(iter);
}
static dbus_bool_t fill_user_info_from_group(struct group *g, DBusGroupInfo *info, DBusError *error) {
  _dbus_assert(g->gr_name != NULL);
  info->gid = g->gr_gid;
  info->groupname = _dbus_strdup(g->gr_name);
  if (info->groupname == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return FALSE;
  }
  return TRUE;
}
static dbus_bool_t fill_group_info(DBusGroupInfo *info, dbus_gid_t gid, const DBusString *groupname, DBusError *error) {
  const char *group_c_str;
  _dbus_assert(groupname != NULL || gid != DBUS_GID_UNSET);
  _dbus_assert(groupname == NULL || gid == DBUS_GID_UNSET);
  if (groupname) group_c_str = _dbus_string_get_const_data(groupname);
  else group_c_str = NULL;
#if defined (HAVE_POSIX_GETPWNAM_R) || defined (HAVE_NONPOSIX_GETPWNAM_R)
  {
      struct group *g;
      int result;
      size_t buflen;
      char *buf;
      struct group g_str;
      dbus_bool_t b;
      buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
      if ((long)buflen <= 0) buflen = 1024;
      result = -1;
      while(1) {
          buf = dbus_malloc(buflen);
          if (buf == NULL) {
              dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
              return FALSE;
          }
          g = NULL;
      #ifdef HAVE_POSIX_GETPWNAM_R
          if (group_c_str) result = getgrnam_r(group_c_str, &g_str, buf, buflen, &g);
          else result = getgrgid_r(gid, &g_str, buf, buflen, &g);
      #else
          g = getgrnam_r(group_c_str, &g_str, buf, buflen);
          result = 0;
      #endif
          if (result == ERANGE && buflen < 512 * 1024) {
              dbus_free(buf);
              buflen *= 2;
          } else break;
      }
      if (result == 0 && g == &g_str) {
          b = fill_user_info_from_group(g, info, error);
          dbus_free(buf);
          return b;
      } else {
          dbus_set_error(error, _dbus_error_from_errno(errno),"Group %s unknown or failed to look it up\n", group_c_str ? group_c_str : "???");
          dbus_free(buf);
          return FALSE;
      }
  }
#else
  {
      struct group *g;
      g = getgrnam(group_c_str);
      if (g != NULL) return fill_user_info_from_group(g, info, error);
      else {
          dbus_set_error(error, _dbus_error_from_errno(errno), "Group %s unknown or failed to look it up\n", group_c_str ? group_c_str : "???");
          return FALSE;
      }
  }
#endif
}
dbus_bool_t _dbus_group_info_fill(DBusGroupInfo *info, const DBusString *groupname, DBusError *error) {
  return fill_group_info(info, DBUS_GID_UNSET, groupname, error);

}
dbus_bool_t _dbus_group_info_fill_gid(DBusGroupInfo *info, dbus_gid_t gid, DBusError *error) {
  return fill_group_info(info, gid, NULL, error);
}
dbus_bool_t _dbus_parse_unix_user_from_config(const DBusString *username, dbus_uid_t *uid_p) {
  return _dbus_get_user_id(username, uid_p);

}
dbus_bool_t _dbus_parse_unix_group_from_config(const DBusString *groupname, dbus_gid_t *gid_p) {
  return _dbus_get_group_id(groupname, gid_p);
}
dbus_bool_t _dbus_unix_groups_from_uid(dbus_uid_t uid, dbus_gid_t **group_ids, int *n_group_ids) {
  return _dbus_groups_from_uid(uid, group_ids, n_group_ids);
}
dbus_bool_t _dbus_unix_user_is_at_console(dbus_uid_t uid, DBusError *error) {
  return _dbus_is_console_user(uid, error);
}
dbus_bool_t _dbus_unix_user_is_process_owner(dbus_uid_t uid) {
  return uid == _dbus_geteuid();
}
dbus_bool_t _dbus_windows_user_is_process_owner(const char *windows_sid) {
  return FALSE;
}
dbus_bool_t _dbus_string_get_dirname(const DBusString *filename, DBusString *dirname) {
  int sep;
  _dbus_assert(filename != dirname);
  _dbus_assert(filename != NULL);
  _dbus_assert(dirname != NULL);
  sep = _dbus_string_get_length(filename);
  if (sep == 0) return _dbus_string_append(dirname, ".");
  while(sep > 0 && _dbus_string_get_byte(filename, sep - 1) == '/') --sep;
  _dbus_assert(sep >= 0);
  if (sep == 0) return _dbus_string_append(dirname, "/");
  _dbus_string_find_byte_backward(filename, sep, '/', &sep);
  if (sep < 0) return _dbus_string_append(dirname, ".");
  while(sep > 0 && _dbus_string_get_byte(filename, sep - 1) == '/') --sep;
  _dbus_assert(sep >= 0);
  if (sep == 0 && _dbus_string_get_byte(filename, 0) == '/') return _dbus_string_append(dirname, "/");
  else return _dbus_string_copy_len(filename, 0, sep - 0, dirname, _dbus_string_get_length(dirname));
}
static void string_squash_nonprintable(DBusString *str) {
  unsigned char *buf;
  int i, len;
  buf = _dbus_string_get_udata(str);
  len = _dbus_string_get_length(str);
  for (i = 0; i < len; i++) {
      unsigned char c = (unsigned char)buf[i];
      if (c == '\0') buf[i] = ' ';
      else if (c < 0x20 || c > 127) buf[i] = '?';
  }
}
dbus_bool_t _dbus_command_for_pid(unsigned long pid, DBusString *str, int max_len, DBusError *error) {
  DBusString path;
  DBusString cmdline;
  int fd;
  if (!_dbus_string_init(&path)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_string_init(&cmdline)) {
      _DBUS_SET_OOM(error);
      _dbus_string_free(&path);
      return FALSE;
  }
  if (!_dbus_string_append_printf(&path, "/proc/%ld/cmdline", pid)) goto oom;
  fd = open(_dbus_string_get_const_data(&path), O_RDONLY);
  if (fd < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to open \"%s\": %s", _dbus_string_get_const_data(&path), _dbus_strerror(errno));
      goto fail;
  }
  if (!_dbus_read(fd, &cmdline, max_len)) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to read from \"%s\": %s", _dbus_string_get_const_data(&path), _dbus_strerror(errno));
      _dbus_close(fd, NULL);
      goto fail;
  }
  if (!_dbus_close(fd, error)) goto fail;
  string_squash_nonprintable(&cmdline);
  if (!_dbus_string_copy(&cmdline, 0, str, _dbus_string_get_length(str))) goto oom;
  _dbus_string_free(&cmdline);
  _dbus_string_free(&path);
  return TRUE;
oom:
  _DBUS_SET_OOM(error);
fail:
  _dbus_string_free(&cmdline);
  _dbus_string_free(&path);
  return FALSE;
}
dbus_bool_t _dbus_replace_install_prefix(DBusString *path) {
  return TRUE;
}
static dbus_bool_t ensure_owned_directory(const char *label, const DBusString *string, dbus_bool_t create, DBusError *error) {
  const char *dir = _dbus_string_get_const_data(string);
  struct stat buf;
  if (create && !_dbus_ensure_directory(string, error)) return FALSE;
  if (stat(dir, &buf) != 0) {
      int saved_errno = errno;
      dbus_set_error(error, _dbus_error_from_errno(saved_errno),"%s \"%s\" not available: %s", label, dir, _dbus_strerror(saved_errno));
      return FALSE;
  }
  if (!S_ISDIR(buf.st_mode)) {
      dbus_set_error(error, DBUS_ERROR_FAILED, "%s \"%s\" is not a directory", label, dir);
      return FALSE;
  }
  if (buf.st_uid != geteuid()) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"%s \"%s\" is owned by uid %ld, not our uid %ld", label, dir, (long)buf.st_uid, (long)geteuid());
      return FALSE;
  }
  if ((S_IWOTH | S_IWGRP) & buf.st_mode) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"%s \"%s\" can be written by others (mode 0%o)", label, dir, buf.st_mode);
      return FALSE;
  }
  return TRUE;
}
#define DBUS_UNIX_STANDARD_SESSION_SERVICEDIR "/dbus-1/services"
#define DBUS_UNIX_STANDARD_SYSTEM_SERVICEDIR "/dbus-1/system-services"
dbus_bool_t _dbus_set_up_transient_session_servicedirs(DBusList **dirs, DBusError *error) {
  const char *xdg_runtime_dir;
  DBusString services;
  DBusString dbus1;
  DBusString xrd;
  dbus_bool_t ret = FALSE;
  char *data = NULL;
  if (!_dbus_string_init(&dbus1)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_string_init(&services)) {
      _dbus_string_free(&dbus1);
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_string_init(&xrd)) {
      _dbus_string_free(&dbus1);
      _dbus_string_free(&services);
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  xdg_runtime_dir = _dbus_getenv("XDG_RUNTIME_DIR");
  if (xdg_runtime_dir == NULL) {
      _dbus_verbose("XDG_RUNTIME_DIR is unset: transient session services not available here\n");
      ret = TRUE;
      goto out;
  }
  if (!_dbus_string_append(&xrd, xdg_runtime_dir) || !_dbus_string_append_printf(&dbus1, "%s/dbus-1", xdg_runtime_dir) ||
      !_dbus_string_append_printf(&services, "%s/dbus-1/services", xdg_runtime_dir)) {
      _DBUS_SET_OOM(error);
      goto out;
  }
  if (!ensure_owned_directory("XDG_RUNTIME_DIR", &xrd, FALSE, error) || !ensure_owned_directory("XDG_RUNTIME_DIR subdirectory", &dbus1, TRUE, error) ||
      !ensure_owned_directory("XDG_RUNTIME_DIR subdirectory", &services, TRUE, error))
      goto out;
  if (!_dbus_string_steal_data(&services, &data) || !_dbus_list_append(dirs, data)) {
      _DBUS_SET_OOM(error);
      goto out;
  }
  _dbus_verbose("Transient service directory is %s\n", data);
  data = NULL;
  ret = TRUE;
out:
  _dbus_string_free(&dbus1);
  _dbus_string_free(&services);
  _dbus_string_free(&xrd);
  dbus_free(data);
  return ret;
}
dbus_bool_t _dbus_get_standard_session_servicedirs(DBusList **dirs) {
  const char *xdg_data_home;
  const char *xdg_data_dirs;
  DBusString servicedir_path;
  if (!_dbus_string_init(&servicedir_path)) return FALSE;
  xdg_data_home = _dbus_getenv("XDG_DATA_HOME");
  xdg_data_dirs = _dbus_getenv("XDG_DATA_DIRS");
  if (xdg_data_home != NULL) {
      if (!_dbus_string_append(&servicedir_path, xdg_data_home)) goto oom;
  } else {
      const DBusString *homedir;
      DBusString local_share;
      if (!_dbus_homedir_from_current_process(&homedir)) goto oom;
      if (!_dbus_string_append(&servicedir_path, _dbus_string_get_const_data(homedir))) goto oom;
      _dbus_string_init_const(&local_share, "/.local/share");
      if (!_dbus_concat_dir_and_file(&servicedir_path, &local_share)) goto oom;
  }
  if (!_dbus_string_append(&servicedir_path, ":")) goto oom;
  if (xdg_data_dirs != NULL) {
      if (!_dbus_string_append(&servicedir_path, xdg_data_dirs)) goto oom;
      if (!_dbus_string_append(&servicedir_path, ":")) goto oom;
  } else {
      if (!_dbus_string_append(&servicedir_path, "/usr/local/share:/usr/share:")) goto oom;
  }
  if (!_dbus_string_append (&servicedir_path, DBUS_DATADIR)) goto oom;
  if (!_dbus_split_paths_and_append (&servicedir_path, DBUS_UNIX_STANDARD_SESSION_SERVICEDIR, dirs)) goto oom;
  _dbus_string_free (&servicedir_path);
  return TRUE;
oom:
  _dbus_string_free (&servicedir_path);
  return FALSE;
}
dbus_bool_t _dbus_get_standard_system_servicedirs(DBusList **dirs) {
  static const char standard_search_path[] = "/usr/local/share:/usr/share:" DBUS_DATADIR ":/lib";
  DBusString servicedir_path;
  _dbus_string_init_const(&servicedir_path, standard_search_path);
  return _dbus_split_paths_and_append(&servicedir_path, DBUS_UNIX_STANDARD_SYSTEM_SERVICEDIR, dirs);
}
dbus_bool_t _dbus_get_system_config_file(DBusString *str) {
  _dbus_assert(_dbus_string_get_length(str) == 0);
  return _dbus_string_append(str, DBUS_SYSTEM_CONFIG_FILE);
}
dbus_bool_t _dbus_get_session_config_file(DBusString *str) {
  _dbus_assert(_dbus_string_get_length(str) == 0);
  return _dbus_string_append(str, DBUS_SESSION_CONFIG_FILE);
}
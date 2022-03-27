#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/un.h>
#include <pwd.h>
#include <time.h>
#include <locale.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <grp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <syslog.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <alloca.h>
#include <pthread.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-unix.h"
#include "dbus-threads.h"
#include "dbus-protocol.h"
#include "dbus-file.h"
#include "dbus-transport.h"
#include "dbus-string.h"
#include "dbus-userdb.h"
#include "dbus-list.h"
#include "dbus-credentials.h"
#include "dbus-nonce.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif
#ifndef HAVE_SOCKLEN_T
#define socklen_t int
#endif
#if defined (__sun) || defined (__sun__)
#ifndef CMSG_ALIGN
#ifdef __sun__
#define CMSG_ALIGN(len) _CMSG_DATA_ALIGN(len)
#else
#define CMSG_ALIGN(len)  (((len) + sizeof(long) - 1) & ~(sizeof(long) - 1))
#endif
#endif
#ifndef CMSG_SPACE
#define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#endif
#ifndef CMSG_LEN
#define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#endif
#endif
dbus_bool_t _dbus_ensure_standard_fds(DBusEnsureStandardFdsFlags flags, const char **error_str_p) {
  static int const relevant_flag[] = { DBUS_FORCE_STDIN_NULL, DBUS_FORCE_STDOUT_NULL, DBUS_FORCE_STDERR_NULL };
  const char *error_str = "Failed mysteriously";
  int devnull = -1;
  int saved_errno;
  _DBUS_STATIC_ASSERT(STDIN_FILENO == 0);
  _DBUS_STATIC_ASSERT(STDOUT_FILENO == 1);
  _DBUS_STATIC_ASSERT(STDERR_FILENO == 2);
  int i;
  for (i = STDIN_FILENO; i <= STDERR_FILENO; i++) {
      if (devnull < i) devnull = open("/dev/null", O_RDWR);
      if (devnull < 0) {
          error_str = "Failed to open /dev/null";
          goto out;
      }
      _dbus_assert(devnull >= i);
      if (devnull != i && (flags & relevant_flag[i]) != 0) {
          if (dup2(devnull, i) < 0) {
              error_str = "Failed to dup2 /dev/null onto a standard fd";
              goto out;
          }
      }
  }
  error_str = NULL;
out:
  saved_errno = errno;
  if (devnull > STDERR_FILENO) close(devnull);
  if (error_str_p != NULL) *error_str_p = error_str;
  errno = saved_errno;
  return (error_str == NULL);
}
static dbus_bool_t _dbus_set_fd_nonblocking(int fd, DBusError *error);
static dbus_bool_t _dbus_open_socket(int *fd_p, int domain, int type, int protocol, DBusError *error) {
#ifdef SOCK_CLOEXEC
  dbus_bool_t cloexec_done;
  *fd_p = socket(domain, type | SOCK_CLOEXEC, protocol);
  cloexec_done = *fd_p >= 0;
  if (*fd_p < 0 && (errno == EINVAL || errno == EPROTOTYPE))
#endif
  *fd_p = socket(domain, type, protocol);
  if (*fd_p >= 0) {
#ifdef SOCK_CLOEXEC
      if (!cloexec_done)
#endif
       _dbus_fd_set_close_on_exec(*fd_p);
      _dbus_verbose("socket fd %d opened\n", *fd_p);
      return TRUE;
  } else {
      dbus_set_error(error, _dbus_error_from_errno(errno), "Failed to open socket: %s", _dbus_strerror(errno));
      return FALSE;
  }
}
static dbus_bool_t _dbus_open_unix_socket(int *fd, DBusError *error) {
  return _dbus_open_socket(fd, PF_UNIX, SOCK_STREAM, 0, error);
}
dbus_bool_t _dbus_close_socket(DBusSocket fd, DBusError *error) {
  return _dbus_close(fd.fd, error);
}
int _dbus_read_socket(DBusSocket fd, DBusString *buffer, int count) {
  return _dbus_read(fd.fd, buffer, count);
}
int _dbus_write_socket(DBusSocket fd, const DBusString *buffer, int start, int len) {
#if HAVE_DECL_MSG_NOSIGNAL
  const char *data;
  int bytes_written;
  data = _dbus_string_get_const_data_len(buffer, start, len);
again:
  bytes_written = send(fd.fd, data, len, MSG_NOSIGNAL);
  if (bytes_written < 0 && errno == EINTR) goto again;
  return bytes_written;
#else
  return _dbus_write(fd.fd, buffer, start, len);
#endif
}
int _dbus_read_socket_with_unix_fds(DBusSocket fd, DBusString *buffer, int count, int *fds, unsigned int *n_fds) {
#ifndef HAVE_UNIX_FD_PASSING
  int r;
  if ((r = _dbus_read_socket(fd, buffer, count)) < 0) return r;
  *n_fds = 0;
  return r;
#else
  int bytes_read;
  int start;
  struct msghdr m;
  struct iovec iov;
  _dbus_assert(count >= 0);
  _dbus_assert(*n_fds <= DBUS_MAXIMUM_MESSAGE_UNIX_FDS);
  start = _dbus_string_get_length(buffer);
  if (!_dbus_string_lengthen(buffer, count)) {
      errno = ENOMEM;
      return -1;
  }
  _DBUS_ZERO(iov);
  iov.iov_base = _dbus_string_get_data_len(buffer, start, count);
  iov.iov_len = count;
  _DBUS_ZERO(m);
  m.msg_iov = &iov;
  m.msg_iovlen = 1;
  m.msg_controllen = CMSG_SPACE(*n_fds * sizeof(int));
  m.msg_control = alloca(m.msg_controllen);
  memset(m.msg_control, 0, m.msg_controllen);
  m.msg_controllen = CMSG_LEN(*n_fds * sizeof(int));
again:
  bytes_read = recvmsg(fd.fd, &m, 0
                    #ifdef MSG_CMSG_CLOEXEC
                       | MSG_CMSG_CLOEXEC
                    #endif
                       );
  if (bytes_read < 0) {
      if (errno == EINTR) goto again;
      else {
          _dbus_string_set_length(buffer, start);
          return -1;
      }
  } else {
      struct cmsghdr *cm;
      dbus_bool_t found = FALSE;
      if (m.msg_flags & MSG_CTRUNC) {
          errno = ENOSPC;
          _dbus_string_set_length(buffer, start);
          return -1;
      }
      for (cm = CMSG_FIRSTHDR(&m); cm; cm = CMSG_NXTHDR(&m, cm))
          if (cm->cmsg_level == SOL_SOCKET && cm->cmsg_type == SCM_RIGHTS) {
              size_t i;
              int *payload = (int*)CMSG_DATA(cm);
              size_t payload_len_bytes = (cm->cmsg_len - CMSG_LEN(0));
              size_t payload_len_fds = payload_len_bytes / sizeof(int);
              size_t fds_to_use;
              _DBUS_STATIC_ASSERT(sizeof(size_t) >= sizeof(unsigned int));
              if (_DBUS_LIKELY(payload_len_fds <= (size_t)*n_fds)) fds_to_use = payload_len_fds;
              else {
                  fds_to_use = (size_t)*n_fds;
                  for (i = fds_to_use; i < payload_len_fds; i++) close(payload[i]);
              }
              memcpy(fds, payload, fds_to_use * sizeof(int));
              found = TRUE;
              *n_fds = (unsigned int)fds_to_use;
              for (i = 0; i < fds_to_use; i++) _dbus_fd_set_close_on_exec(fds[i]);
              break;
          }
      if (!found) *n_fds = 0;
      _dbus_string_set_length(buffer, start + bytes_read);
  #if 0
      if (bytes_read > 0) _dbus_verbose_bytes_of_string(buffer, start, bytes_read);
  #endif
      return bytes_read;
  }
#endif
}
int _dbus_write_socket_with_unix_fds(DBusSocket fd, const DBusString *buffer, int start, int len, const int *fds, int n_fds) {
#ifndef HAVE_UNIX_FD_PASSING
  if (n_fds > 0) {
      errno = ENOTSUP;
      return -1;
  }
  return _dbus_write_socket(fd, buffer, start, len);
#else
  return _dbus_write_socket_with_unix_fds_two(fd, buffer, start, len, NULL, 0, 0, fds, n_fds);
#endif
}
int _dbus_write_socket_with_unix_fds_two(DBusSocket fd, const DBusString *buffer1, int start1, int len1, const DBusString *buffer2, int start2, int len2,
                                         const int *fds, int n_fds) {
#ifndef HAVE_UNIX_FD_PASSING
  if (n_fds > 0) {
      errno = ENOTSUP;
      return -1;
  }
  return _dbus_write_socket_two(fd, buffer1, start1, len1, buffer2, start2, len2);
#else
  struct msghdr m;
  struct cmsghdr *cm;
  struct iovec iov[2];
  int bytes_written;
  _dbus_assert(len1 >= 0);
  _dbus_assert(len2 >= 0);
  _dbus_assert(n_fds >= 0);
  _DBUS_ZERO(iov);
  iov[0].iov_base = (char*)_dbus_string_get_const_data_len(buffer1, start1, len1);
  iov[0].iov_len = len1;
  if (buffer2) {
      iov[1].iov_base = (char*)_dbus_string_get_const_data_len(buffer2, start2, len2);
      iov[1].iov_len = len2;
  }
  _DBUS_ZERO(m);
  m.msg_iov = iov;
  m.msg_iovlen = buffer2 ? 2 : 1;
  if (n_fds > 0) {
      m.msg_controllen = CMSG_SPACE(n_fds * sizeof(int));
      m.msg_control = alloca(m.msg_controllen);
      memset(m.msg_control, 0, m.msg_controllen);
      cm = CMSG_FIRSTHDR(&m);
      cm->cmsg_level = SOL_SOCKET;
      cm->cmsg_type = SCM_RIGHTS;
      cm->cmsg_len = CMSG_LEN(n_fds * sizeof(int));
      memcpy(CMSG_DATA(cm), fds, n_fds * sizeof(int));
  }
again:
  bytes_written = sendmsg(fd.fd, &m, 0
                     #if HAVE_DECL_MSG_NOSIGNAL
                          | MSG_NOSIGNAL
                     #endif
                         );
  if (bytes_written < 0 && errno == EINTR) goto again;
#if 0
  if (bytes_written > 0) _dbus_verbose_bytes_of_string(buffer, start, bytes_written);
#endif
  return bytes_written;
#endif
}
int _dbus_write_socket_two(DBusSocket fd, const DBusString *buffer1, int start1, int len1, const DBusString *buffer2, int start2, int len2) {
#if HAVE_DECL_MSG_NOSIGNAL
  struct iovec vectors[2];
  const char *data1;
  const char *data2;
  int bytes_written;
  struct msghdr m;
  _dbus_assert(buffer1 != NULL);
  _dbus_assert(start1 >= 0);
  _dbus_assert(start2 >= 0);
  _dbus_assert(len1 >= 0);
  _dbus_assert(len2 >= 0);
  data1 = _dbus_string_get_const_data_len(buffer1, start1, len1);
  if (buffer2 != NULL) data2 = _dbus_string_get_const_data_len(buffer2, start2, len2);
  else {
      data2 = NULL;
      start2 = 0;
      len2 = 0;
  }
  vectors[0].iov_base = (char*)data1;
  vectors[0].iov_len = len1;
  vectors[1].iov_base = (char*)data2;
  vectors[1].iov_len = len2;
  _DBUS_ZERO(m);
  m.msg_iov = vectors;
  m.msg_iovlen = data2 ? 2 : 1;
again:
  bytes_written = sendmsg(fd.fd, &m, MSG_NOSIGNAL);
  if (bytes_written < 0 && errno == EINTR) goto again;
  return bytes_written;
#else
  return _dbus_write_two(fd.fd, buffer1, start1, len1, buffer2, start2, len2);
#endif
}
int _dbus_read(int fd, DBusString *buffer, int count) {
  int bytes_read;
  int start;
  char *data;
  _dbus_assert(count >= 0);
  start = _dbus_string_get_length(buffer);
  if (!_dbus_string_lengthen(buffer, count)) {
      errno = ENOMEM;
      return -1;
  }
  data = _dbus_string_get_data_len(buffer, start, count);
again:
  bytes_read = read(fd, data, count);
  if (bytes_read < 0) {
      if (errno == EINTR) goto again;
      else {
          _dbus_string_set_length(buffer, start);
          return -1;
      }
  } else {
      _dbus_string_set_length(buffer, start + bytes_read);
  #if 0
      if (bytes_read > 0) _dbus_verbose_bytes_of_string(buffer, start, bytes_read);
  #endif
      return bytes_read;
  }
}
int _dbus_write(int fd, const DBusString *buffer, int start, int len) {
  const char *data;
  int bytes_written;
  data = _dbus_string_get_const_data_len(buffer, start, len);
again:
  bytes_written = write(fd, data, len);
  if (bytes_written < 0 && errno == EINTR) goto again;
#if 0
  if (bytes_written > 0) _dbus_verbose_bytes_of_string(buffer, start, bytes_written);
#endif
  return bytes_written;
}
int _dbus_write_two(int fd, const DBusString *buffer1, int start1, int len1, const DBusString *buffer2, int start2, int len2) {
  _dbus_assert(buffer1 != NULL);
  _dbus_assert(start1 >= 0);
  _dbus_assert(start2 >= 0);
  _dbus_assert(len1 >= 0);
  _dbus_assert(len2 >= 0);
#ifdef HAVE_WRITEV
  {
      struct iovec vectors[2];
      const char *data1;
      const char *data2;
      int bytes_written;
      data1 = _dbus_string_get_const_data_len(buffer1, start1, len1);
      if (buffer2 != NULL) data2 = _dbus_string_get_const_data_len(buffer2, start2, len2);
      else {
          data2 = NULL;
          start2 = 0;
          len2 = 0;
      }
      vectors[0].iov_base = (char*)data1;
      vectors[0].iov_len = len1;
      vectors[1].iov_base = (char*)data2;
      vectors[1].iov_len = len2;
  again:
      bytes_written = writev(fd, vectors,data2 ? 2 : 1);
      if (bytes_written < 0 && errno == EINTR) goto again;
      return bytes_written;
  }
#else
  {
      int ret1, ret2;
      ret1 = _dbus_write(fd, buffer1, start1, len1);
      if (ret1 == len1 && buffer2 != NULL) {
          ret2 = _dbus_write(fd, buffer2, start2, len2);
          if (ret2 < 0) ret2 = 0;
          return ret1 + ret2;
      } else return ret1;
  }
#endif
}
#define _DBUS_MAX_SUN_PATH_LENGTH  99
int _dbus_connect_unix_socket(const char *path, dbus_bool_t abstract, DBusError *error) {
  int fd;
  size_t path_len;
  struct sockaddr_un addr;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_verbose("connecting to unix socket %s abstract=%d\n", path, abstract);
  if (!_dbus_open_unix_socket(&fd, error)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      return -1;
  }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _DBUS_ZERO(addr);
  addr.sun_family = AF_UNIX;
  path_len = strlen(path);
  if (abstract) {
  #ifdef __linux__
      addr.sun_path[0] = '\0';
      path_len++;
      if (path_len > _DBUS_MAX_SUN_PATH_LENGTH) {
          dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Abstract socket name too long\n");
          _dbus_close(fd, NULL);
          return -1;
	  }
      strncpy(&addr.sun_path[1], path, path_len);
  #else
      dbus_set_error(error, DBUS_ERROR_NOT_SUPPORTED, "Operating system does not support abstract socket namespace\n");
      _dbus_close(fd, NULL);
      return -1;
  #endif
  } else {
      if (path_len > _DBUS_MAX_SUN_PATH_LENGTH) {
          dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Socket name too long\n");
          _dbus_close(fd, NULL);
          return -1;
	  }
      strncpy(addr.sun_path, path, path_len);
  }
  if (connect(fd, (struct sockaddr*)&addr, _DBUS_STRUCT_OFFSET(struct sockaddr_un, sun_path) + path_len) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to connect to socket %s: %s", path, _dbus_strerror(errno));
      _dbus_close(fd, NULL);
      return -1;
  }
  if (!_dbus_set_fd_nonblocking(fd, error)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      _dbus_close(fd, NULL);
      return -1;
  }
  return fd;
}
int _dbus_connect_exec(const char *path, char *const argv[], DBusError *error) {
  int fds[2];
  pid_t pid;
  int retval;
  dbus_bool_t cloexec_done = 0;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_verbose("connecting to process %s\n", path);
#ifdef SOCK_CLOEXEC
  retval = socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds);
  cloexec_done = (retval >= 0);
  if (retval < 0 && (errno == EINVAL || errno == EPROTOTYPE))
#endif
  retval = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  if (retval < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to create socket pair: %s", _dbus_strerror(errno));
      return -1;
  }
  if (!cloexec_done) {
      _dbus_fd_set_close_on_exec(fds[0]);
      _dbus_fd_set_close_on_exec(fds[1]);
  }
  fflush(stdout);
  fflush(stderr);
  pid = fork();
  if (pid < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to fork() to call %s: %s", path, _dbus_strerror(errno));
      close(fds[0]);
      close(fds[1]);
      return -1;
  }
  if (pid == 0) {
      close(fds[0]);
      dup2(fds[1], STDIN_FILENO);
      dup2(fds[1], STDOUT_FILENO);
      if (fds[1] != STDIN_FILENO && fds[1] != STDOUT_FILENO) close(fds[1]);
      _dbus_close_all();
      execvp(path, (char * const*)argv);
      fprintf(stderr, "Failed to execute process %s: %s\n", path, _dbus_strerror(errno));
      _exit(1);
  }
  close(fds[1]);
  if (!_dbus_set_fd_nonblocking(fds[0], error)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      close(fds[0]);
      return -1;
  }
  return fds[0];
}
int _dbus_listen_unix_socket(const char *path, dbus_bool_t abstract, DBusError *error) {
  int listen_fd;
  struct sockaddr_un addr;
  size_t path_len;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_verbose("listening on unix socket %s abstract=%d\n", path, abstract);
  if (!_dbus_open_unix_socket(&listen_fd, error)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      return -1;
  }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _DBUS_ZERO(addr);
  addr.sun_family = AF_UNIX;
  path_len = strlen(path);
  if (abstract) {
  #ifdef __linux__
      addr.sun_path[0] = '\0';
      path_len++;
      if (path_len > _DBUS_MAX_SUN_PATH_LENGTH) {
          dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Abstract socket name too long\n");
          _dbus_close(listen_fd, NULL);
          return -1;
	  }
      strncpy(&addr.sun_path[1], path, path_len);
  #else
      dbus_set_error(error, DBUS_ERROR_NOT_SUPPORTED, "Operating system does not support abstract socket namespace\n");
      _dbus_close(listen_fd, NULL);
      return -1;
  #endif
  } else {
      {
          struct stat sb;
          if (stat(path, &sb) == 0 && S_ISSOCK (sb.st_mode)) unlink(path);
      }
      if (path_len > _DBUS_MAX_SUN_PATH_LENGTH) {
          dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS, "Socket name too long\n");
          _dbus_close(listen_fd, NULL);
          return -1;
	  }
      strncpy(addr.sun_path, path, path_len);
  }
  if (bind(listen_fd, (struct sockaddr*)&addr, _DBUS_STRUCT_OFFSET(struct sockaddr_un, sun_path) + path_len) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to bind socket \"%s\": %s", path, _dbus_strerror(errno));
      _dbus_close(listen_fd, NULL);
      return -1;
  }
  if (listen(listen_fd, SOMAXCONN) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to listen on socket \"%s\": %s", path, _dbus_strerror(errno));
      _dbus_close(listen_fd, NULL);
      return -1;
  }
  if (!_dbus_set_fd_nonblocking(listen_fd, error)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      _dbus_close(listen_fd, NULL);
      return -1;
  }
  if (!abstract && chmod(path, 0777) < 0) _dbus_warn("Could not set mode 0777 on socket %s", path);
  return listen_fd;
}
int _dbus_listen_systemd_sockets(DBusSocket **fds, DBusError *error) {
#ifdef HAVE_SYSTEMD
  int r, n;
  int fd;
  DBusSocket *new_fds;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  n = sd_listen_fds(TRUE);
  if (n < 0) {
      dbus_set_error(error, _dbus_error_from_errno(-n), "Failed to acquire systemd socket: %s", _dbus_strerror(-n));
      return -1;
  }
  if (n <= 0) {
      dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS, "No socket received.");
      return -1;
  }
  for (fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd ++) {
      r = sd_is_socket(fd, AF_UNSPEC, SOCK_STREAM, 1);
      if (r < 0) {
          dbus_set_error(error, _dbus_error_from_errno(-r), "Failed to verify systemd socket type: %s", _dbus_strerror(-r));
          return -1;
      }
      if (!r) {
          dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS, "Passed socket has wrong type.");
          return -1;
      }
  }
  new_fds = dbus_new(DBusSocket, n);
  if (!new_fds) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, "Failed to allocate file handle array.");
      goto fail;
  }
  for (fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd ++) {
      if (!_dbus_set_fd_nonblocking(fd, error)) {
          _DBUS_ASSERT_ERROR_IS_SET(error);
          goto fail;
      }
      new_fds[fd - SD_LISTEN_FDS_START].fd = fd;
  }
  *fds = new_fds;
  return n;
fail:
  for (fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd ++) _dbus_close(fd, NULL);
  dbus_free(new_fds);
  return -1;
#else
  dbus_set_error_const(error, DBUS_ERROR_NOT_SUPPORTED,"dbus was compiled without systemd support");
  return -1;
#endif
}
DBusSocket _dbus_connect_tcp_socket(const char *host, const char *port, const char *family, DBusError *error) {
    return _dbus_connect_tcp_socket_with_nonce(host, port, family, (const char*)NULL, error);
}
DBusSocket _dbus_connect_tcp_socket_with_nonce(const char *host, const char *port, const char *family, const char *noncefile, DBusError *error) {
  int saved_errno = 0;
  DBusSocket fd = DBUS_SOCKET_INIT;
  int res;
  struct addrinfo hints;
  struct addrinfo *ai, *tmp;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _DBUS_ZERO(hints);
  if (!family) hints.ai_family = AF_UNSPEC;
  else if (!strcmp(family, "ipv4")) hints.ai_family = AF_INET;
  else if (!strcmp(family, "ipv6")) hints.ai_family = AF_INET6;
  else {
      dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Unknown address family %s", family);
      return _dbus_socket_get_invalid();
  }
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_ADDRCONFIG;
  if ((res = getaddrinfo(host, port, &hints, &ai)) != 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to lookup host/port: \"%s:%s\": %s (%d)", host, port, gai_strerror(res), res);
      return _dbus_socket_get_invalid();
  }
  tmp = ai;
  while(tmp) {
      if (!_dbus_open_socket(&fd.fd, tmp->ai_family, SOCK_STREAM, 0, error)) {
          freeaddrinfo(ai);
          _DBUS_ASSERT_ERROR_IS_SET(error);
          return _dbus_socket_get_invalid();
      }
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      if (connect(fd.fd, (struct sockaddr*) tmp->ai_addr, tmp->ai_addrlen) < 0) {
          saved_errno = errno;
          _dbus_close(fd.fd, NULL);
          fd.fd = -1;
          tmp = tmp->ai_next;
          continue;
      }
      break;
  }
  freeaddrinfo(ai);
  if (fd.fd == -1) {
      dbus_set_error(error, _dbus_error_from_errno(saved_errno),"Failed to connect to socket \"%s:%s\" %s", host, port, _dbus_strerror(saved_errno));
      return _dbus_socket_get_invalid();
  }
  if (noncefile != NULL) {
      DBusString noncefileStr;
      dbus_bool_t ret;
      _dbus_string_init_const(&noncefileStr, noncefile);
      ret = _dbus_send_nonce(fd, &noncefileStr, error);
      _dbus_string_free(&noncefileStr);
      if (!ret) {
          _dbus_close(fd.fd, NULL);
          return _dbus_socket_get_invalid();
      }
  }
  if (!_dbus_set_fd_nonblocking(fd.fd, error)) {
      _dbus_close(fd.fd, NULL);
      return _dbus_socket_get_invalid();
  }
  return fd;
}
int _dbus_listen_tcp_socket(const char *host, const char *port, const char *family, DBusString *retport, DBusSocket **fds_p, DBusError *error) {
  int saved_errno;
  int nlisten_fd = 0, res, i;
  DBusSocket *listen_fd = NULL;
  struct addrinfo hints;
  struct addrinfo *ai, *tmp;
  unsigned int reuseaddr;
  *fds_p = NULL;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _DBUS_ZERO(hints);
  if (!family) hints.ai_family = AF_UNSPEC;
  else if (!strcmp(family, "ipv4")) hints.ai_family = AF_INET;
  else if (!strcmp(family, "ipv6")) hints.ai_family = AF_INET6;
  else {
      dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Unknown address family %s", family);
      return -1;
  }
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;
redo_lookup_with_port:
  ai = NULL;
  if ((res = getaddrinfo(host, port, &hints, &ai)) != 0 || !ai) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to lookup host/port: \"%s:%s\": %s (%d)", host ? host : "*", port, gai_strerror(res), res);
      goto failed;
  }
  tmp = ai;
  while(tmp) {
      int fd = -1, tcp_nodelay_on;
      DBusSocket *newlisten_fd;
      if (!_dbus_open_socket(&fd, tmp->ai_family, SOCK_STREAM, 0, error)) {
          _DBUS_ASSERT_ERROR_IS_SET(error);
          goto failed;
      }
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      reuseaddr = 1;
      if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr))==-1) {
          _dbus_warn("Failed to set socket option \"%s:%s\": %s", host ? host : "*", port, _dbus_strerror(errno));
      }
      tcp_nodelay_on = 1;
      if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay_on, sizeof(tcp_nodelay_on)) == -1) {
          _dbus_warn("Failed to set TCP_NODELAY socket option \"%s:%s\": %s", host ? host : "*", port, _dbus_strerror(errno));
      }
      if (bind(fd, (struct sockaddr*)tmp->ai_addr, tmp->ai_addrlen) < 0) {
          saved_errno = errno;
          _dbus_close(fd, NULL);
          if (saved_errno == EADDRINUSE) {
              tmp = tmp->ai_next;
              continue;
          }
          dbus_set_error(error, _dbus_error_from_errno(saved_errno),"Failed to bind socket \"%s:%s\": %s", host ? host : "*", port, _dbus_strerror(saved_errno));
          goto failed;
      }
      if (listen(fd, 30) < 0) {
          saved_errno = errno;
          _dbus_close(fd, NULL);
          dbus_set_error(error, _dbus_error_from_errno(saved_errno),"Failed to listen on socket \"%s:%s\": %s", host ? host : "*", port, _dbus_strerror(saved_errno));
          goto failed;
      }
      newlisten_fd = dbus_realloc(listen_fd, sizeof(DBusSocket) * (nlisten_fd + 1));
      if (!newlisten_fd) {
          _dbus_close(fd, NULL);
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY,"Failed to allocate file handle array");
          goto failed;
      }
      listen_fd = newlisten_fd;
      listen_fd[nlisten_fd].fd = fd;
      nlisten_fd++;
      if (!_dbus_string_get_length(retport)) {
          if (!port || !strcmp(port, "0")) {
              int result;
              struct sockaddr_storage addr;
              socklen_t addrlen;
              char portbuf[50];
              addrlen = sizeof(addr);
              result = getsockname(fd, (struct sockaddr*) &addr, &addrlen);
              if (result == -1 || (res = getnameinfo((struct sockaddr*)&addr, addrlen, NULL, 0, portbuf, sizeof(portbuf),NI_NUMERICHOST |
                  NI_NUMERICSERV)) != 0) {
                  dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to resolve port \"%s:%s\": %s (%d)", host ? host : "*", port, gai_strerror(res), res);
                  goto failed;
              }
              if (!_dbus_string_append(retport, portbuf)) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  goto failed;
              }
              port = _dbus_string_get_const_data(retport);
              freeaddrinfo(ai);
              goto redo_lookup_with_port;
          } else {
              if (!_dbus_string_append(retport, port)) {
                    dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                    goto failed;
              }
          }
      }
      tmp = tmp->ai_next;
  }
  freeaddrinfo(ai);
  ai = NULL;
  if (!nlisten_fd) {
      errno = EADDRINUSE;
      dbus_set_error (error, _dbus_error_from_errno (errno),"Failed to bind socket \"%s:%s\": %s", host ? host : "*", port, _dbus_strerror (errno));
      goto failed;
  }
  for (i = 0 ; i < nlisten_fd ; i++) {
      if (!_dbus_set_fd_nonblocking (listen_fd[i].fd, error)) goto failed;
  }
  *fds_p = listen_fd;
  return nlisten_fd;
failed:
  if (ai) freeaddrinfo(ai);
  for (i = 0 ; i < nlisten_fd ; i++) _dbus_close(listen_fd[i].fd, NULL);
  dbus_free(listen_fd);
  return -1;
}
static dbus_bool_t
write_credentials_byte(int server_fd, DBusError *error) {
  int bytes_written;
  char buf[1] = { '\0' };
#if defined(HAVE_CMSGCRED)
  union {
	  struct cmsghdr hdr;
	  char cred[CMSG_SPACE(sizeof(struct cmsghdr))];
  } cmsg;
  struct iovec iov;
  struct msghdr msg;
  iov.iov_base = buf;
  iov.iov_len = 1;
  _DBUS_ZERO(msg);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = (caddr_t)&cmsg;
  msg.msg_controllen = CMSG_SPACE(sizeof(struct cmsghdr));
  _DBUS_ZERO(cmsg);
  cmsg.hdr.cmsg_len = CMSG_LEN(sizeof(struct cmsghdr));
  cmsg.hdr.cmsg_level = SOL_SOCKET;
  //cmsg.hdr.cmsg_type = SCM_CREDS;
#endif
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
 again:
#if defined(HAVE_CMSGCRED)
  bytes_written = sendmsg(server_fd, &msg, 0
                      #if HAVE_DECL_MSG_NOSIGNAL
                          | MSG_NOSIGNAL
                      #endif
                         );
  if (bytes_written < 0 && errno == EINVAL)
#endif
  {
      bytes_written = send(server_fd, buf, 1, 0
                       #if HAVE_DECL_MSG_NOSIGNAL
                           | MSG_NOSIGNAL
                       #endif
                          );
  }
  if (bytes_written < 0 && errno == EINTR) goto again;
  if (bytes_written < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to write credentials byte: %s", _dbus_strerror(errno));
      return FALSE;
  } else if (bytes_written == 0) {
      dbus_set_error(error, DBUS_ERROR_IO_ERROR,"wrote zero bytes writing credentials byte");
      return FALSE;
  } else {
      _dbus_assert(bytes_written == 1);
      _dbus_verbose("wrote credentials byte\n");
      return TRUE;
  }
}
static dbus_bool_t add_linux_security_label_to_credentials(int client_fd, DBusCredentials *credentials) {
#if defined(__linux__) && defined(SO_PEERSEC)
  DBusString buf;
  socklen_t len = 1024;
  dbus_bool_t oom = FALSE;
  if (!_dbus_string_init_preallocated(&buf, len) || !_dbus_string_set_length(&buf, len)) return FALSE;
  while(getsockopt(client_fd, SOL_SOCKET, SO_PEERSEC, _dbus_string_get_data(&buf), &len) < 0) {
      int e = errno;
      _dbus_verbose("getsockopt failed with %s, len now %lu\n", _dbus_strerror(e), (unsigned long)len);
      if (e != ERANGE || len <= _dbus_string_get_length_uint(&buf)) {
          _dbus_verbose("Failed to getsockopt(SO_PEERSEC): %s\n", _dbus_strerror(e));
          goto out;
      }
      if (!_dbus_string_set_length(&buf, len)) {
          oom = TRUE;
          goto out;
      }
      _dbus_verbose("will try again with %lu\n", (unsigned long)len);
  }
  if (len <= 0) {
      _dbus_verbose("getsockopt(SO_PEERSEC) yielded <= 0 bytes: %lu\n", (unsigned long)len);
      goto out;
  }
  if (len > _dbus_string_get_length_uint(&buf)) {
      _dbus_verbose("%lu > %u", (unsigned long)len, _dbus_string_get_length_uint(&buf));
      _dbus_assert_not_reached("getsockopt(SO_PEERSEC) overflowed");
  }
  if (_dbus_string_get_byte(&buf, len - 1) == 0) {
      _dbus_verbose("subtracting trailing \\0\n");
      len--;
  }
  if (!_dbus_string_set_length(&buf, len)) {
      _dbus_assert_not_reached("shortening string should not lead to OOM");
      oom = TRUE;
      goto out;
  }
  if (strlen(_dbus_string_get_const_data(&buf)) != len) {
      _dbus_verbose("security label from kernel had an embedded \\0, ignoring it\n");
      goto out;
  }
  _dbus_verbose("getsockopt(SO_PEERSEC): %lu bytes excluding \\0: %s\n", (unsigned long)len, _dbus_string_get_const_data(&buf));
  if (!_dbus_credentials_add_linux_security_label(credentials, _dbus_string_get_const_data(&buf))) {
      oom = TRUE;
      goto out;
  }
out:
  _dbus_string_free(&buf);
  return !oom;
#else
  return TRUE;
#endif
}
dbus_bool_t _dbus_read_credentials_socket(DBusSocket client_fd, DBusCredentials *credentials, DBusError *error) {
  struct msghdr msg;
  struct iovec iov;
  char buf;
  dbus_uid_t uid_read;
  dbus_pid_t pid_read;
  int bytes_read;
#ifdef HAVE_CMSGCRED
  union {
      struct cmsghdr hdr;
      char cred[CMSG_SPACE(sizeof(struct cmsghdr))];
  } cmsg;
#endif
  _DBUS_STATIC_ASSERT(sizeof(pid_t) <= sizeof(dbus_pid_t));
  _DBUS_STATIC_ASSERT(sizeof(uid_t) <= sizeof(dbus_uid_t));
  _DBUS_STATIC_ASSERT(sizeof(gid_t) <= sizeof(dbus_gid_t));
  uid_read = DBUS_UID_UNSET;
  pid_read = DBUS_PID_UNSET;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_credentials_clear(credentials);
  iov.iov_base = &buf;
  iov.iov_len = 1;
  _DBUS_ZERO(msg);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
#if defined(HAVE_CMSGCRED)
  _DBUS_ZERO(cmsg);
  msg.msg_control = (caddr_t)&cmsg;
  msg.msg_controllen = CMSG_SPACE(sizeof(struct cmsghdr));
#endif
again:
  bytes_read = recvmsg(client_fd.fd, &msg, 0);
  if (bytes_read < 0) {
      if (errno == EINTR) goto again;
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to read credentials byte: %s", _dbus_strerror(errno));
      return FALSE;
  } else if (bytes_read == 0) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Failed to read credentials byte (zero-length read)");
      return FALSE;
  } else if (buf != '\0') {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Credentials byte was not nul");
      return FALSE;
  }
  _dbus_verbose("read credentials byte\n");
  {
  #ifdef SO_PEERCRED
  #ifdef __OpenBSD__
      struct sockpeercred cr;
  #else
      struct ucred cr;
  #endif
      socklen_t cr_len = sizeof(cr);
      if (getsockopt(client_fd.fd, SOL_SOCKET, SO_PEERCRED, &cr, &cr_len) != 0) { _dbus_verbose("Failed to getsockopt(SO_PEERCRED): %s\n", _dbus_strerror(errno)); }
      else if (cr_len != sizeof(cr)) { _dbus_verbose("Failed to getsockopt(SO_PEERCRED), returned %d bytes, expected %d\n", cr_len, (int)sizeof(cr)); }
      else {
          pid_read = cr.pid;
          uid_read = cr.uid;
      }
  #elif defined(HAVE_UNPCBID) && defined(LOCAL_PEEREID)
      struct unpcbid cr;
      socklen_t cr_len = sizeof(cr);
      if (getsockopt(client_fd.fd, 0, LOCAL_PEEREID, &cr, &cr_len) != 0) { _dbus_verbose("Failed to getsockopt(LOCAL_PEEREID): %s\n", _dbus_strerror(errno)); }
      else if (cr_len != sizeof(cr)) {
          _dbus_verbose("Failed to getsockopt(LOCAL_PEEREID), returned %d bytes, expected %d\n", cr_len, (int)sizeof(cr));
      } else {
          pid_read = cr.unp_pid;
          uid_read = cr.unp_euid;
      }
  #elif defined(HAVE_CMSGCRED)
      struct cmsgcred *cred;
      struct cmsghdr *cmsgp;
      for (cmsgp = CMSG_FIRSTHDR(&msg); cmsgp != NULL; cmsgp = CMSG_NXTHDR(&msg, cmsgp)) {
          if (cmsgp->cmsg_type == SCM_CREDS && cmsgp->cmsg_level == SOL_SOCKET && cmsgp->cmsg_len >= CMSG_LEN(sizeof(struct cmsgcred))) {
              cred = (struct cmsgcred*)CMSG_DATA(cmsgp);
              pid_read = cred->cmcred_pid;
              uid_read = cred->cmcred_euid;
              break;
          }
      }
  #elif defined(HAVE_GETPEERUCRED)
      ucred_t * ucred = NULL;
      if (getpeerucred(client_fd.fd, &ucred) == 0) {
      #ifdef HAVE_ADT
          adt_session_data_t *adth = NULL;
      #endif
          pid_read = ucred_getpid(ucred);
          uid_read = ucred_geteuid(ucred);
      #ifdef HAVE_ADT
          if (adt_start_session(&adth, NULL, 0) || (adth == NULL)) _dbus_verbose("Failed to adt_start_session(): %s\n", _dbus_strerror(errno));
          else {
              if (adt_set_from_ucred(adth, ucred, ADT_NEW)) _dbus_verbose("Failed to adt_set_from_ucred(): %s\n", _dbus_strerror(errno));
              else {
                  adt_export_data_t *data = NULL;
                  size_t size = adt_export_session_data(adth, &data);
                  if (size <= 0) _dbus_verbose("Failed to adt_export_session_data(): %s\n", _dbus_strerror(errno));
                  else {
                      _dbus_credentials_add_adt_audit_data(credentials, data, size);
                      free(data);
                  }
              }
              (void)adt_end_session(adth);
          }
      #endif
      } else _dbus_verbose("Failed to getpeerucred() credentials: %s\n", _dbus_strerror(errno));
      if (ucred != NULL) ucred_free(ucred);
  #elif defined(HAVE_GETPEEREID)
      uid_t euid;
      gid_t egid;
      if (getpeereid(client_fd.fd, &euid, &egid) == 0) uid_read = euid;
      else _dbus_verbose("Failed to getpeereid() credentials: %s\n", _dbus_strerror(errno));
  #else
      #warning Socket credentials not supported on this Unix OS
      #warning Please tell https://bugs.freedesktop.org/enter_bug.cgi?product=DBus
      #if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__linux__) || defined(__OpenBSD__) || defined(__NetBSD__)
      #error Credentials passing not working on this OS is a regression!
      #endif
      _dbus_verbose("Socket credentials not supported on this OS\n");
#endif
  }
  _dbus_verbose("Credentials:  pid "DBUS_PID_FORMAT "  uid "DBUS_UID_FORMAT "\n", pid_read, uid_read);
  if (pid_read != DBUS_PID_UNSET) {
      if (!_dbus_credentials_add_pid(credentials, pid_read)) {
          _DBUS_SET_OOM(error);
          return FALSE;
      }
  }
  if (uid_read != DBUS_UID_UNSET) {
      if (!_dbus_credentials_add_unix_uid(credentials, uid_read)) {
          _DBUS_SET_OOM(error);
          return FALSE;
      }
  }
  if (!add_linux_security_label_to_credentials(client_fd.fd, credentials)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_send_credentials_socket(DBusSocket server_fd, DBusError *error) {
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (write_credentials_byte(server_fd.fd, error)) return TRUE;
  else return FALSE;
}
DBusSocket _dbus_accept(DBusSocket listen_fd) {
  DBusSocket client_fd;
  struct sockaddr addr;
  socklen_t addrlen;
#ifdef HAVE_ACCEPT4
  dbus_bool_t cloexec_done;
#endif
  addrlen = sizeof(addr);
retry:
#ifdef HAVE_ACCEPT4
  client_fd.fd = accept4(listen_fd.fd, &addr, &addrlen, SOCK_CLOEXEC);
  cloexec_done = client_fd.fd >= 0;
  if (client_fd.fd < 0 && (errno == ENOSYS || errno == EINVAL))
#endif
  client_fd.fd = accept(listen_fd.fd, &addr, &addrlen);
  if (client_fd.fd < 0) {
      if (errno == EINTR) goto retry;
  }
  _dbus_verbose("client fd %d accepted\n", client_fd.fd);
#ifdef HAVE_ACCEPT4
  if (!cloexec_done)
#endif
  _dbus_fd_set_close_on_exec(client_fd.fd);
  return client_fd;
}
dbus_bool_t _dbus_check_dir_is_private_to_user(DBusString *dir, DBusError *error) {
  const char *directory;
  struct stat sb;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  directory = _dbus_string_get_const_data(dir);
  if (stat(directory, &sb) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno), "%s", _dbus_strerror(errno));
      return FALSE;
  }
  if (sb.st_uid != geteuid()) {
      dbus_set_error(error, DBUS_ERROR_FAILED, "%s directory is owned by user %lu, not %lu", directory, (unsigned long)sb.st_uid, (unsigned long)geteuid());
      return FALSE;
  }
  if ((S_IROTH & sb.st_mode) || (S_IWOTH & sb.st_mode) || (S_IRGRP & sb.st_mode) || (S_IWGRP & sb.st_mode)) {
      dbus_set_error(error, DBUS_ERROR_FAILED, "%s directory is not private to the user", directory);
      return FALSE;
  }
  return TRUE;
}
static dbus_bool_t fill_user_info_from_passwd(struct passwd *p, DBusUserInfo *info, DBusError *error) {
  _dbus_assert(p->pw_name != NULL);
  _dbus_assert(p->pw_dir != NULL);
  info->uid = p->pw_uid;
  info->primary_gid = p->pw_gid;
  info->username = _dbus_strdup(p->pw_name);
  info->homedir = _dbus_strdup(p->pw_dir);
  if (info->username == NULL || info->homedir == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return FALSE;
  }
  return TRUE;
}
static dbus_bool_t fill_user_info(DBusUserInfo *info, dbus_uid_t uid, const DBusString *username, DBusError *error) {
  const char *username_c;
  _dbus_assert(username != NULL || uid != DBUS_UID_UNSET);
  _dbus_assert(username == NULL || uid == DBUS_UID_UNSET);
  info->uid = DBUS_UID_UNSET;
  info->primary_gid = DBUS_GID_UNSET;
  info->group_ids = NULL;
  info->n_group_ids = 0;
  info->username = NULL;
  info->homedir = NULL;
  if (username != NULL) username_c = _dbus_string_get_const_data(username);
  else username_c = NULL;
#if defined(HAVE_POSIX_GETPWNAM_R) || defined(HAVE_NONPOSIX_GETPWNAM_R)
  {
      struct passwd *p;
      int result;
      size_t buflen;
      char *buf;
      struct passwd p_str;
      buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
      if ((long) buflen <= 0) buflen = 1024;
      result = -1;
      while(1) {
          buf = dbus_malloc(buflen);
          if (buf == NULL) {
              dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
              return FALSE;
          }
          p = NULL;
      #ifdef HAVE_POSIX_GETPWNAM_R
          if (uid != DBUS_UID_UNSET) result = getpwuid_r(uid, &p_str, buf, buflen, &p);
          else result = getpwnam_r(username_c, &p_str, buf, buflen, &p);
      #else
          if (uid != DBUS_UID_UNSET) p = getpwuid_r(uid, &p_str, buf, buflen);
          else p = getpwnam_r(username_c, &p_str, buf, buflen);
          result = 0;
      #endif
          if (result == ERANGE && buflen < 512 * 1024) {
              dbus_free(buf);
              buflen *= 2;
          } else break;
      }
      if (result == 0 && p == &p_str) {
          if (!fill_user_info_from_passwd(p, info, error)) {
              dbus_free(buf);
              return FALSE;
          }
          dbus_free(buf);
      } else {
          dbus_set_error(error, _dbus_error_from_errno(errno),"User \"%s\" unknown or no memory to allocate password entry\n",
                         username_c ? username_c : "???");
          _dbus_verbose("User %s unknown\n", username_c ? username_c : "???");
          dbus_free(buf);
          return FALSE;
      }
  }
#else
  {
      struct passwd *p;
      if (uid != DBUS_UID_UNSET) p = getpwuid(uid);
      else p = getpwnam(username_c);
      if (p != NULL) {
          if (!fill_user_info_from_passwd(p, info, error)) return FALSE;
      } else {
          dbus_set_error(error, _dbus_error_from_errno (errno), "User \"%s\" unknown or no memory to allocate password entry\n", username_c ? username_c : "???");
          _dbus_verbose("User %s unknown\n", username_c ? username_c : "???");
          return FALSE;
      }
  }
#endif
  username_c = info->username;
#ifdef HAVE_GETGROUPLIST
  {
      gid_t *buf;
      int buf_count;
      int i;
      int initial_buf_count;
      initial_buf_count = 17;
      buf_count = initial_buf_count;
      buf = dbus_new (gid_t, buf_count);
      if (buf == NULL) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          goto failed;
      }
      if (getgrouplist(username_c, info->primary_gid, buf, &buf_count) < 0) {
          gid_t *new;
          if (buf_count == initial_buf_count) buf_count *= 16;
          new = dbus_realloc(buf, buf_count * sizeof(buf[0]));
          if (new == NULL) {
              dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
              dbus_free(buf);
              goto failed;
          }
          buf = new;
          errno = 0;
          if (getgrouplist(username_c, info->primary_gid, buf, &buf_count) < 0) {
              if (errno == 0) {
                  _dbus_warn("It appears that username \"%s\" is in more than %d groups.\nProceeding with just the first %d groups.", username_c,
                             buf_count, buf_count);
              } else {
                  dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to get groups for username \"%s\" primary GID " DBUS_GID_FORMAT
                                 ": %s\n", username_c, info->primary_gid, _dbus_strerror(errno));
                  dbus_free(buf);
                  goto failed;
              }
          }
      }
      info->group_ids = dbus_new(dbus_gid_t, buf_count);
      if (info->group_ids == NULL) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          dbus_free(buf);
          goto failed;
      }
    for (i = 0; i < buf_count; ++i) info->group_ids[i] = buf[i];
    info->n_group_ids = buf_count;
    dbus_free(buf);
  }
#else
  {
      info->group_ids = dbus_new(dbus_gid_t, 1);
      if (info->group_ids == NULL) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          goto failed;
      }
      info->n_group_ids = 1;
      (info->group_ids)[0] = info->primary_gid;
  }
#endif
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  return TRUE;
failed:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  return FALSE;
}
dbus_bool_t _dbus_user_info_fill(DBusUserInfo *info, const DBusString *username, DBusError *error) {
  return fill_user_info(info, DBUS_UID_UNSET, username, error);
}
dbus_bool_t _dbus_user_info_fill_uid(DBusUserInfo *info, dbus_uid_t uid, DBusError *error) {
  return fill_user_info(info, uid,NULL, error);
}
dbus_bool_t _dbus_credentials_add_from_current_process(DBusCredentials *credentials) {
  _DBUS_STATIC_ASSERT(sizeof (pid_t) <= sizeof(dbus_pid_t));
  _DBUS_STATIC_ASSERT(sizeof (uid_t) <= sizeof(dbus_uid_t));
  _DBUS_STATIC_ASSERT(sizeof (gid_t) <= sizeof(dbus_gid_t));
  if (!_dbus_credentials_add_pid(credentials, _dbus_getpid())) return FALSE;
  if (!_dbus_credentials_add_unix_uid(credentials, _dbus_geteuid())) return FALSE;
  return TRUE;
}
dbus_bool_t _dbus_append_user_from_current_process(DBusString *str) {
  return _dbus_string_append_uint(str, _dbus_geteuid());
}
dbus_pid_t _dbus_getpid(void) {
  return getpid();
}
dbus_uid_t _dbus_getuid(void) {
  return getuid();
}
dbus_uid_t _dbus_geteuid(void) {
  return geteuid();
}
unsigned long _dbus_pid_for_log(void) {
  return getpid();
}
dbus_bool_t _dbus_parse_uid(const DBusString *uid_str, dbus_uid_t *uid) {
  int end;
  long val;
  if (_dbus_string_get_length(uid_str) == 0) {
      _dbus_verbose("UID string was zero length\n");
      return FALSE;
  }
  val = -1;
  end = 0;
  if (!_dbus_string_parse_int(uid_str, 0, &val, &end)) {
      _dbus_verbose("could not parse string as a UID\n");
      return FALSE;
  }
  if (end != _dbus_string_get_length(uid_str)) {
      _dbus_verbose("string contained trailing stuff after UID\n");
      return FALSE;
  }
  *uid = val;
  return TRUE;
}
#if !DBUS_USE_SYNC
static pthread_mutex_t atomic_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
dbus_int32_t _dbus_atomic_inc(DBusAtomic *atomic) {
#if DBUS_USE_SYNC
  return __sync_add_and_fetch(&atomic->value, 1)-1;
#else
  dbus_int32_t res;
  pthread_mutex_lock(&atomic_mutex);
  res = atomic->value;
  atomic->value += 1;
  pthread_mutex_unlock(&atomic_mutex);
  return res;
#endif
}
dbus_int32_t _dbus_atomic_dec(DBusAtomic *atomic) {
#if DBUS_USE_SYNC
  return __sync_sub_and_fetch(&atomic->value, 1)+1;
#else
  dbus_int32_t res;
  pthread_mutex_lock(&atomic_mutex);
  res = atomic->value;
  atomic->value -= 1;
  pthread_mutex_unlock(&atomic_mutex);
  return res;
#endif
}
dbus_int32_t _dbus_atomic_get(DBusAtomic *atomic) {
#if DBUS_USE_SYNC
  __sync_synchronize();
  return atomic->value;
#else
  dbus_int32_t res;
  pthread_mutex_lock(&atomic_mutex);
  res = atomic->value;
  pthread_mutex_unlock(&atomic_mutex);
  return res;
#endif
}
int _dbus_poll(DBusPollFD *fds, int n_fds, int timeout_milliseconds) {
#if defined(HAVE_POLL) && !defined(BROKEN_POLL)
  if (timeout_milliseconds < -1) timeout_milliseconds = -1;
  return poll(fds, n_fds, timeout_milliseconds);
#else
  fd_set read_set, write_set, err_set;
  int max_fd = 0;
  int i;
  struct timeval tv;
  int ready;
  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_ZERO(&err_set);
  for (i = 0; i < n_fds; i++) {
      DBusPollFD *fdp = &fds[i];
      if (fdp->events & _DBUS_POLLIN) FD_SET(fdp->fd, &read_set);
      if (fdp->events & _DBUS_POLLOUT) FD_SET(fdp->fd, &write_set);
      FD_SET(fdp->fd, &err_set);
      max_fd = MAX(max_fd, fdp->fd);
  }
  tv.tv_sec = timeout_milliseconds / 1000;
  tv.tv_usec = (timeout_milliseconds % 1000) * 1000;
  ready = select(max_fd + 1, &read_set, &write_set, &err_set, timeout_milliseconds < 0 ? NULL : &tv);
  if (ready > 0) {
      for (i = 0; i < n_fds; i++) {
          DBusPollFD *fdp = &fds[i];
          fdp->revents = 0;
          if (FD_ISSET(fdp->fd, &read_set)) fdp->revents |= _DBUS_POLLIN;
          if (FD_ISSET(fdp->fd, &write_set)) fdp->revents |= _DBUS_POLLOUT;
          if (FD_ISSET(fdp->fd, &err_set)) fdp->revents |= _DBUS_POLLERR;
	  }
  }
  return ready;
#endif
}
void _dbus_get_monotonic_time(long *tv_sec, long *tv_usec) {
#ifdef HAVE_MONOTONIC_CLOCK
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  if (tv_sec) *tv_sec = ts.tv_sec;
  if (tv_usec) *tv_usec = ts.tv_nsec / 1000;
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  if (tv_sec) *tv_sec = t.tv_sec;
  if (tv_usec) *tv_usec = t.tv_usec;
#endif
}
void _dbus_get_real_time(long *tv_sec, long *tv_usec) {
  struct timeval t;
  gettimeofday(&t, NULL);
  if (tv_sec) *tv_sec = t.tv_sec;
  if (tv_usec) *tv_usec = t.tv_usec;
}
dbus_bool_t _dbus_ensure_directory(const DBusString *filename, DBusError *error) {
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  if (mkdir (filename_c, 0700) < 0) {
      if (errno == EEXIST) return TRUE;
      dbus_set_error(error, DBUS_ERROR_FAILED, "Failed to create directory %s: %s\n", filename_c, _dbus_strerror(errno));
      return FALSE;
  } else return TRUE;
}
dbus_bool_t _dbus_create_directory(const DBusString *filename, DBusError *error) {
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  if (mkdir(filename_c, 0700) < 0) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Failed to create directory %s: %s\n", filename_c, _dbus_strerror(errno));
      return FALSE;
  } else return TRUE;
}
dbus_bool_t _dbus_concat_dir_and_file(DBusString *dir, const DBusString *next_component) {
  dbus_bool_t dir_ends_in_slash;
  dbus_bool_t file_starts_with_slash;
  if (_dbus_string_get_length(dir) == 0 || _dbus_string_get_length(next_component) == 0) return TRUE;
  dir_ends_in_slash = '/' == _dbus_string_get_byte(dir,_dbus_string_get_length(dir) - 1);
  file_starts_with_slash = '/' == _dbus_string_get_byte(next_component, 0);
  if (dir_ends_in_slash && file_starts_with_slash) _dbus_string_shorten(dir, 1);
  else if (!(dir_ends_in_slash || file_starts_with_slash)) {
      if (!_dbus_string_append_byte(dir, '/')) return FALSE;
  }
  return _dbus_string_copy(next_component, 0, dir, _dbus_string_get_length(dir));
}
#define NANOSECONDS_PER_SECOND  1000000000
#define MICROSECONDS_PER_SECOND  1000000
#define MILLISECONDS_PER_SECOND  1000
#define NANOSECONDS_PER_MILLISECOND  1000000
#define MICROSECONDS_PER_MILLISECOND  1000
void _dbus_sleep_milliseconds(int milliseconds) {
#ifdef HAVE_NANOSLEEP
  struct timespec req;
  struct timespec rem;
  req.tv_sec = milliseconds / MILLISECONDS_PER_SECOND;
  req.tv_nsec = (milliseconds % MILLISECONDS_PER_SECOND) * NANOSECONDS_PER_MILLISECOND;
  rem.tv_sec = 0;
  rem.tv_nsec = 0;
  while (nanosleep(&req, &rem) < 0 && errno == EINTR) req = rem;
#elif defined(HAVE_USLEEP)
  usleep(milliseconds * MICROSECONDS_PER_MILLISECOND);
#else
  sleep(MAX (milliseconds / 1000, 1));
#endif
}
dbus_bool_t _dbus_generate_random_bytes(DBusString *str, int n_bytes, DBusError *error) {
  int old_len;
  int fd;
  int result;
  old_len = _dbus_string_get_length(str);
  fd = -1;
  fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not open /dev/urandom: %s", _dbus_strerror(errno));
      return FALSE;
  }
  _dbus_verbose("/dev/urandom fd %d opened\n", fd);
  result = _dbus_read(fd, str, n_bytes);
  if (result != n_bytes) {
      if (result < 0) dbus_set_error(error, _dbus_error_from_errno(errno),"Could not read /dev/urandom: %s", _dbus_strerror(errno));
      else dbus_set_error(error, DBUS_ERROR_IO_ERROR,"Short read from /dev/urandom");
      _dbus_close(fd, NULL);
      _dbus_string_set_length(str, old_len);
      return FALSE;
  }
  _dbus_verbose("Read %d bytes from /dev/urandom\n", n_bytes);
  _dbus_close(fd, NULL);
  return TRUE;
}
void _dbus_exit(int code) {
  _exit(code);
}
const char* _dbus_strerror(int error_number) {
  const char *msg;
  msg = strerror(error_number);
  if (msg == NULL) msg = "unknown";
  return msg;
}
void _dbus_disable_sigpipe(void) {
  signal(SIGPIPE, SIG_IGN);
}
void _dbus_fd_set_close_on_exec(int fd) {
  int val;
  val = fcntl(fd, F_GETFD, 0);
  if (val < 0) return;
  val |= FD_CLOEXEC;
  fcntl(fd, F_SETFD, val);
}
dbus_bool_t _dbus_close(int fd, DBusError *error) {
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
again:
  if (close(fd) < 0) {
      if (errno == EINTR) goto again;
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not close fd %d", fd);
      return FALSE;
  }
  return TRUE;
}
int _dbus_dup(int fd, DBusError *error) {
  int new_fd;
#ifdef F_DUPFD_CLOEXEC
  dbus_bool_t cloexec_done;
  new_fd = fcntl(fd, F_DUPFD_CLOEXEC, 3);
  cloexec_done = new_fd >= 0;
  if (new_fd < 0 && errno == EINVAL)
#endif
  new_fd = fcntl(fd, F_DUPFD, 3);
  if (new_fd < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not duplicate fd %d", fd);
      return -1;
  }
#ifdef F_DUPFD_CLOEXEC
  if (!cloexec_done)
#endif
  _dbus_fd_set_close_on_exec(new_fd);
  return new_fd;
}
dbus_bool_t _dbus_set_socket_nonblocking(DBusSocket fd, DBusError *error) {
  return _dbus_set_fd_nonblocking(fd.fd, error);
}
static dbus_bool_t _dbus_set_fd_nonblocking(int fd, DBusError *error) {
  int val;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  val = fcntl(fd, F_GETFL, 0);
  if (val < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to get flags from file descriptor %d: %s", fd, _dbus_strerror(errno));
      _dbus_verbose("Failed to get flags for fd %d: %s\n", fd, _dbus_strerror(errno));
      return FALSE;
  }
  if (fcntl(fd, F_SETFL, val | O_NONBLOCK) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to set nonblocking flag of file descriptor %d: %s", fd, _dbus_strerror(errno));
      _dbus_verbose("Failed to set fd %d nonblocking: %s\n", fd, _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
void _dbus_print_backtrace(void) {
#if defined(HAVE_BACKTRACE) && defined(DBUS_BUILT_R_DYNAMIC)
  void *bt[500];
  int bt_size;
  int i;
  char **syms;
  bt_size = backtrace(bt, 500);
  syms = backtrace_symbols(bt, bt_size);
  i = 0;
  while(i < bt_size) {
      fprintf(stderr, "  %s\n", syms[i]);
      ++i;
  }
  fflush(stderr);
  free(syms);
#elif defined(HAVE_BACKTRACE) && ! defined(DBUS_BUILT_R_DYNAMIC)
  fprintf(stderr, "  D-Bus not built with -rdynamic so unable to print a backtrace\n");
#else
  fprintf(stderr, "  D-Bus not compiled with backtrace support so unable to print a backtrace\n");
#endif
}
dbus_bool_t _dbus_socketpair(DBusSocket *fd1, DBusSocket *fd2, dbus_bool_t blocking, DBusError *error) {
#ifdef HAVE_SOCKETPAIR
  int fds[2];
  int retval;
#ifdef SOCK_CLOEXEC
  dbus_bool_t cloexec_done;
  retval = socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds);
  cloexec_done = retval >= 0;
  if (retval < 0 && (errno == EINVAL || errno == EPROTOTYPE))
#endif
  retval = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  if (retval < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not create full-duplex pipe");
      return FALSE;
  }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
#ifdef SOCK_CLOEXEC
  if (!cloexec_done)
#endif
  {
      _dbus_fd_set_close_on_exec(fds[0]);
      _dbus_fd_set_close_on_exec(fds[1]);
  }
  if (!blocking && (!_dbus_set_fd_nonblocking(fds[0], NULL) || !_dbus_set_fd_nonblocking(fds[1], NULL))) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not set full-duplex pipe nonblocking");
      _dbus_close(fds[0], NULL);
      _dbus_close(fds[1], NULL);
      return FALSE;
  }
  fd1->fd = fds[0];
  fd2->fd = fds[1];
  _dbus_verbose("full-duplex pipe %d <-> %d\n", fd1->fd, fd2->fd);
  return TRUE;
#else
  _dbus_warn("_dbus_socketpair() not implemented on this OS");
  dbus_set_error(error, DBUS_ERROR_FAILED, "_dbus_socketpair() not implemented on this OS");
  return FALSE;
#endif
}
int _dbus_printf_string_upper_bound(const char *format, va_list args) {
  char static_buf[1024];
  int bufsize = sizeof(static_buf);
  int len;
  va_list args_copy = NULL;
  //DBUS_VA_COPY(args_copy, args);
  len = vsnprintf(static_buf, bufsize, format, args_copy);
  va_end(args_copy);
  if (len == bufsize) {
      //DBUS_VA_COPY(args_copy, args);
      if (vsnprintf(static_buf, 1, format, args_copy) == 1) len = -1;
      va_end(args_copy);
  }
  while(len < 0) {
      char *buf;
      bufsize *= 2;
      buf = dbus_malloc(bufsize);
      if (buf == NULL) return -1;
      //DBUS_VA_COPY(args_copy, args);
      len = vsnprintf(buf, bufsize, format, args_copy);
      va_end(args_copy);
      dbus_free (buf);
      if (len == bufsize) len = -1;
  }
  return len;
}
const char* _dbus_get_tmpdir(void) {
  static const char* tmpdir = NULL;
  if (!_DBUS_LOCK(sysdeps)) return NULL;
  if (tmpdir == NULL) {
      if (tmpdir == NULL) tmpdir = getenv("TMPDIR");
      if (tmpdir == NULL) tmpdir = getenv("TMP");
      if (tmpdir == NULL) tmpdir = getenv("TEMP");
      if (tmpdir == NULL) tmpdir = "/tmp";
  }
  _DBUS_UNLOCK(sysdeps);
  _dbus_assert(tmpdir != NULL);
  return tmpdir;
}
#if defined(DBUS_ENABLE_X11_AUTOLAUNCH) || defined(DBUS_ENABLE_LAUNCHD)
static dbus_bool_t _read_subprocess_line_argv(const char *progpath, dbus_bool_t path_fallback, const char * const *argv, DBusString *result, DBusError *error) {
  int result_pipe[2] = { -1, -1 };
  int errors_pipe[2] = { -1, -1 };
  pid_t pid;
  int ret;
  int status;
  int orig_len;
  dbus_bool_t retval;
  sigset_t new_set, old_set;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  retval = FALSE;
  sigemptyset(&new_set);
  sigaddset(&new_set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &new_set, &old_set);
  orig_len = _dbus_string_get_length(result);
#define READ_END  0
#define WRITE_END  1
  if (pipe (result_pipe) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to create a pipe to call %s: %s", progpath, _dbus_strerror(errno));
      _dbus_verbose("Failed to create a pipe to call %s: %s\n", progpath, _dbus_strerror(errno));
      goto out;
  }
  if (pipe(errors_pipe) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to create a pipe to call %s: %s", progpath, _dbus_strerror(errno));
      _dbus_verbose("Failed to create a pipe to call %s: %s\n", progpath, _dbus_strerror(errno));
      goto out;
  }
  fflush(stdout);
  fflush(stderr);
  pid = fork();
  if (pid < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to fork() to call %s: %s", progpath, _dbus_strerror(errno));
      _dbus_verbose("Failed to fork() to call %s: %s\n", progpath, _dbus_strerror(errno));
      goto out;
  }
  if (pid == 0) {
      const char *error_str;
      if (!_dbus_ensure_standard_fds(DBUS_FORCE_STDIN_NULL, &error_str)) {
          int saved_errno = errno;
          if (write(errors_pipe[WRITE_END], error_str, strlen(error_str)) < 0 || write(errors_pipe[WRITE_END], ": ", 2) < 0);
          error_str = _dbus_strerror (saved_errno);
          if (write(errors_pipe[WRITE_END], error_str, strlen(error_str)) < 0);
          _exit(1);
      }
      close(result_pipe[READ_END]);
      close(errors_pipe[READ_END]);
      if (dup2(result_pipe[WRITE_END], 1) == -1) _exit(1);
      if (dup2(errors_pipe[WRITE_END], 2) == -1) _exit(1);
      _dbus_close_all();
      sigprocmask(SIG_SETMASK, &old_set, NULL);
      if (progpath[0] == '/') {
          execv(progpath, (char * const*)argv);
          if (path_fallback) execvp(strrchr(progpath, '/') + 1, (char * const*)argv);
      } else execvp(progpath, (char * const*)argv);
      _exit(1);
  }
  close(result_pipe[WRITE_END]);
  close(errors_pipe[WRITE_END]);
  result_pipe[WRITE_END] = -1;
  errors_pipe[WRITE_END] = -1;
  ret = 0;
  do {
      ret = _dbus_read(result_pipe[READ_END], result, 1024);
  } while(ret > 0);
  do {
      ret = waitpid(pid, &status, 0);
  } while(ret == -1 && errno == EINTR);
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 ) {
      DBusString error_message;
      if (!_dbus_string_init(&error_message)) {
          _DBUS_SET_OOM(error);
          goto out;
      }
      ret = 0;
      do {
          ret = _dbus_read(errors_pipe[READ_END], &error_message, 1024);
      } while(ret > 0);
      _dbus_string_set_length(result, orig_len);
      if (_dbus_string_get_length(&error_message) > 0) {
          dbus_set_error(error, DBUS_ERROR_SPAWN_EXEC_FAILED,"%s terminated abnormally with the following error: %s", progpath,
                         _dbus_string_get_data(&error_message));
      } else dbus_set_error(error, DBUS_ERROR_SPAWN_EXEC_FAILED,"%s terminated abnormally without any error message", progpath);
      goto out;
  }
  retval = TRUE;
out:
  sigprocmask(SIG_SETMASK, &old_set, NULL);
  if (retval) { _DBUS_ASSERT_ERROR_IS_CLEAR(error); }
  else { _DBUS_ASSERT_ERROR_IS_SET(error); }
  if (result_pipe[0] != -1) close(result_pipe[0]);
  if (result_pipe[1] != -1) close(result_pipe[1]);
  if (errors_pipe[0] != -1) close(errors_pipe[0]);
  if (errors_pipe[1] != -1) close(errors_pipe[1]);
  return retval;
}
#endif
dbus_bool_t _dbus_get_autolaunch_address(const char *scope, DBusString *address, DBusError *error) {
#ifdef DBUS_ENABLE_X11_AUTOLAUNCH
  static const char arg_dbus_launch[] = "dbus-launch";
  static const char arg_autolaunch[] = "--autolaunch";
  static const char arg_binary_syntax[] = "--binary-syntax";
  static const char arg_close_stderr[] = "--close-stderr";
  const char *display;
  const char *progpath;
  const char *argv[6];
  int i;
  DBusString uuid;
  dbus_bool_t retval;
  if (_dbus_check_setuid()) {
      dbus_set_error_const(error, DBUS_ERROR_NOT_SUPPORTED,"Unable to autolaunch when setuid");
      return FALSE;
  }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  retval = FALSE;
  display = _dbus_getenv("DISPLAY");
  if (display == NULL || display[0] == '\0') {
      dbus_set_error_const(error, DBUS_ERROR_NOT_SUPPORTED,"Unable to autolaunch a dbus-daemon without a $DISPLAY for X11");
      return FALSE;
  }
  if (!_dbus_string_init(&uuid)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_get_local_machine_uuid_encoded(&uuid, error)) goto out;
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  progpath = _dbus_getenv("DBUS_TEST_DBUS_LAUNCH");
  if (progpath == NULL)
#endif
    progpath = DBUS_BINDIR "/dbus-launch";
  i = 0;
  argv[i] = arg_dbus_launch;
  ++i;
  argv[i] = arg_autolaunch;
  ++i;
  argv[i] = _dbus_string_get_data(&uuid);
  ++i;
  argv[i] = arg_binary_syntax;
  ++i;
  argv[i] = arg_close_stderr;
  ++i;
  argv[i] = NULL;
  ++i;
  _dbus_assert(i == _DBUS_N_ELEMENTS(argv));
  retval = _read_subprocess_line_argv(progpath, TRUE, argv, address, error);
 out:
  _dbus_string_free(&uuid);
  return retval;
#else
  dbus_set_error_const(error, DBUS_ERROR_NOT_SUPPORTED, "Using X11 for dbus-daemon autolaunch was disabled at compile time, set your DBUS_SESSION_BUS_ADDRESS "
                       "instead");
  return FALSE;
#endif
}
dbus_bool_t _dbus_read_local_machine_uuid(DBusGUID *machine_id, dbus_bool_t create_if_not_found, DBusError *error) {
  DBusError our_error = DBUS_ERROR_INIT;
  DBusError etc_error = DBUS_ERROR_INIT;
  DBusString filename;
  dbus_bool_t b;
  _dbus_string_init_const(&filename, DBUS_MACHINE_UUID_FILE);
  b = _dbus_read_uuid_file(&filename, machine_id, FALSE, &our_error);
  if (b) return TRUE;
  _dbus_string_init_const(&filename, "/etc/machine-id");
  b = _dbus_read_uuid_file(&filename, machine_id, FALSE, &etc_error);
  if (b) {
      if (create_if_not_found) {
          _dbus_string_init_const(&filename, DBUS_MACHINE_UUID_FILE);
          _dbus_write_uuid_file(&filename, machine_id, NULL);
      }
      dbus_error_free(&our_error);
      return TRUE;
  }
  if (!create_if_not_found) {
      dbus_set_error(error, etc_error.name,"D-Bus library appears to be incorrectly set up: see the manual page for dbus-uuidgen to correct "
                     "this issue. (%s; %s)", our_error.message, etc_error.message);
      dbus_error_free(&our_error);
      dbus_error_free(&etc_error);
      return FALSE;
  }
  dbus_error_free(&our_error);
  dbus_error_free(&etc_error);
  _dbus_string_init_const(&filename, DBUS_MACHINE_UUID_FILE);
  if (!_dbus_generate_uuid(machine_id, error)) return FALSE;
  return _dbus_write_uuid_file(&filename, machine_id, error);
}
dbus_bool_t _dbus_lookup_launchd_socket(DBusString *socket_path, const char *launchd_env_var, DBusError *error) {
#ifdef DBUS_ENABLE_LAUNCHD
  char *argv[4];
  int i;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (_dbus_check_setuid()) {
      dbus_set_error_const(error, DBUS_ERROR_NOT_SUPPORTED, "Unable to find launchd socket when setuid");
      return FALSE;
  }
  i = 0;
  argv[i] = "launchctl";
  ++i;
  argv[i] = "getenv";
  ++i;
  argv[i] = (char*)launchd_env_var;
  ++i;
  argv[i] = NULL;
  ++i;
  _dbus_assert(i == _DBUS_N_ELEMENTS(argv));
  if (!_read_subprocess_line_argv(argv[0], TRUE, argv, socket_path, error)) return FALSE;
  if (_dbus_string_get_length(socket_path) == 0) return FALSE;
  _dbus_string_shorten(socket_path, 1);
  return TRUE;
#else
  dbus_set_error(error, DBUS_ERROR_NOT_SUPPORTED,"can't lookup socket from launchd; launchd support not compiled in");
  return FALSE;
#endif
}
#ifndef DBUS_ENABLE_LAUNCHD
static dbus_bool_t _dbus_lookup_session_address_launchd(DBusString *address, DBusError  *error) {
  dbus_bool_t valid_socket;
  DBusString socket_path;
  if (_dbus_check_setuid()) {
      dbus_set_error_const(error, DBUS_ERROR_NOT_SUPPORTED, "Unable to find launchd socket when setuid");
      return FALSE;
  }
  if (!_dbus_string_init(&socket_path)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  valid_socket = _dbus_lookup_launchd_socket(&socket_path, "DBUS_LAUNCHD_SESSION_BUS_SOCKET", error);
  if (dbus_error_is_set(error)) {
      _dbus_string_free(&socket_path);
      return FALSE;
  }
  if (!valid_socket) {
      dbus_set_error(error, "no socket path", "launchd did not provide a socket path, verify that org.freedesktop.dbus-session.plist is loaded!");
      _dbus_string_free(&socket_path);
      return FALSE;
  }
  if (!_dbus_string_append(address, "unix:path=")) {
      _DBUS_SET_OOM(error);
      _dbus_string_free(&socket_path);
      return FALSE;
  }
  if (!_dbus_string_copy(&socket_path, 0, address, _dbus_string_get_length(address))) {
      _DBUS_SET_OOM(error);
      _dbus_string_free(&socket_path);
      return FALSE;
  }
  _dbus_string_free(&socket_path);
  return TRUE;
}
#endif
dbus_bool_t _dbus_lookup_user_bus(dbus_bool_t *supported, DBusString *address, DBusError *error) {
  const char *runtime_dir = _dbus_getenv("XDG_RUNTIME_DIR");
  dbus_bool_t ret = FALSE;
  struct stat stbuf;
  DBusString user_bus_path;
  if (runtime_dir == NULL) {
      _dbus_verbose("XDG_RUNTIME_DIR not found in environment");
      *supported = FALSE;
      return TRUE;
  }
  if (!_dbus_string_init(&user_bus_path)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_string_append_printf(&user_bus_path, "%s/bus", runtime_dir)) {
      _DBUS_SET_OOM(error);
      goto out;
  }
  if (lstat(_dbus_string_get_const_data(&user_bus_path), &stbuf) == -1) {
      _dbus_verbose("XDG_RUNTIME_DIR/bus not available: %s", _dbus_strerror(errno));
      *supported = FALSE;
      ret = TRUE;
      goto out;
  }
  if (stbuf.st_uid != getuid()) {
      _dbus_verbose("XDG_RUNTIME_DIR/bus owned by uid %ld, not our uid %ld", (long)stbuf.st_uid, (long)getuid());
      *supported = FALSE;
      ret = TRUE;
      goto out;
  }
  if ((stbuf.st_mode & S_IFMT) != S_IFSOCK) {
      _dbus_verbose("XDG_RUNTIME_DIR/bus is not a socket: st_mode = 0o%lo", (long)stbuf.st_mode);
      *supported = FALSE;
      ret = TRUE;
      goto out;
  }
  if (!_dbus_string_append(address, "unix:path=") || !_dbus_address_append_escaped(address, &user_bus_path)) {
      _DBUS_SET_OOM(error);
      goto out;
  }
  *supported = TRUE;
  ret = TRUE;
out:
  _dbus_string_free(&user_bus_path);
  return ret;
}
dbus_bool_t _dbus_lookup_session_address(dbus_bool_t *supported, DBusString *address, DBusError *error) {
#ifdef DBUS_ENABLE_LAUNCHD
  *supported = TRUE;
  return _dbus_lookup_session_address_launchd(address, error);
#else
  *supported = FALSE;
  if (!_dbus_lookup_user_bus(supported, address, error)) return FALSE;
  else if (*supported) return TRUE;
  return TRUE;
#endif
}
void _dbus_flush_caches(void) {
  _dbus_user_database_flush_system();
}
dbus_bool_t _dbus_append_keyring_directory_for_credentials(DBusString *directory, DBusCredentials *credentials) {
  DBusString homedir;
  DBusString dotdir;
  dbus_uid_t uid;
  _dbus_assert(credentials != NULL);
  _dbus_assert(!_dbus_credentials_are_anonymous(credentials));
  if (!_dbus_string_init(&homedir)) return FALSE;
  uid = _dbus_credentials_get_unix_uid(credentials);
  _dbus_assert(uid != DBUS_UID_UNSET);
  if (!_dbus_homedir_from_uid(uid, &homedir)) goto failed;
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  {
      const char *override;
      override = _dbus_getenv("DBUS_TEST_HOMEDIR");
      if (override != NULL && *override != '\0') {
          _dbus_string_set_length(&homedir, 0);
          if (!_dbus_string_append(&homedir, override)) goto failed;
          _dbus_verbose("Using fake homedir for testing: %s\n", _dbus_string_get_const_data(&homedir));
      } else {
          static dbus_bool_t already_warned = FALSE;
          if (!already_warned) {
              _dbus_warn("Using %s for testing, set DBUS_TEST_HOMEDIR to avoid", _dbus_string_get_const_data(&homedir));
              already_warned = TRUE;
          }
      }
  }
#endif
  _dbus_string_init_const(&dotdir, ".dbus-keyrings");
  if (!_dbus_concat_dir_and_file(&homedir, &dotdir)) goto failed;
  if (!_dbus_string_copy(&homedir, 0, directory, _dbus_string_get_length(directory))) goto failed;
  _dbus_string_free(&homedir);
  return TRUE;
failed:
  _dbus_string_free(&homedir);
  return FALSE;
}
dbus_bool_t _dbus_daemon_publish_session_bus_address(const char* addr, const char *scope) {
  return TRUE;
}
void _dbus_daemon_unpublish_session_bus_address(void) {}
dbus_bool_t _dbus_get_is_errno_eagain_or_ewouldblock(int e) {
  return e == EAGAIN || e == EWOULDBLOCK;
}
dbus_bool_t _dbus_delete_directory(const DBusString *filename, DBusError *error) {
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  if (rmdir (filename_c) != 0) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Failed to remove directory %s: %s\n", filename_c, _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_socket_can_pass_unix_fd(DBusSocket fd) {
#ifdef SCM_RIGHTS
  union {
      struct sockaddr sa;
      struct sockaddr_storage storage;
      struct sockaddr_un un;
  } sa_buf;
  socklen_t sa_len = sizeof(sa_buf);
  _DBUS_ZERO(sa_buf);
  if (getsockname(fd.fd, &sa_buf.sa, &sa_len) < 0) return FALSE;
  return sa_buf.sa.sa_family == AF_UNIX;
#else
  return FALSE;
#endif
}
void _dbus_close_all(void) {
  int maxfds, i;
#ifdef __linux__
  DIR *d;
  d = opendir("/proc/self/fd");
  if (d) {
      for (;;) {
          struct dirent *de;
          int fd;
          long l;
          char *e = NULL;
          de = readdir(d);
          if (!de) break;
          if (de->d_name[0] == '.') continue;
          errno = 0;
          l = strtol(de->d_name, &e, 10);
          if (errno != 0 || e == NULL || *e != '\0') continue;
          fd = (int) l;
          if (fd < 3) continue;
          if (fd == dirfd(d)) continue;
          close(fd);
      }
      closedir(d);
      return;
  }
#endif
  maxfds = sysconf(_SC_OPEN_MAX);
  if (maxfds < 0) maxfds = 1024;
  for (i = 3; i < maxfds; i++) close(i);
}
dbus_bool_t _dbus_check_setuid(void) {
#if 0 && defined(HAVE_LIBC_ENABLE_SECURE)
  {
      extern int __libc_enable_secure;
      return __libc_enable_secure;
  }
#elif defined(HAVE_ISSETUGID)
  return issetugid ();
#else
  uid_t ruid, euid, suid;
  gid_t rgid, egid, sgid;
  static dbus_bool_t check_setuid_initialised;
  static dbus_bool_t is_setuid;
  if (_DBUS_UNLIKELY(!check_setuid_initialised)) {
  #ifdef HAVE_GETRESUID
      if (getresuid(&ruid, &euid, &suid) != 0 || getresgid (&rgid, &egid, &sgid) != 0)
  #endif
      {
          suid = ruid = getuid();
          sgid = rgid = getgid();
          euid = geteuid();
          egid = getegid();
      }
      check_setuid_initialised = TRUE;
      is_setuid = (ruid != euid || ruid != suid || rgid != egid || rgid != sgid);
  }
  return is_setuid;
#endif
}
dbus_bool_t _dbus_append_address_from_socket(DBusSocket fd, DBusString *address, DBusError *error) {
  union {
      struct sockaddr sa;
      struct sockaddr_storage storage;
      struct sockaddr_un un;
      struct sockaddr_in ipv4;
      struct sockaddr_in6 ipv6;
  } socket;
  char hostip[INET6_ADDRSTRLEN];
  socklen_t size = sizeof(socket);
  DBusString path_str;
  if (getsockname(fd.fd, &socket.sa, &size)) goto err;
  switch(socket.sa.sa_family) {
      case AF_UNIX:
          if (socket.un.sun_path[0]=='\0') {
              _dbus_string_init_const(&path_str, &(socket.un.sun_path[1]));
              if (_dbus_string_append(address, "unix:abstract=") && _dbus_address_append_escaped(address, &path_str)) return TRUE;
          } else {
              _dbus_string_init_const(&path_str, socket.un.sun_path);
              if (_dbus_string_append(address, "unix:path=") && _dbus_address_append_escaped(address, &path_str)) return TRUE;
          }
          break;
      case AF_INET:
          if (inet_ntop(AF_INET, &socket.ipv4.sin_addr, hostip, sizeof(hostip)))
              if (_dbus_string_append_printf(address, "tcp:family=ipv4,host=%s,port=%u", hostip, ntohs(socket.ipv4.sin_port))) return TRUE;
          break;
  #ifdef AF_INET6
      case AF_INET6:
          _dbus_string_init_const(&path_str, hostip);
          if (inet_ntop(AF_INET6, &socket.ipv6.sin6_addr, hostip, sizeof(hostip)))
              if (_dbus_string_append_printf(address, "tcp:family=ipv6,port=%u,host=", ntohs(socket.ipv6.sin6_port)) &&
                  _dbus_address_append_escaped(address, &path_str))
                  return TRUE;
          break;
  #endif
      default:
          dbus_set_error(error,_dbus_error_from_errno(EINVAL),"Failed to read address from socket: Unknown socket type.");
          return FALSE;
  }
err:
  dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to open socket: %s", _dbus_strerror(errno));
  return FALSE;
}
int _dbus_save_socket_errno(void) {
  return errno;
}
void _dbus_restore_socket_errno(int saved_errno) {
  errno = saved_errno;
}
static const char *syslog_tag = "dbus";
#ifdef HAVE_SYSLOG_H
static DBusLogFlags log_flags = DBUS_LOG_FLAGS_STDERR;
#endif
void _dbus_init_system_log(const char *tag, DBusLogFlags flags) {
  _dbus_assert((flags & (DBUS_LOG_FLAGS_STDERR | DBUS_LOG_FLAGS_SYSTEM_LOG)) != 0);
  syslog_tag = tag;
#ifdef HAVE_SYSLOG_H
  log_flags = flags;
  if (log_flags & DBUS_LOG_FLAGS_SYSTEM_LOG) openlog(tag, LOG_PID, LOG_DAEMON);
#endif
}
void _dbus_logv(DBusSystemLogSeverity severity, const char *msg, va_list args) {
  va_list tmp = NULL;
#ifdef HAVE_SYSLOG_H
  if (log_flags & DBUS_LOG_FLAGS_SYSTEM_LOG) {
      int flags;
      switch(severity) {
          case DBUS_SYSTEM_LOG_INFO: flags =  LOG_DAEMON | LOG_INFO; break;
          case DBUS_SYSTEM_LOG_WARNING: flags =  LOG_DAEMON | LOG_WARNING; break;
          case DBUS_SYSTEM_LOG_SECURITY: flags = LOG_AUTH | LOG_NOTICE; break;
          case DBUS_SYSTEM_LOG_ERROR: flags = LOG_DAEMON|LOG_CRIT; break;
          default: _dbus_assert_not_reached("invalid log severity");
      }
      //DBUS_VA_COPY(tmp, args);
      vsyslog(flags, msg, tmp);
      va_end(tmp);
  }
  if (log_flags & DBUS_LOG_FLAGS_STDERR)
#endif
  {
      //DBUS_VA_COPY(tmp, args);
      fprintf(stderr, "%s[" DBUS_PID_FORMAT "]: ", syslog_tag, _dbus_getpid());
      vfprintf(stderr, msg, tmp);
      fputc('\n', stderr);
      va_end(tmp);
  }
}
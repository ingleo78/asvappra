#define __USE_GNU
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#define __USE_GNU 1
#include <zconf.h>
#include "../curl/curl_setup_once.h"
#include "config.h"
#if defined(DBUS_WIN) || !defined(DBUS_UNIX)
#error "This file only makes sense on Unix OSs"
#endif
#include "dbus-spawn.h"
#include "dbus-sysdeps-unix.h"
#include "dbus-internals.h"
#include "dbus-test.h"
#include "dbus-protocol.h"
#if defined(__APPLE__)
#include <crt_externs.h>

#define environ(*_NSGetEnviron ())
#elif !HAVE_DECL_ENVIRON
extern char **environ;
#endif
typedef enum {
  READ_STATUS_OK,
  READ_STATUS_ERROR,
  READ_STATUS_EOF
} ReadStatus;
static ReadStatus read_ints(int fd, int *buf, int n_ints_in_buf, int *n_ints_read, DBusError *error) {
  size_t bytes = 0;
  ReadStatus retval;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  retval = READ_STATUS_OK;
  while(TRUE) {
      ssize_t chunk;
      size_t to_read;
      to_read = sizeof(int) * n_ints_in_buf - bytes;
      if (to_read == 0) break;
  again:
      chunk = read(fd,((char*)buf) + bytes, to_read);
      if (chunk < 0 && errno == EINTR) goto again;
      if (chunk < 0) {
          dbus_set_error(error, DBUS_ERROR_SPAWN_FAILED,"Failed to read from child pipe (%s)", _dbus_strerror(errno));
          retval = READ_STATUS_ERROR;
          break;
      } else if (chunk == 0) {
          retval = READ_STATUS_EOF;
          break;
      } else bytes += chunk;
  }
  *n_ints_read = (int)(bytes / sizeof(int));
  return retval;
}
static ReadStatus read_pid(int fd, pid_t *buf, DBusError *error) {
  size_t bytes = 0;
  ReadStatus retval;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  retval = READ_STATUS_OK;
  while(TRUE) {
      ssize_t chunk;
      size_t to_read;
      to_read = sizeof(pid_t) - bytes;
      if (to_read == 0) break;
  again:
      chunk = read(fd,((char*)buf) + bytes, to_read);
      if (chunk < 0 && errno == EINTR) goto again;
      if (chunk < 0) {
          dbus_set_error(error, DBUS_ERROR_SPAWN_FAILED,"Failed to read from child pipe (%s)", _dbus_strerror(errno));
          retval = READ_STATUS_ERROR;
          break;
      } else if (chunk == 0) {
          retval = READ_STATUS_EOF;
          break;
      } else bytes += chunk;
  }
  return retval;
}
enum {
  CHILD_EXITED,
  CHILD_FORK_FAILED,
  CHILD_EXEC_FAILED,
  CHILD_PID
};
struct DBusBabysitter {
  int refcount;
  char *log_name;
  DBusSocket socket_to_babysitter;
  int error_pipe_from_child;
  pid_t sitter_pid;
  pid_t grandchild_pid;
  DBusWatchList *watches;
  DBusWatch *error_watch;
  DBusWatch *sitter_watch;
  DBusBabysitterFinishedFunc finished_cb;
  void *finished_data;
  int errnum;
  int status;
  unsigned int have_child_status : 1;
  unsigned int have_fork_errnum : 1;
  unsigned int have_exec_errnum : 1;
};
static DBusBabysitter* _dbus_babysitter_new(void) {
  DBusBabysitter *sitter;
  sitter = dbus_new0(DBusBabysitter, 1);
  if (sitter == NULL) return NULL;
  sitter->refcount = 1;
  sitter->socket_to_babysitter.fd = -1;
  sitter->error_pipe_from_child = -1;
  sitter->sitter_pid = -1;
  sitter->grandchild_pid = -1;
  sitter->watches = _dbus_watch_list_new();
  if (sitter->watches == NULL) goto failed;
  return sitter;
failed:
  _dbus_babysitter_unref(sitter);
  return NULL;
}
DBusBabysitter *_dbus_babysitter_ref(DBusBabysitter *sitter) {
  _dbus_assert(sitter != NULL);
  _dbus_assert(sitter->refcount > 0);
  sitter->refcount += 1;
  return sitter;
}
static void close_socket_to_babysitter(DBusBabysitter *sitter);
static void close_error_pipe_from_child(DBusBabysitter *sitter);
void _dbus_babysitter_unref(DBusBabysitter *sitter) {
  _dbus_assert(sitter != NULL);
  _dbus_assert(sitter->refcount > 0);
  sitter->refcount -= 1;
  if (sitter->refcount == 0) {
      close_socket_to_babysitter(sitter);
      close_error_pipe_from_child(sitter);
      if (sitter->sitter_pid > 0) {
          int status;
          int ret;
          ret = waitpid(sitter->sitter_pid, &status, WNOHANG);
          if (ret == 0) kill(sitter->sitter_pid, SIGKILL);
          if (ret == 0) {
              do {
                  ret = waitpid(sitter->sitter_pid, &status, 0);
              } while(_DBUS_UNLIKELY(ret < 0 && errno == EINTR));
          }
          if (ret < 0) {
              if (errno == ECHILD) _dbus_warn("Babysitter process not available to be reaped; should not happen");
              else _dbus_warn("Unexpected error %d in waitpid() for babysitter: %s", errno, _dbus_strerror(errno));
          } else {
              _dbus_verbose("Reaped %ld, waiting for babysitter %ld\n", (long)ret, (long)sitter->sitter_pid);
              if (WIFEXITED(sitter->status)) _dbus_verbose("Babysitter exited with status %d\n", WEXITSTATUS(sitter->status));
              else if (WIFSIGNALED(sitter->status)) _dbus_verbose("Babysitter received signal %d\n", WTERMSIG(sitter->status));
              else _dbus_verbose("Babysitter exited abnormally\n");
          }
          sitter->sitter_pid = -1;
      }
      if (sitter->watches) _dbus_watch_list_free(sitter->watches);
      dbus_free(sitter->log_name);
      dbus_free(sitter);
  }
}
static ReadStatus read_data(DBusBabysitter *sitter, int fd) {
  int what;
  int got;
  DBusError error = DBUS_ERROR_INIT;
  ReadStatus r;
  r = read_ints(fd, &what, 1, &got, &error);
  switch(r) {
      case READ_STATUS_ERROR:
          _dbus_warn("Failed to read data from fd %d: %s", fd, error.message);
          dbus_error_free(&error);
          return r;
      case READ_STATUS_EOF: return r;
      case READ_STATUS_OK: break;
      default: _dbus_assert_not_reached("invalid ReadStatus");
  }
  if (got == 1) {
      switch(what) {
          case CHILD_EXITED: case CHILD_FORK_FAILED: case CHILD_EXEC_FAILED: {
                  int arg;
                  r = read_ints(fd, &arg, 1, &got, &error);
                  switch(r) {
                      case READ_STATUS_ERROR:
                          _dbus_warn("Failed to read arg from fd %d: %s", fd, error.message);
                          dbus_error_free(&error);
                          return r;
                      case READ_STATUS_EOF: return r;
                      case READ_STATUS_OK: break;
                      default: _dbus_assert_not_reached ("invalid ReadStatus");
                  }
                  if (got == 1) {
                      if (what == CHILD_EXITED) {
                          sitter->have_child_status = TRUE;
                          sitter->status = arg;
                          _dbus_verbose ("recorded child status exited = %d signaled = %d exitstatus = %d termsig = %d\n", WIFEXITED (sitter->status),
                                         WIFSIGNALED (sitter->status), WEXITSTATUS (sitter->status), WTERMSIG (sitter->status));
                      } else if (what == CHILD_FORK_FAILED) {
                          sitter->have_fork_errnum = TRUE;
                          sitter->errnum = arg;
                          _dbus_verbose ("recorded fork errnum %d\n", sitter->errnum);
                      } else if (what == CHILD_EXEC_FAILED) {
                          sitter->have_exec_errnum = TRUE;
                          sitter->errnum = arg;
                          _dbus_verbose ("recorded exec errnum %d\n", sitter->errnum);
                      }
                  }
              }
              break;
          case CHILD_PID: {
                  pid_t pid = -1;
                  r = read_pid(fd, &pid, &error);
                  switch(r) {
                      case READ_STATUS_ERROR:
                          _dbus_warn("Failed to read PID from fd %d: %s", fd, error.message);
                          dbus_error_free(&error);
                          return r;
                      case READ_STATUS_EOF: return r;
                      case READ_STATUS_OK: break;
                      default: _dbus_assert_not_reached("invalid ReadStatus");
                  }
                  sitter->grandchild_pid = pid;
                  _dbus_verbose("recorded grandchild pid %d\n", sitter->grandchild_pid);
              }
              break;
          default: _dbus_warn("Unknown message received from babysitter process");
      }
  }
  return r;
}
static void close_socket_to_babysitter(DBusBabysitter *sitter) {
  _dbus_verbose("Closing babysitter\n");
  if (sitter->sitter_watch != NULL) {
      _dbus_assert(sitter->watches != NULL);
      _dbus_watch_list_remove_watch(sitter->watches,  sitter->sitter_watch);
      _dbus_watch_invalidate(sitter->sitter_watch);
      _dbus_watch_unref(sitter->sitter_watch);
      sitter->sitter_watch = NULL;
  }
  if (sitter->socket_to_babysitter.fd >= 0) {
      _dbus_close_socket(sitter->socket_to_babysitter, NULL);
      sitter->socket_to_babysitter.fd = -1;
    }
}
static void close_error_pipe_from_child(DBusBabysitter *sitter) {
  _dbus_verbose("Closing child error\n");
  if (sitter->error_watch != NULL) {
      _dbus_assert(sitter->watches != NULL);
      _dbus_watch_list_remove_watch(sitter->watches,  sitter->error_watch);
      _dbus_watch_invalidate(sitter->error_watch);
      _dbus_watch_unref(sitter->error_watch);
      sitter->error_watch = NULL;
  }
  if (sitter->error_pipe_from_child >= 0) {
      _dbus_close(sitter->error_pipe_from_child, NULL);
      sitter->error_pipe_from_child = -1;
  }
}
static void handle_babysitter_socket(DBusBabysitter *sitter, int revents) {
  if (revents & _DBUS_POLLIN) {
      _dbus_verbose("Reading data from babysitter\n");
      if (read_data(sitter, sitter->socket_to_babysitter.fd) != READ_STATUS_OK) close_socket_to_babysitter(sitter);
  } else if (revents & (_DBUS_POLLERR | _DBUS_POLLHUP)) close_socket_to_babysitter(sitter);
}
static void handle_error_pipe(DBusBabysitter *sitter, int revents) {
  if (revents & _DBUS_POLLIN) {
      _dbus_verbose("Reading data from child error\n");
      if (read_data(sitter, sitter->error_pipe_from_child) != READ_STATUS_OK) close_error_pipe_from_child(sitter);
  } else if (revents & (_DBUS_POLLERR | _DBUS_POLLHUP)) close_error_pipe_from_child(sitter);
}
static dbus_bool_t babysitter_iteration(DBusBabysitter *sitter, dbus_bool_t block) {
  DBusPollFD fds[2];
  int i;
  dbus_bool_t descriptors_ready;
  descriptors_ready = FALSE;
  i = 0;
  if (sitter->error_pipe_from_child >= 0) {
      fds[i].fd = sitter->error_pipe_from_child;
      fds[i].events = _DBUS_POLLIN;
      fds[i].revents = 0;
      ++i;
  }
  if (sitter->socket_to_babysitter.fd >= 0) {
      fds[i].fd = sitter->socket_to_babysitter.fd;
      fds[i].events = _DBUS_POLLIN;
      fds[i].revents = 0;
      ++i;
  }
  if (i > 0) {
      int ret;
      do {
          ret = _dbus_poll(fds, i, 0);
      } while(ret < 0 && errno == EINTR);
      if (ret == 0 && block) {
          do {
              ret = _dbus_poll(fds, i, -1);
          } while(ret < 0 && errno == EINTR);
      }
      if (ret > 0) {
          descriptors_ready = TRUE;
          while(i > 0) {
              --i;
              if (fds[i].fd == sitter->error_pipe_from_child) handle_error_pipe(sitter, fds[i].revents);
              else if (fds[i].fd == sitter->socket_to_babysitter.fd) handle_babysitter_socket(sitter, fds[i].revents);
          }
      }
  }
  return descriptors_ready;
}
#define LIVE_CHILDREN(sitter) ((sitter)->socket_to_babysitter.fd >= 0 || (sitter)->error_pipe_from_child >= 0)
void _dbus_babysitter_kill_child(DBusBabysitter *sitter) {
  while(LIVE_CHILDREN(sitter) && sitter->grandchild_pid == -1) babysitter_iteration(sitter, TRUE);
  _dbus_verbose("Got child PID %ld for killing\n", (long)sitter->grandchild_pid);
  if (sitter->grandchild_pid == -1) return;
  kill(sitter->grandchild_pid, SIGKILL);
}
dbus_bool_t _dbus_babysitter_get_child_exited(DBusBabysitter *sitter) {
  while(LIVE_CHILDREN(sitter) && babysitter_iteration(sitter, FALSE));
  return sitter->socket_to_babysitter.fd < 0;
}
dbus_bool_t _dbus_babysitter_get_child_exit_status(DBusBabysitter *sitter, int *status) {
  if (!_dbus_babysitter_get_child_exited(sitter)) _dbus_assert_not_reached("Child has not exited");
  if (!sitter->have_child_status || !(WIFEXITED(sitter->status))) return FALSE;
  *status = WEXITSTATUS(sitter->status);
  return TRUE;
}
void _dbus_babysitter_set_child_exit_error(DBusBabysitter *sitter, DBusError *error) {
  if (!_dbus_babysitter_get_child_exited(sitter)) return;
  if (sitter->have_exec_errnum) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_EXEC_FAILED, "Failed to execute program %s: %s", sitter->log_name, _dbus_strerror(sitter->errnum));
  } else if (sitter->have_fork_errnum) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY,"Failed to fork a new process %s: %s", sitter->log_name, _dbus_strerror(sitter->errnum));
  } else if (sitter->have_child_status) {
      if (WIFEXITED(sitter->status)) {
          dbus_set_error(error, DBUS_ERROR_SPAWN_CHILD_EXITED,"Process %s exited with status %d", sitter->log_name, WEXITSTATUS(sitter->status));
      } else if (WIFSIGNALED(sitter->status)) {
          dbus_set_error(error, DBUS_ERROR_SPAWN_CHILD_SIGNALED, "Process %s received signal %d", sitter->log_name, WTERMSIG(sitter->status));
      } else dbus_set_error(error, DBUS_ERROR_FAILED,"Process %s exited abnormally", sitter->log_name);
  } else dbus_set_error(error, DBUS_ERROR_FAILED,"Process %s exited, reason unknown", sitter->log_name);
}
dbus_bool_t _dbus_babysitter_set_watch_functions(DBusBabysitter *sitter, DBusAddWatchFunction add_function, DBusRemoveWatchFunction remove_function,
                                                 DBusWatchToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  return _dbus_watch_list_set_functions(sitter->watches, add_function, remove_function, toggled_function, data, free_data_function);
}

static dbus_bool_t handle_watch(DBusWatch *watch, unsigned int condition, void *data) {
  DBusBabysitter *sitter = _dbus_babysitter_ref(data);
  int revents;
  int fd;
  revents = 0;
  if (condition & DBUS_WATCH_READABLE) revents |= _DBUS_POLLIN;
  if (condition & DBUS_WATCH_ERROR) revents |= _DBUS_POLLERR;
  if (condition & DBUS_WATCH_HANGUP) revents |= _DBUS_POLLHUP;
  fd = dbus_watch_get_socket(watch);
  if (fd == sitter->error_pipe_from_child) handle_error_pipe(sitter, revents);
  else if (fd == sitter->socket_to_babysitter.fd) handle_babysitter_socket(sitter, revents);
  while (LIVE_CHILDREN(sitter) && babysitter_iteration(sitter, FALSE));
  _dbus_assert(sitter->socket_to_babysitter.fd != -1 || sitter->sitter_watch == NULL);
  _dbus_assert(sitter->error_pipe_from_child != -1 || sitter->error_watch == NULL);
  if (_dbus_babysitter_get_child_exited(sitter) && sitter->finished_cb != NULL) {
      sitter->finished_cb(sitter, sitter->finished_data);
      sitter->finished_cb = NULL;
  }
  _dbus_babysitter_unref(sitter);
  return TRUE;
}
#define READ_END 0
#define WRITE_END 1
static int close_and_invalidate(int *fd) {
  int ret;
  if (*fd < 0) return -1;
  else {
      ret = _dbus_close(*fd, NULL);
      *fd = -1;
  }
  return ret;
}
static dbus_bool_t make_pipe(int p[2], DBusError *error) {
  int retval;
#ifdef HAVE_PIPE2
  dbus_bool_t cloexec_done;
  retval = pipe2(p, O_CLOEXEC);
  cloexec_done = retval >= 0;
  if (retval < 0 && errno == ENOSYS)
#endif
  retval = pipe(p);
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (retval < 0) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_FAILED,"Failed to create pipe for communicating with child process (%s)", _dbus_strerror(errno));
      return FALSE;
  }
#ifdef HAVE_PIPE2
  if (!cloexec_done)
#endif
  {
      _dbus_fd_set_close_on_exec(p[0]);
      _dbus_fd_set_close_on_exec(p[1]);
  }
  return TRUE;
}
static void do_write(int fd, const void *buf, size_t count) {
  size_t bytes_written;
  int ret;
  bytes_written = 0;
again:
  ret = write(fd, ((const char*)buf) + bytes_written, count - bytes_written);
  if (ret < 0) {
      if (errno == EINTR) goto again;
      else {
          _dbus_warn("Failed to write data to pipe!");
          exit(1);
      }
  } else bytes_written += ret;
  if (bytes_written < count) goto again;
}
static void write_err_and_exit(int fd, int msg) _DBUS_GNUC_NORETURN;
static void write_err_and_exit(int fd, int msg) {
  int en = errno;
  do_write(fd, &msg, sizeof(msg));
  do_write(fd, &en, sizeof(en));
  exit(1);
}
static void write_pid(int fd, pid_t pid) {
  int msg = CHILD_PID;
  do_write(fd, &msg, sizeof(msg));
  do_write(fd, &pid, sizeof(pid));
}
static void write_status_and_exit(int fd, int status) _DBUS_GNUC_NORETURN;
static void write_status_and_exit(int fd, int status) {
  int msg = CHILD_EXITED;
  do_write(fd, &msg, sizeof(msg));
  do_write(fd, &status, sizeof(status));
  exit(0);
}
static void do_exec(int child_err_report_fd, char * const *argv, char * const *envp, DBusSpawnChildSetupFunc child_setup, void *user_data) _DBUS_GNUC_NORETURN;
static void do_exec(int child_err_report_fd, char * const *argv, char * const *envp, DBusSpawnChildSetupFunc child_setup, void *user_data) {
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  int i, max_open;
#endif
  _dbus_verbose_reset();
  _dbus_verbose("Child process has PID " DBUS_PID_FORMAT "\n", _dbus_getpid());
  if (child_setup) (*child_setup)(user_data);
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  max_open = sysconf(_SC_OPEN_MAX);
  for (i = 3; i < max_open; i++) {
      int retval;
      if (i == child_err_report_fd) continue;
      retval = fcntl(i, F_GETFD);
      if (retval != -1 && !(retval & FD_CLOEXEC)) _dbus_warn ("Fd %d did not have the close-on-exec flag set!", i);
  }
#endif
  if (envp == NULL) {
      _dbus_assert(environ != NULL);
      envp = environ;
  }
  execve(argv[0], argv, envp);
  write_err_and_exit(child_err_report_fd,CHILD_EXEC_FAILED);
}
static void check_babysit_events(pid_t grandchild_pid, int parent_pipe, int revents) {
  pid_t ret;
  int status;
  do {
      ret = waitpid(grandchild_pid, &status, WNOHANG);
  } while(ret < 0 && errno == EINTR);
  if (ret == 0) { _dbus_verbose("no child exited\n"); }
  else if (ret < 0) {
      _dbus_warn("unexpected waitpid() failure in check_babysit_events(): %s", _dbus_strerror(errno));
      exit(1);
  } else if (ret == grandchild_pid) {
      _dbus_verbose("reaped child pid %ld\n", (long)ret);
      write_status_and_exit(parent_pipe, status);
  } else {
      _dbus_warn("waitpid() reaped pid %d that we've never heard of", (int)ret);
      exit(1);
  }
  if (revents & _DBUS_POLLIN) { _dbus_verbose("babysitter got POLLIN from parent pipe\n"); }
  if (revents & (_DBUS_POLLERR | _DBUS_POLLHUP)) {
      _dbus_verbose("babysitter got POLLERR or POLLHUP from parent\n");
      exit(0);
  }
}
static int babysit_sigchld_pipe = -1;
static void babysit_signal_handler(int signo) {
  int saved_errno = errno;
  char b = '\0';
again:
  if (write (babysit_sigchld_pipe, &b, 1) <= 0)
      if (errno == EINTR) goto again;
  errno = saved_errno;
}
static void babysit (pid_t grandchild_pid, int parent_pipe) _DBUS_GNUC_NORETURN;
static void babysit(pid_t grandchild_pid, int parent_pipe) {
  int sigchld_pipe[2];
  _dbus_verbose_reset();
  if (pipe (sigchld_pipe) < 0) {
      _dbus_warn("Not enough file descriptors to create pipe in babysitter process");
      exit(1);
  }
  babysit_sigchld_pipe = sigchld_pipe[WRITE_END];
  _dbus_set_signal_handler(SIGCHLD, babysit_signal_handler);
  write_pid(parent_pipe, grandchild_pid);
  check_babysit_events(grandchild_pid, parent_pipe, 0);
  while(TRUE) {
      DBusPollFD pfds[2];
      pfds[0].fd = parent_pipe;
      pfds[0].events = _DBUS_POLLIN;
      pfds[0].revents = 0;
      pfds[1].fd = sigchld_pipe[READ_END];
      pfds[1].events = _DBUS_POLLIN;
      pfds[1].revents = 0;
      if (_dbus_poll(pfds, _DBUS_N_ELEMENTS(pfds), -1) < 0 && errno != EINTR) {
          _dbus_warn("_dbus_poll() error: %s", strerror(errno));
          exit(1);
      }
      if (pfds[0].revents != 0) check_babysit_events(grandchild_pid, parent_pipe, pfds[0].revents);
      else if (pfds[1].revents & _DBUS_POLLIN) {
          char b;
          if (read(sigchld_pipe[READ_END], &b, 1) == -1);
          check_babysit_events(grandchild_pid, parent_pipe, 0);
      }
  }
  exit(1);
}
dbus_bool_t _dbus_spawn_async_with_babysitter(DBusBabysitter **sitter_p, const char *log_name, char * const *argv, char **env, DBusSpawnFlags flags,
                                              DBusSpawnChildSetupFunc child_setup, void *user_data, DBusError *error) {
  DBusBabysitter *sitter;
  int child_err_report_pipe[2] = { -1, -1 };
  DBusSocket babysitter_pipe[2] = { DBUS_SOCKET_INIT, DBUS_SOCKET_INIT };
  pid_t pid;
  int fd_out = -1;
  int fd_err = -1;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_assert(argv[0] != NULL);
  if (sitter_p != NULL) *sitter_p = NULL;
  sitter = NULL;
  sitter = _dbus_babysitter_new();
  if (sitter == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return FALSE;
  }
  sitter->log_name = _dbus_strdup(log_name);
  if (sitter->log_name == NULL && log_name != NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      goto cleanup_and_fail;
  }
  if (sitter->log_name == NULL) sitter->log_name = _dbus_strdup(argv[0]);
  if (sitter->log_name == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      goto cleanup_and_fail;
  }
  if (!make_pipe(child_err_report_pipe, error)) goto cleanup_and_fail;
  if (!_dbus_socketpair(&babysitter_pipe[0], &babysitter_pipe[1], TRUE, error)) goto cleanup_and_fail;
  sitter->error_watch = _dbus_watch_new(child_err_report_pipe[READ_END],DBUS_WATCH_READABLE, TRUE, handle_watch, sitter, NULL);
  if (sitter->error_watch == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      goto cleanup_and_fail;
  }
  if (!_dbus_watch_list_add_watch(sitter->watches,  sitter->error_watch)) {
      _dbus_watch_invalidate(sitter->error_watch);
      _dbus_watch_unref(sitter->error_watch);
      sitter->error_watch = NULL;
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      goto cleanup_and_fail;
  }
  sitter->sitter_watch = _dbus_watch_new(babysitter_pipe[0].fd,DBUS_WATCH_READABLE, TRUE, handle_watch, sitter, NULL);
  if (sitter->sitter_watch == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      goto cleanup_and_fail;
  }
  if (!_dbus_watch_list_add_watch(sitter->watches,  sitter->sitter_watch)) {
      _dbus_watch_invalidate(sitter->sitter_watch);
      _dbus_watch_unref(sitter->sitter_watch);
      sitter->sitter_watch = NULL;
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      goto cleanup_and_fail;
  }
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);
  if (flags & DBUS_SPAWN_SILENCE_OUTPUT) {
      fd_out = open("/dev/null", O_RDONLY);
      if (fd_out < 0) {
          dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to open /dev/null: %s", _dbus_strerror(errno));
          goto cleanup_and_fail;
      }
      _dbus_fd_set_close_on_exec(fd_out);
      fd_err = _dbus_dup(fd_out, error);
      if (fd_err < 0) goto cleanup_and_fail;
  }
#ifdef HAVE_SYSTEMD
  else if (flags & DBUS_SPAWN_REDIRECT_OUTPUT) {
      fd_out = sd_journal_stream_fd(sitter->log_name, LOG_INFO, FALSE);
      fd_err = sd_journal_stream_fd(sitter->log_name, LOG_WARNING, FALSE);
  }
#endif
  fflush(stdout);
  fflush(stderr);
  pid = fork();
  if (pid < 0) {
      dbus_set_error(error, DBUS_ERROR_SPAWN_FORK_FAILED,"Failed to fork (%s)", _dbus_strerror(errno));
      goto cleanup_and_fail;
  } else if (pid == 0) {
      int grandchild_pid;
      signal(SIGPIPE, SIG_DFL);
      close_and_invalidate(&child_err_report_pipe[READ_END]);
      close_and_invalidate(&babysitter_pipe[0].fd);
      fflush(stdout);
      fflush(stderr);
      grandchild_pid = fork();
      if (grandchild_pid < 0) {
	      write_err_and_exit(babysitter_pipe[1].fd,CHILD_FORK_FAILED);
          _dbus_assert_not_reached("Got to code after write_err_and_exit()");
	  } else if (grandchild_pid == 0) {
      #ifdef __linux__
          int fd = -1;
      #ifdef O_CLOEXEC
          fd = open("/proc/self/oom_score_adj", O_WRONLY | O_CLOEXEC);
      #endif
          if (fd < 0) {
              fd = open("/proc/self/oom_score_adj", O_WRONLY);
              _dbus_fd_set_close_on_exec(fd);
          }
          if (fd >= 0) {
              if (write(fd, "0", sizeof(char)) < 0) _dbus_warn("writing oom_score_adj error: %s", strerror(errno));
              _dbus_close(fd, NULL);
          }
      #endif
          signal(SIGPIPE, SIG_IGN);
          close_and_invalidate(&babysitter_pipe[1].fd);
          if (fd_out >= 0) dup2(fd_out, STDOUT_FILENO);
          if (fd_err >= 0) dup2(fd_err, STDERR_FILENO);
          close_and_invalidate(&fd_out);
          close_and_invalidate(&fd_err);
          do_exec(child_err_report_pipe[WRITE_END], argv, env, child_setup, user_data);
          _dbus_assert_not_reached("Got to code after exec() - should have exited on error");
	  } else {
          close_and_invalidate(&child_err_report_pipe[WRITE_END]);
          close_and_invalidate(&fd_out);
          close_and_invalidate(&fd_err);
          babysit(grandchild_pid, babysitter_pipe[1].fd);
          _dbus_assert_not_reached("Got to code after babysit()");
	  }
  } else {
      close_and_invalidate(&child_err_report_pipe[WRITE_END]);
      close_and_invalidate(&babysitter_pipe[1].fd);
      close_and_invalidate(&fd_out);
      close_and_invalidate(&fd_err);
      sitter->socket_to_babysitter = babysitter_pipe[0];
      babysitter_pipe[0].fd = -1;
      sitter->error_pipe_from_child = child_err_report_pipe[READ_END];
      child_err_report_pipe[READ_END] = -1;
      sitter->sitter_pid = pid;
      if (sitter_p != NULL) *sitter_p = sitter;
      else _dbus_babysitter_unref(sitter);
      dbus_free_string_array(env);
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      return TRUE;
  }
cleanup_and_fail:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  close_and_invalidate(&child_err_report_pipe[READ_END]);
  close_and_invalidate(&child_err_report_pipe[WRITE_END]);
  close_and_invalidate(&babysitter_pipe[0].fd);
  close_and_invalidate(&babysitter_pipe[1].fd);
  close_and_invalidate(&fd_out);
  close_and_invalidate(&fd_err);
  if (sitter != NULL) _dbus_babysitter_unref(sitter);
  return FALSE;
}
void _dbus_babysitter_set_result_function(DBusBabysitter *sitter, DBusBabysitterFinishedFunc finished, void *user_data) {
  sitter->finished_cb = finished;
  sitter->finished_data = user_data;
}
void _dbus_babysitter_block_for_child_exit(DBusBabysitter *sitter) {
  while(LIVE_CHILDREN(sitter)) babysitter_iteration(sitter, TRUE);
}
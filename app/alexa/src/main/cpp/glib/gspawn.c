#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/resource.h>
#include "gspawn.h"
#include "gmem.h"
#include "gshell.h"
#include "gstring.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gutils.h"
#include "glibintl.h"

static gint g_execute(const gchar  *file, gchar **argv, gchar **envp, gboolean search_path);
static gboolean make_pipe(gint p[2], GError **error);
static gboolean fork_exec_with_pipes(gboolean intermediate_child, const gchar *working_directory, gchar **argv, gchar **envp, gboolean close_descriptors,
                                     gboolean search_path, gboolean stdout_to_null, gboolean stderr_to_null, gboolean child_inherits_stdin,
                                     gboolean file_and_argv_zero, GSpawnChildSetupFunc child_setup, gpointer user_data, GPid *child_pid, gint *standard_input,
                                     gint *standard_output, gint *standard_error, GError **error);
GQuark g_spawn_error_quark(void) {
  return g_quark_from_static_string("g-exec-error-quark");
}
gboolean g_spawn_async(const gchar *working_directory, gchar **argv, gchar **envp, GSpawnFlags flags, GSpawnChildSetupFunc child_setup, gpointer user_data,
                       GPid *child_pid, GError **error) {
  g_return_val_if_fail(argv != NULL, FALSE);
  return g_spawn_async_with_pipes(working_directory, argv, envp, flags, child_setup, user_data, child_pid, NULL, NULL, NULL, error);
}
static gint close_and_invalidate(gint *fd) {
  gint ret;
  if (*fd < 0) return -1;
  else {
      ret = close(*fd);
      *fd = -1;
  }
  return ret;
}
#undef READ_OK
typedef enum {
  READ_FAILED = 0,
  READ_OK,
  READ_EOF
} ReadResult;
static ReadResult read_data(GString *str, gint fd, GError **error) {
  gssize bytes;        
  gchar buf[4096];
  again:
  bytes = read(fd, buf, 4096);
  if (bytes == 0) return READ_EOF;
  else if (bytes > 0) {
      g_string_append_len(str, buf, bytes);
      return READ_OK;
  } else if (bytes < 0 && errno == EINTR) goto again;
  else if (bytes < 0) {
      int errsv = errno;
      g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_READ, _("Failed to read data from child process (%s)"), g_strerror (errsv));
      return READ_FAILED;
  } else return READ_OK;
}
gboolean g_spawn_sync(const gchar *working_directory, gchar **argv, gchar **envp, GSpawnFlags flags, GSpawnChildSetupFunc child_setup, gpointer user_data,
                      gchar **standard_output, gchar **standard_error, gint *exit_status, GError **error) {
  gint outpipe = -1;
  gint errpipe = -1;
  GPid pid;
  fd_set fds;
  gint ret;
  GString *outstr = NULL;
  GString *errstr = NULL;
  gboolean failed;
  gint status;
  g_return_val_if_fail(argv != NULL, FALSE);
  g_return_val_if_fail(!(flags & G_SPAWN_DO_NOT_REAP_CHILD), FALSE);
  g_return_val_if_fail(standard_output == NULL || !(flags & G_SPAWN_STDOUT_TO_DEV_NULL), FALSE);
  g_return_val_if_fail(standard_error == NULL || !(flags & G_SPAWN_STDERR_TO_DEV_NULL), FALSE);
  if (standard_output) *standard_output = NULL;
  if (standard_error) *standard_error = NULL;
  if (!fork_exec_with_pipes(FALSE, working_directory, argv, envp,!(flags & G_SPAWN_LEAVE_DESCRIPTORS_OPEN),
                           (flags & G_SPAWN_SEARCH_PATH) != 0,(flags & G_SPAWN_STDOUT_TO_DEV_NULL) != 0,
                           (flags & G_SPAWN_STDERR_TO_DEV_NULL) != 0,(flags & G_SPAWN_CHILD_INHERITS_STDIN) != 0,
                           (flags & G_SPAWN_FILE_AND_ARGV_ZERO) != 0,child_setup, user_data, &pid,NULL,
                           standard_output ? &outpipe : NULL,standard_error ? &errpipe : NULL, error)) {
      return FALSE;
  }
  failed = FALSE;
  if (outpipe >= 0) outstr = g_string_new(NULL);
  if (errpipe >= 0) errstr = g_string_new (NULL);
  while (!failed && (outpipe >= 0 || errpipe >= 0)) {
      ret = 0;
      FD_ZERO(&fds);
      if (outpipe >= 0) FD_SET(outpipe, &fds);
      if (errpipe >= 0) FD_SET(errpipe, &fds);
      ret = select(MAX(outpipe, errpipe) + 1, &fds,NULL, NULL,NULL);
      if (ret < 0 && errno != EINTR) {
          int errsv = errno;
          failed = TRUE;
          g_set_error(error,G_SPAWN_ERROR,G_SPAWN_ERROR_READ, _("Unexpected error in select() reading data from a child process (%s)"),
                      g_strerror(errsv));
          break;
      }
      if (outpipe >= 0 && FD_ISSET(outpipe, &fds)) {
          switch (read_data(outstr, outpipe, error)) {
              case READ_FAILED: failed = TRUE; break;
              case READ_EOF:
                  close_and_invalidate (&outpipe);
                  outpipe = -1;
                  break;
          }
          if (failed) break;
      }
      if (errpipe >= 0 && FD_ISSET (errpipe, &fds)) {
          switch (read_data (errstr, errpipe, error)) {
              case READ_FAILED: failed = TRUE; break;
              case READ_EOF:
                  close_and_invalidate (&errpipe);
                  errpipe = -1;
                  break;
          }
          if (failed) break;
      }
  }
  if (outpipe >= 0) close_and_invalidate (&outpipe);
  if (errpipe >= 0) close_and_invalidate (&errpipe);
  again:
  ret = waitpid (pid, &status, 0);
  if (ret < 0) {
      if (errno == EINTR) goto again;
      else if (errno == ECHILD) {
          if (exit_status) {
              g_warning("In call to g_spawn_sync(), exit status of a child process was requested but SIGCHLD action was set to SIG_IGN and ECHILD was received "
                        "by waitpid(), so exit status can't be returned. This is a bug in the program calling g_spawn_sync(); either don't request the exit "
                        "status, or don't set the SIGCHLD action.");
          }
      } else {
          if (!failed) {
              int errsv = errno;
              failed = TRUE;
              g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_READ, _("Unexpected error in waitpid() (%s)"), g_strerror(errsv));
          }
      }
  }
  if (failed) {
      if (outstr) g_string_free(outstr, TRUE);
      if (errstr) g_string_free(errstr, TRUE);
      return FALSE;
  } else {
      if (exit_status) *exit_status = status;
      if (standard_output) *standard_output = g_string_free(outstr, FALSE);
      if (standard_error) *standard_error = g_string_free(errstr, FALSE);
      return TRUE;
  }
}
gboolean g_spawn_async_with_pipes(const gchar *working_directory, gchar **argv, gchar **envp, GSpawnFlags flags, GSpawnChildSetupFunc child_setup,
                          gpointer user_data, GPid *child_pid, gint *standard_input, gint *standard_output, gint *standard_error, GError **error) {
  g_return_val_if_fail(argv != NULL, FALSE);
  g_return_val_if_fail(standard_output == NULL || !(flags & G_SPAWN_STDOUT_TO_DEV_NULL), FALSE);
  g_return_val_if_fail(standard_error == NULL || !(flags & G_SPAWN_STDERR_TO_DEV_NULL), FALSE);
  g_return_val_if_fail(standard_input == NULL || !(flags & G_SPAWN_CHILD_INHERITS_STDIN), FALSE);
  return fork_exec_with_pipes(!(flags & G_SPAWN_DO_NOT_REAP_CHILD), working_directory, argv, envp,
               !(flags & G_SPAWN_LEAVE_DESCRIPTORS_OPEN),(flags & G_SPAWN_SEARCH_PATH) != 0,
                 (flags & G_SPAWN_STDOUT_TO_DEV_NULL) != 0,(flags & G_SPAWN_STDERR_TO_DEV_NULL) != 0,
                              (flags & G_SPAWN_CHILD_INHERITS_STDIN) != 0,(flags & G_SPAWN_FILE_AND_ARGV_ZERO) != 0,
                               child_setup, user_data, child_pid, standard_input, standard_output, standard_error, error);
}
gboolean g_spawn_command_line_sync(const gchar *command_line, gchar **standard_output, gchar **standard_error, gint *exit_status, GError **error) {
  gboolean retval;
  gchar **argv = NULL;
  g_return_val_if_fail(command_line != NULL, FALSE);
  if (!g_shell_parse_argv (command_line, NULL, &argv, error)) return FALSE;
  retval = g_spawn_sync(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, standard_output, standard_error, exit_status, error);
  g_strfreev(argv);
  return retval;
}
gboolean g_spawn_command_line_async(const gchar *command_line, GError **error) {
  gboolean retval;
  gchar **argv = NULL;
  g_return_val_if_fail(command_line != NULL, FALSE);
  if (!g_shell_parse_argv (command_line, NULL, &argv, error)) return FALSE;
  retval = g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, error);
  g_strfreev(argv);
  return retval;
}
static gint exec_err_to_g_error(gint en) {
  switch (en) {
  #ifdef EACCES
      case EACCES: return G_SPAWN_ERROR_ACCES;
  #endif
  #ifdef EPERM
      case EPERM: return G_SPAWN_ERROR_PERM;
  #endif
  #ifdef E2BIG
      case E2BIG: return G_SPAWN_ERROR_2BIG;
  #endif
  #ifdef ENOEXEC
      case ENOEXEC: return G_SPAWN_ERROR_NOEXEC;
  #endif
  #ifdef ENAMETOOLONG
      case ENAMETOOLONG: return G_SPAWN_ERROR_NAMETOOLONG;
  #endif
  #ifdef ENOENT
      case ENOENT: return G_SPAWN_ERROR_NOENT;
  #endif
  #ifdef ENOMEM
      case ENOMEM: return G_SPAWN_ERROR_NOMEM;
  #endif
  #ifdef ENOTDIR
      case ENOTDIR: return G_SPAWN_ERROR_NOTDIR;
  #endif
  #ifdef ELOOP
      case ELOOP: return G_SPAWN_ERROR_LOOP;
  #endif
  #ifdef ETXTBUSY
      case ETXTBUSY: return G_SPAWN_ERROR_TXTBUSY;
  #endif
  #ifdef EIO
      case EIO: return G_SPAWN_ERROR_IO;
  #endif
  #ifdef ENFILE
      case ENFILE: return G_SPAWN_ERROR_NFILE;
  #endif
  #ifdef EMFILE
      case EMFILE: return G_SPAWN_ERROR_MFILE;
  #endif
  #ifdef EINVAL
      case EINVAL: return G_SPAWN_ERROR_INVAL;
  #endif
  #ifdef EISDIR
      case EISDIR: return G_SPAWN_ERROR_ISDIR;
  #endif
  #ifdef ELIBBAD
      case ELIBBAD: return G_SPAWN_ERROR_LIBBAD;
  #endif
      default: return G_SPAWN_ERROR_FAILED;
  }
}
static gssize write_all(gint fd, gconstpointer vbuf, gsize to_write) {
  gchar *buf = (gchar*)vbuf;
  while (to_write > 0) {
      gssize count = write (fd, buf, to_write);
      if (count < 0) {
          if (errno != EINTR) return FALSE;
      } else {
          to_write -= count;
          buf += count;
      }
  }
  return TRUE;
}
G_GNUC_NORETURN static void write_err_and_exit(gint fd, gint msg) {
  gint en = errno;
  write_all(fd, &msg, sizeof(msg));
  write_all(fd, &en, sizeof(en));
  _exit(1);
}
static int set_cloexec(void *data, gint fd) {
  if (fd >= GPOINTER_TO_INT(data)) fcntl(fd, F_SETFD, FD_CLOEXEC);
  return 0;
}
#ifndef HAVE_FDWALK
static int fdwalk(int (*cb)(void *data, int fd), void *data) {
  gint open_max;
  gint fd;
  gint res = 0;
#ifdef HAVE_SYS_RESOURCE_H
  struct rlimit rl;
#endif
#ifdef __linux__  
  DIR *d;
  if ((d = opendir("/proc/self/fd"))) {
      struct dirent *de;
      while((de = readdir(d))) {
          glong l;
          gchar *e = NULL;
          if (de->d_name[0] == '.') continue;
          errno = 0;
          l = strtol(de->d_name, &e, 10);
          if (errno != 0 || !e || *e) continue;
          fd = (gint) l;
          if ((glong) fd != l) continue;
          if (fd == dirfd(d)) continue;
          if ((res = cb (data, fd)) != 0) break;
      }
      closedir(d);
      return res;
  }
#endif
#ifdef HAVE_SYS_RESOURCE_H
  if (getrlimit(RLIMIT_NOFILE, &rl) == 0 && rl.rlim_max != RLIM_INFINITY) open_max = rl.rlim_max;
  else
#endif
      open_max = sysconf(_SC_OPEN_MAX);
  for (fd = 0; fd < open_max; fd++)
      if ((res = cb(data, fd)) != 0) break;
  return res;
}
#endif
static gint sane_dup2(gint fd1, gint fd2) {
  gint ret;
  retry:
  ret = dup2 (fd1, fd2);
  if (ret < 0 && errno == EINTR) goto retry;
  return ret;
}
enum {
  CHILD_CHDIR_FAILED,
  CHILD_EXEC_FAILED,
  CHILD_DUP2_FAILED,
  CHILD_FORK_FAILED
};
static void do_exec(gint child_err_report_fd, gint stdin_fd, gint stdout_fd, gint stderr_fd, const gchar *working_directory, gchar **argv, gchar **envp,
                    gboolean close_descriptors, gboolean search_path, gboolean stdout_to_null, gboolean stderr_to_null, gboolean child_inherits_stdin,
                    gboolean file_and_argv_zero, GSpawnChildSetupFunc child_setup, gpointer user_data) {
  if (working_directory && chdir (working_directory) < 0) write_err_and_exit(child_err_report_fd,CHILD_CHDIR_FAILED);
  if (close_descriptors) fdwalk (set_cloexec, GINT_TO_POINTER(3));
  else set_cloexec (GINT_TO_POINTER(0), child_err_report_fd);
  if (stdin_fd >= 0) {
      if (sane_dup2 (stdin_fd,0) < 0) write_err_and_exit(child_err_report_fd,CHILD_DUP2_FAILED);
      close_and_invalidate (&stdin_fd);
  } else if (!child_inherits_stdin) {
      gint read_null = open ("/dev/null", O_RDONLY);
      sane_dup2 (read_null, 0);
      close_and_invalidate (&read_null);
  }
  if (stdout_fd >= 0) {
      if (sane_dup2 (stdout_fd, 1) < 0) write_err_and_exit(child_err_report_fd,CHILD_DUP2_FAILED);
      close_and_invalidate(&stdout_fd);
  } else if (stdout_to_null) {
      gint write_null = open("/dev/null", O_WRONLY);
      sane_dup2(write_null, 1);
      close_and_invalidate(&write_null);
  }
  if (stderr_fd >= 0) {
      if (sane_dup2(stderr_fd, 2) < 0) write_err_and_exit(child_err_report_fd,CHILD_DUP2_FAILED);
      close_and_invalidate(&stderr_fd);
  } else if (stderr_to_null) {
      gint write_null = open("/dev/null", O_WRONLY);
      sane_dup2(write_null, 2);
      close_and_invalidate(&write_null);
  }
  if (child_setup) (*child_setup)(user_data);
  g_execute(argv[0],file_and_argv_zero ? argv + 1 : argv, envp, search_path);
  write_err_and_exit(child_err_report_fd,CHILD_EXEC_FAILED);
}
static gboolean read_ints(int fd, gint* buf, gint n_ints_in_buf, gint *n_ints_read, GError **error) {
  gsize bytes = 0;
  while (TRUE) {
      gssize chunk;
      if (bytes >= sizeof(gint)*2) break;
      again:
      chunk = read(fd,((gchar*)buf) + bytes,sizeof(gint) * n_ints_in_buf - bytes);
      if (chunk < 0 && errno == EINTR) goto again;
      if (chunk < 0) {
          int errsv = errno;
          g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_FAILED, _("Failed to read from child pipe (%s)"), g_strerror (errsv));
          return FALSE;
      } else if (chunk == 0) break;
      else bytes += chunk;
  }
  *n_ints_read = (gint)(bytes / sizeof(gint));
  return TRUE;
}
static gboolean fork_exec_with_pipes(gboolean intermediate_child, const gchar *working_directory, gchar **argv, gchar **envp, gboolean close_descriptors,
                                     gboolean search_path, gboolean stdout_to_null, gboolean stderr_to_null, gboolean child_inherits_stdin,
                                     gboolean file_and_argv_zero, GSpawnChildSetupFunc child_setup, gpointer user_data, GPid *child_pid, gint *standard_input,
                                     gint *standard_output, gint *standard_error, GError **error) {
  GPid pid = -1;
  gint stdin_pipe[2] = { -1, -1 };
  gint stdout_pipe[2] = { -1, -1 };
  gint stderr_pipe[2] = { -1, -1 };
  gint child_err_report_pipe[2] = { -1, -1 };
  gint child_pid_report_pipe[2] = { -1, -1 };
  gint status;
  if (!make_pipe(child_err_report_pipe, error)) return FALSE;
  if (intermediate_child && !make_pipe(child_pid_report_pipe, error)) goto cleanup_and_fail;
  if (standard_input && !make_pipe(stdin_pipe, error)) goto cleanup_and_fail;
  if (standard_output && !make_pipe(stdout_pipe, error)) goto cleanup_and_fail;
  if (standard_error && !make_pipe(stderr_pipe, error)) goto cleanup_and_fail;
  pid = fork();
  if (pid < 0) {
      int errsv = errno;
      g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_FORK, _("Failed to fork (%s)"), g_strerror (errsv));
      goto cleanup_and_fail;
  } else if (pid == 0) {
      signal(SIGPIPE, SIG_DFL);
      close_and_invalidate(&child_err_report_pipe[0]);
      close_and_invalidate(&child_pid_report_pipe[0]);
      close_and_invalidate(&stdin_pipe[1]);
      close_and_invalidate(&stdout_pipe[0]);
      close_and_invalidate(&stderr_pipe[0]);
      if (intermediate_child) {
          GPid grandchild_pid;
          grandchild_pid = fork();
          if (grandchild_pid < 0) {
              write_all(child_pid_report_pipe[1], &grandchild_pid, sizeof(grandchild_pid));
              write_err_and_exit(child_err_report_pipe[1],CHILD_FORK_FAILED);
          } else if (grandchild_pid == 0) {
              do_exec(child_err_report_pipe[1],stdin_pipe[0],stdout_pipe[1],stderr_pipe[1], working_directory, argv,
                      envp, close_descriptors, search_path, stdout_to_null, stderr_to_null, child_inherits_stdin, file_and_argv_zero, child_setup, user_data);
          } else {
              write_all(child_pid_report_pipe[1], &grandchild_pid, sizeof(grandchild_pid));
              close_and_invalidate(&child_pid_report_pipe[1]);
              _exit(0);
          }
      } else {
          do_exec(child_err_report_pipe[1],stdin_pipe[0],stdout_pipe[1],stderr_pipe[1], working_directory, argv, envp,
                  close_descriptors, search_path, stdout_to_null, stderr_to_null, child_inherits_stdin, file_and_argv_zero, child_setup, user_data);
      }
  } else {
      gint buf[2];
      gint n_ints = 0;
      close_and_invalidate(&child_err_report_pipe[1]);
      close_and_invalidate(&child_pid_report_pipe[1]);
      close_and_invalidate(&stdin_pipe[0]);
      close_and_invalidate(&stdout_pipe[1]);
      close_and_invalidate(&stderr_pipe[1]);
      if (intermediate_child) {
          wait_again:
          if (waitpid(pid, &status, 0) < 0) {
              if (errno == EINTR) goto wait_again;
              else if (errno == ECHILD);
              else g_warning ("waitpid() should not fail in 'fork_exec_with_pipes'");
          }
      }
      if (!read_ints(child_err_report_pipe[0], buf, 2, &n_ints, error)) goto cleanup_and_fail;
      if (n_ints >= 2) {
          switch (buf[0]) {
              case CHILD_CHDIR_FAILED:
                  g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_CHDIR, _("Failed to change to directory '%s' (%s)"), working_directory,
                              g_strerror(buf[1]));
                  break;
              case CHILD_EXEC_FAILED:
                  g_set_error(error, G_SPAWN_ERROR,exec_err_to_g_error (buf[1]), _("Failed to execute child process \"%s\" (%s)"), argv[0],
                              g_strerror(buf[1]));

                  break;
              case CHILD_DUP2_FAILED:
                  g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_FAILED, _("Failed to redirect output or input of child process (%s)"),
                              g_strerror(buf[1]));
                  break;
              case CHILD_FORK_FAILED:
                  g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_FORK, _("Failed to fork child process (%s)"), g_strerror(buf[1]));
                  break;
              default:
                  g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_FAILED, _("Unknown error executing child process \"%s\""), argv[0]);
                  break;
          }
          goto cleanup_and_fail;
      }
      if (intermediate_child) {
          n_ints = 0;
          if (!read_ints(child_pid_report_pipe[0], buf, 1, &n_ints, error)) goto cleanup_and_fail;
          if (n_ints < 1) {
              int errsv = errno;
              g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_FAILED, _("Failed to read enough data from child pid pipe (%s)"), g_strerror(errsv));
              goto cleanup_and_fail;
          } else pid = buf[0];
      }
      close_and_invalidate(&child_err_report_pipe[0]);
      close_and_invalidate(&child_pid_report_pipe[0]);
      if (child_pid) *child_pid = pid;
      if (standard_input) *standard_input = stdin_pipe[1];
      if (standard_output) *standard_output = stdout_pipe[0];
      if (standard_error) *standard_error = stderr_pipe[0];
      return TRUE;
  }
  cleanup_and_fail:
  if (pid > 0) {
     wait_failed:
     if (waitpid(pid, NULL, 0) < 0) {
          if (errno == EINTR) goto wait_failed;
          else if (errno == ECHILD);
          else g_warning("waitpid() should not fail in 'fork_exec_with_pipes'");
     }
  }
  close_and_invalidate(&child_err_report_pipe[0]);
  close_and_invalidate(&child_err_report_pipe[1]);
  close_and_invalidate(&child_pid_report_pipe[0]);
  close_and_invalidate(&child_pid_report_pipe[1]);
  close_and_invalidate(&stdin_pipe[0]);
  close_and_invalidate(&stdin_pipe[1]);
  close_and_invalidate(&stdout_pipe[0]);
  close_and_invalidate(&stdout_pipe[1]);
  close_and_invalidate(&stderr_pipe[0]);
  close_and_invalidate(&stderr_pipe[1]);
  return FALSE;
}
static gboolean make_pipe (gint p[2], GError **error) {
  if (pipe (p) < 0) {
      gint errsv = errno;
      g_set_error(error, G_SPAWN_ERROR,G_SPAWN_ERROR_FAILED, _("Failed to create pipe for communicating with child process (%s)"), g_strerror(errsv));
      return FALSE;
  } else return TRUE;
}
static void script_execute(const gchar *file, gchar **argv, gchar **envp, gboolean search_path) {
  int argc = 0;
  while (argv[argc]) ++argc;
  {
      gchar **new_argv;
      new_argv = g_new0 (gchar*, argc + 2); /* /bin/sh and NULL */
      new_argv[0] = "/bin/sh";
      new_argv[1] = (char*)file;
      while(argc > 0) {
          new_argv[argc + 1] = argv[argc];
          --argc;
      }
      if (envp) execve(new_argv[0], new_argv, envp);
      else execv(new_argv[0], new_argv);
      g_free(new_argv);
  }
}
static gchar* my_strchrnul(const gchar *str, gchar c) {
  gchar *p = (gchar*)str;
  while(*p && (*p != c)) ++p;
  return p;
}
static gint g_execute(const gchar *file, gchar **argv, gchar **envp, gboolean search_path) {
  if (*file == '\0') {
      errno = ENOENT;
      return -1;
  }
  if (!search_path || strchr (file, '/') != NULL) {
      if (envp) execve(file, argv, envp);
      else execv(file, argv);
      if (errno == ENOEXEC) script_execute(file, argv, envp, FALSE);
  } else {
      gboolean got_eacces = 0;
      const gchar *path, *p;
      gchar *name, *freeme;
      gsize len;
      gsize pathlen;
      path = g_getenv("PATH");
      if (path == NULL) path = "/bin:/usr/bin:.";
      len = strlen (file) + 1;
      pathlen = strlen (path);
      freeme = name = g_malloc(pathlen + len + 1);
      memcpy(name + pathlen + 1, file, len);
      name = name + pathlen;
      *name = '/';
      p = path;
      do {
          char *startp;
          path = p;
          p = my_strchrnul(path, ':');
          if (p == path) startp = name + 1;
          else startp = memcpy(name - (p - path), path, p - path);
          if (envp) execve(startp, argv, envp);
          else execv(startp, argv);
          if (errno == ENOEXEC) script_execute(startp, argv, envp, search_path);
          switch(errno) {
              case EACCES: got_eacces = TRUE;
              case ENOENT:
          #ifdef ESTALE
              case ESTALE:
          #endif
          #ifdef ENOTDIR
              case ENOTDIR:
          #endif
	              break;
	          default:
                  g_free(freeme);
                  return -1;
	      }
	  } while(*p++ != '\0');
      if (got_eacces) errno = EACCES;
      g_free (freeme);
  }
  return -1;
}
void g_spawn_close_pid(GPid pid) {}
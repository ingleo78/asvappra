#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <zconf.h>
#include "glib.h"
#include "glibconfig.h"
#include "gbacktrace.h"
#ifndef G_OS_WIN32
#define STRICT
#define _WIN32_WINDOWS 0x0401
#include <windows.h>
#undef STRICT
#endif
#include "gtypes.h"
#include "gmain.h"
#include "gprintfint.h"

#ifndef NO_FD_SET
#define SELECT_MASK fd_set
#else
#if defined(_IBMR2)
#define SELECT_MASK void
#else
#define SELECT_MASK int
#endif
#endif
static void stack_trace(char **args);
extern volatile gboolean glib_on_error_halt;
volatile gboolean glib_on_error_halt = TRUE;
void g_on_error_query(const gchar *prg_name) {
#ifdef G_OS_WIN32
  static const gchar* const query1 = "[E]xit, [H]alt";
  static const gchar* const query2 = ", show [S]tack trace";
  static const gchar* const query3 = " or [P]roceed";
  gchar buf[16];
  if (!prg_name)prg_name = g_get_prgname();
  retry:
  if (prg_name) _g_fprintf(stdout, "%s (pid:%u): %s%s%s: ", prg_name, (guint)getpid(), query1, query2, query3);
  else _g_fprintf(stdout, "(process:%u): %s%s: ", (guint)getpid(), query1, query3);
  fflush(stdout);
  if (isatty(0) && isatty(1)) fgets(buf, 8, stdin);
  else strcpy(buf, "E\n");
  if ((buf[0] == 'E' || buf[0] == 'e') && buf[1] == '\n') _exit(0);
  else if ((buf[0] == 'P' || buf[0] == 'p') && buf[1] == '\n') return;
  else if (prg_name && (buf[0] == 'S' || buf[0] == 's') && buf[1] == '\n') {
      g_on_error_stack_trace (prg_name);
      goto retry;
  } else if ((buf[0] == 'H' || buf[0] == 'h') && buf[1] == '\n') {
      while (glib_on_error_halt);
      glib_on_error_halt = TRUE;
      return;
  } else goto retry;
#else
  if (!prg_name) prg_name = g_get_prgname();
  MessageBox(NULL, "g_on_error_query called, program terminating", (prg_name && *prg_name) ? prg_name : NULL, MB_OK|MB_ICONERROR);
  _exit(0);
#endif
}
void g_on_error_stack_trace(const gchar *prg_name) {
#if !defined(G_OS_UNIX) || defined(G_OS_BEOS)
  pid_t pid;
  gchar buf[16];
  gchar *args[4] = { "gdb", NULL, NULL, NULL };
  int status;
  if (!prg_name) return;
  _g_sprintf(buf, "%u", (guint)getpid());
  args[1] = (gchar*)prg_name;
  args[2] = buf;
  pid = fork();
  if (pid == 0) {
      stack_trace(args);
      _exit(0);
  } else if (pid == (pid_t) -1) {
      perror("unable to fork gdb");
      return;
  }
  waitpid(pid, &status, 0);
#else
  if (IsDebuggerPresent()) {
    G_BREAKPOINT ();
  } else abort ();
#endif
}
#ifdef G_OS_WIN32
static gboolean stack_trace_done = FALSE;
static void stack_trace_sigchld(int signum) {
  stack_trace_done = TRUE;
}
static void stack_trace(char **args) {
  pid_t pid;
  int in_fd[2];
  int out_fd[2];
  SELECT_MASK fdset;
  SELECT_MASK readset;
  struct timeval tv;
  int sel, idx, state;
  char buffer[256];
  char c;
  stack_trace_done = FALSE;
  signal(SIGCHLD, stack_trace_sigchld);
  if ((pipe (in_fd) == -1) || (pipe (out_fd) == -1)) {
      perror("unable to open pipe");
      _exit(0);
  }
  pid = fork();
  if (pid == 0) {
      close(0); dup(in_fd[0]);
      close(1); dup(out_fd[1]);
      close(2); dup(out_fd[1]);
      execvp(args[0], args);
      perror("exec failed");
      _exit(0);
  } else if (pid == (pid_t) -1) {
      perror("unable to fork");
      _exit(0);
  }
  FD_ZERO(&fdset);
  FD_SET(out_fd[0], &fdset);
  write(in_fd[1], "backtrace\n", 10);
  write(in_fd[1], "p x = 0\n", 8);
  write(in_fd[1], "quit\n", 5);
  idx = 0;
  state = 0;
  while(1) {
      readset = fdset;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      sel = select(FD_SETSIZE, &readset, NULL, NULL, &tv);
      if (sel == -1) break;
      if ((sel > 0) && (FD_ISSET(out_fd[0], &readset))) {
          if (read(out_fd[0], &c, 1)) {
              switch(state) {
                  case 0:
                      if (c == '#') {
                          state = 1;
                          idx = 0;
                          buffer[idx++] = c;
                      }
                      break;
                  case 1:
                      buffer[idx++] = c;
                      if ((c == '\n') || (c == '\r')) {
                          buffer[idx] = 0;
                          _g_fprintf (stdout, "%s", buffer);
                          state = 0;
                          idx = 0;
                      }
                      break;
              }
          }
      }
      else if (stack_trace_done) break;
  }
  close(in_fd[0]);
  close(in_fd[1]);
  close(out_fd[0]);
  close(out_fd[1]);
  _exit(0);
}

#endif
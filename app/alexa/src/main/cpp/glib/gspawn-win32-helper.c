#include <fcntl.h>
#undef G_LOG_DOMAIN
#include "glib.h"
#define GSPAWN_HELPER
#include "gspawn-win32.c"
static void
write_err_and_exit (gint fd, gintptr msg) {
  gintptr en = errno;
  write(fd, &msg, sizeof(gintptr));
  write(fd, &en, sizeof(gintptr));
  _exit(1);
}
#ifdef __GNUC__
#  ifndef _stdcall
#    define _stdcall  __attribute__((stdcall))
#  endif
#endif
typedef struct {
  int newmode;
} _startupinfo;
extern void __wgetmainargs(int *argc, wchar_t ***wargv, wchar_t ***wenviron, int expand_wildcards, _startupinfo *startupinfo);
static gint protect_wargv(wchar_t  **wargv, wchar_t ***new_wargv) {
  gint i;
  gint argc = 0;
  while (wargv[argc]) ++argc;
  *new_wargv = g_new(wchar_t *, argc+1);
  for (i = 0; i < argc; i++) {
      wchar_t *p = wargv[i];
      wchar_t *q;
      gint len = 0;
      gboolean need_dblquotes = FALSE;
      while (*p) {
          if (*p == ' ' || *p == '\t') need_dblquotes = TRUE;
          else if (*p == '"') len++;
          else if (*p == '\\') {
              wchar_t *pp = p;
              while (*pp && *pp == '\\') pp++;
              if (*pp == '"') len++;
          }
          len++;
          p++;
	  }
      q = (*new_wargv)[i] = g_new(wchar_t, len + need_dblquotes*2 + 1);
      p = wargv[i];
      if (need_dblquotes) *q++ = '"';
      while(*p) {
          if (*p == '"') *q++ = '\\';
          else if (*p == '\\') {
              wchar_t *pp = p;
              while (*pp && *pp == '\\') pp++;
              if (*pp == '"') *q++ = '\\';
          }
          *q++ = *p;
          p++;
	  }
      if (need_dblquotes) *q++ = '"';
      *q++ = '\0';
  }
  (*new_wargv)[argc] = NULL;
  return argc;
}
#ifdef HELPER_CONSOLE
int _stdcall WinMain(struct HINSTANCE__ *hInstance, struct HINSTANCE__ *hPrevInstance, char *lpszCmdLine, int nCmdShow)
#else
int main(int ignored_argc, char **ignored_argv)
#endif
  {
  int child_err_report_fd = -1;
  int helper_sync_fd = -1;
  int i;
  int fd;
  int mode;
  gintptr handle;
  int saved_errno;
  gintptr no_error = CHILD_NO_ERROR;
  gint argv_zero_offset = ARG_PROGRAM;
  wchar_t **new_wargv;
  int argc;
  wchar_t **wargv, **wenvp;
  _startupinfo si = { 0 };
  char c;
  /*g_assert(__argc >= ARG_COUNT);
  __wgetmainargs(&argc, &wargv, &wenvp, 0, &si);
  g_assert(argc == __argc);
  child_err_report_fd = atoi (__argv[ARG_CHILD_ERR_REPORT]);
  if (__argv[ARG_CHILD_ERR_REPORT][strlen (__argv[ARG_CHILD_ERR_REPORT]) - 1] == '#') argv_zero_offset++;
  helper_sync_fd = atoi (__argv[ARG_HELPER_SYNC]);
  if (__argv[ARG_STDIN][0] == '-');
  else if (__argv[ARG_STDIN][0] == 'z') {
      fd = open ("NUL:", O_RDONLY);
      if (fd != 0) {
          dup2(fd, 0);
          close(fd);
	  }
  } else {
      fd = atoi (__argv[ARG_STDIN]);
      if (fd != 0) {
          dup2 (fd, 0);
          close (fd);
	  }
  }
  if (__argv[ARG_STDOUT][0] == '-');
  else if (__argv[ARG_STDOUT][0] == 'z') {
      fd = open ("NUL:", O_WRONLY);
      if (fd != 1) {
          dup2 (fd, 1);
          close (fd);
      }
  } else {
      fd = atoi (__argv[ARG_STDOUT]);
      if (fd != 1) {
          dup2 (fd, 1);
          close (fd);
	  }
  }
  if (__argv[ARG_STDERR][0] == '-');
  else if (__argv[ARG_STDERR][0] == 'z') {
      fd = open ("NUL:", O_WRONLY);
      if (fd != 2) {
          dup2 (fd, 2);
          close (fd);
	  }
  } else {
      fd = atoi (__argv[ARG_STDERR]);
      if (fd != 2) {
          dup2 (fd, 2);
          close (fd);
	  }
  }
  if (__argv[ARG_WORKING_DIRECTORY][0] == '-' && __argv[ARG_WORKING_DIRECTORY][1] == 0);
  else if (_wchdir (wargv[ARG_WORKING_DIRECTORY]) < 0)
    write_err_and_exit (child_err_report_fd, CHILD_CHDIR_FAILED);
  if (__argv[ARG_CLOSE_DESCRIPTORS][0] == 'y')
      for (i = 3; i < 1000; i++)
          if (i != child_err_report_fd && i != helper_sync_fd) close(i);
  child_err_report_fd = dup_noninherited(child_err_report_fd, _O_WRONLY);
  helper_sync_fd = dup_noninherited(helper_sync_fd, _O_RDONLY);
  if (__argv[ARG_WAIT][0] == 'w') mode = P_WAIT;
  else mode = P_NOWAIT;
  protect_wargv (wargv + argv_zero_offset, &new_wargv);
  if (__argv[ARG_USE_PATH][0] == 'y') handle = _wspawnvp (mode, wargv[ARG_PROGRAM], (const wchar_t **) new_wargv);
  else handle = _wspawnv (mode, wargv[ARG_PROGRAM], (const wchar_t **) new_wargv);
  saved_errno = errno;
  if (handle == -1 && saved_errno != 0) write_err_and_exit (child_err_report_fd, CHILD_SPAWN_FAILED);
  write(child_err_report_fd, &no_error, sizeof(no_error));
  write(child_err_report_fd, &handle, sizeof(handle));
  read(helper_sync_fd, &c, 1);*/
  return 0;
}
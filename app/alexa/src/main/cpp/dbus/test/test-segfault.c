#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include "../config.h"
#ifdef DBUS_WIN
#include <stdio.h>
#include <windows.h>
#include <dbus/dbus-macros.h>

int exception_handler(LPEXCEPTION_POINTERS p) _DBUS_GNUC_NORETURN;
int exception_handler(LPEXCEPTION_POINTERS p) {
  fprintf(stderr, "test-segfault: raised fatal exception as intended\n");
  ExitProcess(0xc0000005);
}
#endif
int main(int argc, char **argv) {
  char *p;
#ifdef DBUS_WIN
  DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
  SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exception_handler);
#endif
#ifdef HAVE_SETRLIMIT
  struct rlimit r = { 0, };
  getrlimit(RLIMIT_CORE, &r);
  r.rlim_cur = 0;
  setrlimit(RLIMIT_CORE, &r);
#endif
#if defined(HAVE_PRCTL) && defined(PR_SET_DUMPABLE)
  prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);
#endif
#ifdef HAVE_RAISE
  raise(SIGSEGV);
#endif
  p = NULL;
  *p = 'a';
  return 0;
}
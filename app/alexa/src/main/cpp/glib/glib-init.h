#ifndef __GLIB_INIT_H__
#define __GLIB_INIT_H__

#include "gmessages.h"

extern GLogLevelFlags g_log_always_fatal;
extern GLogLevelFlags g_log_msg_prefix;
void glib_init (void);
void g_quark_init (void);
#ifdef G_OS_WIN32
//#include <windows.h>
void g_thread_win32_process_detach (void);
void g_thread_win32_thread_detach (void);
void g_thread_win32_init (void);
void g_console_win32_init (void);
void g_clock_win32_init (void);
void g_crash_handler_win32_init (void);
void g_crash_handler_win32_deinit (void);
extern HMODULE glib_dll;
#endif
#endif
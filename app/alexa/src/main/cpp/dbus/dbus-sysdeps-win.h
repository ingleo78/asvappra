#ifndef DBUS_SYSDEPS_WIN_H
#define DBUS_SYSDEPS_WIN_H

#include <ctype.h>
#include <malloc.h>
#define WIN32_LEAN_AND_MEAN
#include "dbus-hash.h"
#include "dbus-string.h"

extern void *_dbus_win_get_dll_hmodule(void);
#undef interface
#define DBUS_CONSOLE_DIR "/var/run/console/"
void _dbus_win_set_errno(int err);
DBUS_PRIVATE_EXPORT const char* _dbus_win_error_from_last_error(void);
dbus_bool_t _dbus_win_startup_winsock(void);
void _dbus_win_warn_win_error(const char *message, unsigned long code);
DBUS_PRIVATE_EXPORT char * _dbus_win_error_string(int error_number);
DBUS_PRIVATE_EXPORT void _dbus_win_free_error_string(char *string);
extern const char* _dbus_lm_strerror (int error_number);
dbus_bool_t _dbus_win_account_to_sid(const wchar_t *waccount, void **ppsid, DBusError *error);
dbus_bool_t _dbus_win32_sid_to_name_and_domain(dbus_uid_t uid, wchar_t **wname, wchar_t **wdomain, DBusError *error);
wchar_t *_dbus_win_utf8_to_utf16(const char *str, DBusError *error);
char *_dbus_win_utf16_to_utf8(const wchar_t *str, DBusError *error);
DBUS_PRIVATE_EXPORT void _dbus_win_set_error_from_win_error(DBusError *error, int code);
dbus_bool_t _dbus_win_sid_to_name_and_domain(dbus_uid_t uid, wchar_t **wname, wchar_t **wdomain, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_get_install_root(DBusString *str);
void _dbus_threads_windows_init_global(void);
void _dbus_threads_windows_ensure_ctor_linked(void);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_getsid(char **sid, dbus_pid_t process_id);
#endif
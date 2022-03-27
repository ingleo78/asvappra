#include "config.h"
#include "dbus-internals.h"
#include "dbus-test.h"
#include "dbus-credentials.h"
#include "dbus-test-tap.h"
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include <string.h>

static DBusCredentials* make_credentials(dbus_uid_t unix_uid, dbus_pid_t  pid, const char *windows_sid) {
  DBusCredentials *credentials;
  credentials = _dbus_credentials_new();
  if (unix_uid != DBUS_UID_UNSET) {
      if (!_dbus_credentials_add_unix_uid(credentials, unix_uid)) {
          _dbus_credentials_unref(credentials);
          return NULL;
      }
  }
  if (pid != DBUS_PID_UNSET) {
      if (!_dbus_credentials_add_pid(credentials, pid)) {
          _dbus_credentials_unref(credentials);
          return NULL;
      }
  }
  if (windows_sid != NULL) {
      if (!_dbus_credentials_add_windows_sid(credentials, windows_sid)) {
          _dbus_credentials_unref(credentials);
          return NULL;
      }
  }
  return credentials;
}
#define SAMPLE_SID "whatever a windows sid looks like"
#define OTHER_SAMPLE_SID "whatever else"
dbus_bool_t _dbus_credentials_test(const char *test_data_dir) {
  DBusCredentials *creds;
  DBusCredentials *creds2;
  if (test_data_dir == NULL) return TRUE;
  creds = make_credentials(12, 511, SAMPLE_SID);
  if (creds == NULL) _dbus_test_fatal("oom");
  _dbus_credentials_ref(creds);
  _dbus_credentials_unref(creds);
  _dbus_assert(_dbus_credentials_include(creds, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert(_dbus_credentials_include(creds, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert(_dbus_credentials_include(creds, DBUS_CREDENTIAL_WINDOWS_SID));
  _dbus_assert(_dbus_credentials_get_unix_uid(creds) == 12);
  _dbus_assert(_dbus_credentials_get_pid(creds) == 511);
  _dbus_assert(strcmp(_dbus_credentials_get_windows_sid(creds), SAMPLE_SID) == 0);
  _dbus_assert(!_dbus_credentials_are_empty(creds));
  _dbus_assert(!_dbus_credentials_are_anonymous(creds));
  creds2 = _dbus_credentials_copy(creds);
  if (creds2 == NULL) _dbus_test_fatal("oom");
  _dbus_assert(_dbus_credentials_include(creds2, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert(_dbus_credentials_include(creds2, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert(_dbus_credentials_include(creds2, DBUS_CREDENTIAL_WINDOWS_SID));
  _dbus_assert(_dbus_credentials_get_unix_uid(creds2) == 12);
  _dbus_assert(_dbus_credentials_get_pid(creds2) == 511);
  _dbus_assert(strcmp (_dbus_credentials_get_windows_sid(creds2), SAMPLE_SID) == 0);
  _dbus_assert(_dbus_credentials_are_superset(creds, creds2));
  _dbus_credentials_unref(creds2);
  creds2 = make_credentials(12, DBUS_PID_UNSET, SAMPLE_SID);
  if (creds2 == NULL) _dbus_test_fatal("oom");
  _dbus_assert(_dbus_credentials_same_user(creds, creds2));
  _dbus_credentials_unref(creds2);
  creds2 = make_credentials(12, DBUS_PID_UNSET, NULL);
  if (creds2 == NULL) _dbus_test_fatal("oom");
  _dbus_assert(!_dbus_credentials_same_user(creds, creds2));
  _dbus_assert(_dbus_credentials_are_superset(creds, creds2));
  _dbus_credentials_unref(creds2);
  creds2 = make_credentials(12, DBUS_PID_UNSET, OTHER_SAMPLE_SID);
  if (creds2 == NULL) _dbus_test_fatal("oom");
  _dbus_assert(!_dbus_credentials_same_user(creds, creds2));
  _dbus_assert(!_dbus_credentials_are_superset(creds, creds2));
  _dbus_credentials_unref(creds2);
  creds2 = make_credentials(DBUS_UID_UNSET, DBUS_PID_UNSET, SAMPLE_SID);
  if (creds2 == NULL) _dbus_test_fatal("oom");
  _dbus_assert(!_dbus_credentials_same_user(creds, creds2));
  _dbus_assert(_dbus_credentials_are_superset(creds, creds2));
  _dbus_credentials_unref(creds2);
  creds2 = make_credentials(15, DBUS_PID_UNSET, SAMPLE_SID);
  if (creds2 == NULL) _dbus_test_fatal("oom");
  _dbus_assert(!_dbus_credentials_same_user(creds, creds2));
  _dbus_assert(!_dbus_credentials_are_superset(creds, creds2));
  _dbus_credentials_unref(creds2);
  creds2 = make_credentials(DBUS_UID_UNSET, DBUS_PID_UNSET, NULL);
  if (creds2 == NULL) _dbus_test_fatal("oom");
  _dbus_assert(!_dbus_credentials_same_user(creds, creds2));
  _dbus_assert(_dbus_credentials_are_superset(creds, creds2));
  _dbus_credentials_unref(creds2);
  _dbus_credentials_clear(creds);
  _dbus_assert(!_dbus_credentials_include(creds, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert(!_dbus_credentials_include(creds, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert(!_dbus_credentials_include(creds, DBUS_CREDENTIAL_WINDOWS_SID));
  _dbus_assert(_dbus_credentials_get_unix_uid(creds) == DBUS_UID_UNSET);
  _dbus_assert(_dbus_credentials_get_pid(creds) == DBUS_PID_UNSET);
  _dbus_assert(_dbus_credentials_get_windows_sid(creds) == NULL);
  _dbus_assert(_dbus_credentials_are_empty(creds));
  _dbus_assert(_dbus_credentials_are_anonymous(creds));
  _dbus_credentials_unref(creds);
  return TRUE;
}
#endif
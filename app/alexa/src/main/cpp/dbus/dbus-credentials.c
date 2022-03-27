#include <string.h>
#include "config.h"
#include "dbus-credentials.h"
#include "dbus-internals.h"

struct DBusCredentials {
  int refcount;
  dbus_uid_t unix_uid;
  dbus_pid_t pid;
  char *windows_sid;
  char *linux_security_label;
  void *adt_audit_data;
  dbus_int32_t adt_audit_data_size;
};
DBusCredentials* _dbus_credentials_new(void) {
  DBusCredentials *creds;
  creds = dbus_new(DBusCredentials, 1);
  if (creds == NULL) return NULL;
  creds->refcount = 1;
  creds->unix_uid = DBUS_UID_UNSET;
  creds->pid = DBUS_PID_UNSET;
  creds->windows_sid = NULL;
  creds->linux_security_label = NULL;
  creds->adt_audit_data = NULL;
  creds->adt_audit_data_size = 0;
  return creds;
}
DBusCredentials* _dbus_credentials_new_from_current_process(void) {
  DBusCredentials *creds;
  creds = _dbus_credentials_new();
  if (creds == NULL) return NULL;
  if (!_dbus_credentials_add_from_current_process(creds)) {
      _dbus_credentials_unref(creds);
      return NULL;
  }
  return creds;
}
void _dbus_credentials_ref(DBusCredentials *credentials) {
  _dbus_assert(credentials->refcount > 0);
  credentials->refcount += 1;
}
void _dbus_credentials_unref(DBusCredentials *credentials) {
  _dbus_assert(credentials->refcount > 0);
  credentials->refcount -= 1;
  if (credentials->refcount == 0) {
      dbus_free(credentials->windows_sid);
      dbus_free(credentials->linux_security_label);
      dbus_free(credentials->adt_audit_data);
      dbus_free(credentials);
  }
}
dbus_bool_t _dbus_credentials_add_pid(DBusCredentials *credentials, dbus_pid_t pid) {
  credentials->pid = pid;
  return TRUE;
}
dbus_bool_t _dbus_credentials_add_unix_uid(DBusCredentials *credentials, dbus_uid_t uid) {
  credentials->unix_uid = uid;
  return TRUE;
}
dbus_bool_t _dbus_credentials_add_windows_sid(DBusCredentials *credentials, const char *windows_sid) {
  char *copy;
  copy = _dbus_strdup(windows_sid);
  if (copy == NULL) return FALSE;
  dbus_free (credentials->windows_sid);
  credentials->windows_sid = copy;
  return TRUE;
}
dbus_bool_t _dbus_credentials_add_linux_security_label(DBusCredentials *credentials, const char *label) {
  char *copy;
  copy = _dbus_strdup(label);
  if (copy == NULL) return FALSE;
  dbus_free(credentials->linux_security_label);
  credentials->linux_security_label = copy;
  return TRUE;
}
dbus_bool_t _dbus_credentials_add_adt_audit_data(DBusCredentials *credentials, void *audit_data, dbus_int32_t size) {
  void *copy;
  copy = _dbus_memdup(audit_data, size);
  if (copy == NULL) return FALSE;
  dbus_free (credentials->adt_audit_data);
  credentials->adt_audit_data = copy;
  credentials->adt_audit_data_size = size;
  return TRUE;
}
dbus_bool_t _dbus_credentials_include(DBusCredentials *credentials, DBusCredentialType type) {
  switch(type) {
      case DBUS_CREDENTIAL_UNIX_PROCESS_ID: return credentials->pid != DBUS_PID_UNSET;
      case DBUS_CREDENTIAL_UNIX_USER_ID: return credentials->unix_uid != DBUS_UID_UNSET;
      case DBUS_CREDENTIAL_WINDOWS_SID: return credentials->windows_sid != NULL;
      case DBUS_CREDENTIAL_LINUX_SECURITY_LABEL: return credentials->linux_security_label != NULL;
      case DBUS_CREDENTIAL_ADT_AUDIT_DATA_ID: return credentials->adt_audit_data != NULL;
      default:
          _dbus_assert_not_reached ("Unknown credential enum value");
          return FALSE;
  }
}
dbus_pid_t _dbus_credentials_get_pid(DBusCredentials *credentials) {
  return credentials->pid;
}
dbus_uid_t _dbus_credentials_get_unix_uid(DBusCredentials *credentials) {
  return credentials->unix_uid;
}
const char* _dbus_credentials_get_windows_sid(DBusCredentials *credentials) {
  return credentials->windows_sid;
}
const char *_dbus_credentials_get_linux_security_label(DBusCredentials *credentials) {
  return credentials->linux_security_label;
}
void *_dbus_credentials_get_adt_audit_data(DBusCredentials *credentials) {
  return credentials->adt_audit_data;
}
dbus_int32_t _dbus_credentials_get_adt_audit_data_size(DBusCredentials *credentials) {
  return credentials->adt_audit_data_size;
}
dbus_bool_t _dbus_credentials_are_superset(DBusCredentials *credentials, DBusCredentials *possible_subset) {
  return (possible_subset->pid == DBUS_PID_UNSET || possible_subset->pid == credentials->pid) && (possible_subset->unix_uid == DBUS_UID_UNSET ||
          possible_subset->unix_uid == credentials->unix_uid) && (possible_subset->windows_sid == NULL || (credentials->windows_sid &&
          strcmp(possible_subset->windows_sid, credentials->windows_sid) == 0)) && (possible_subset->linux_security_label == NULL ||
          (credentials->linux_security_label != NULL && strcmp(possible_subset->linux_security_label, credentials->linux_security_label) == 0)) &&
          (possible_subset->adt_audit_data == NULL || (credentials->adt_audit_data && memcmp(possible_subset->adt_audit_data, credentials->adt_audit_data,
          credentials->adt_audit_data_size) == 0));
}
dbus_bool_t _dbus_credentials_are_empty(DBusCredentials *credentials) {
  return credentials->pid == DBUS_PID_UNSET && credentials->unix_uid == DBUS_UID_UNSET && credentials->windows_sid == NULL &&
         credentials->linux_security_label == NULL && credentials->adt_audit_data == NULL;
}
dbus_bool_t _dbus_credentials_are_anonymous(DBusCredentials *credentials) {
  return credentials->unix_uid == DBUS_UID_UNSET && credentials->windows_sid == NULL;
}
dbus_bool_t _dbus_credentials_add_credentials(DBusCredentials *credentials, DBusCredentials *other_credentials) {
  return _dbus_credentials_add_credential(credentials,DBUS_CREDENTIAL_UNIX_PROCESS_ID, other_credentials) && _dbus_credentials_add_credential(credentials,
         DBUS_CREDENTIAL_UNIX_USER_ID, other_credentials) && _dbus_credentials_add_credential(credentials,DBUS_CREDENTIAL_ADT_AUDIT_DATA_ID,
         other_credentials) && _dbus_credentials_add_credential(credentials, DBUS_CREDENTIAL_LINUX_SECURITY_LABEL, other_credentials) &&
         _dbus_credentials_add_credential(credentials,DBUS_CREDENTIAL_WINDOWS_SID, other_credentials);
}
dbus_bool_t _dbus_credentials_add_credential(DBusCredentials *credentials, DBusCredentialType which, DBusCredentials *other_credentials) {
  if (which == DBUS_CREDENTIAL_UNIX_PROCESS_ID && other_credentials->pid != DBUS_PID_UNSET) {
      if (!_dbus_credentials_add_pid(credentials, other_credentials->pid)) return FALSE;
  } else if (which == DBUS_CREDENTIAL_UNIX_USER_ID && other_credentials->unix_uid != DBUS_UID_UNSET) {
      if (!_dbus_credentials_add_unix_uid(credentials, other_credentials->unix_uid)) return FALSE;
  } else if (which == DBUS_CREDENTIAL_WINDOWS_SID && other_credentials->windows_sid != NULL) {
      if (!_dbus_credentials_add_windows_sid(credentials, other_credentials->windows_sid)) return FALSE;
  } else if (which == DBUS_CREDENTIAL_LINUX_SECURITY_LABEL && other_credentials->linux_security_label != NULL) {
      if (!_dbus_credentials_add_linux_security_label(credentials, other_credentials->linux_security_label)) return FALSE;
  } else if (which == DBUS_CREDENTIAL_ADT_AUDIT_DATA_ID && other_credentials->adt_audit_data != NULL) {
      if (!_dbus_credentials_add_adt_audit_data(credentials, other_credentials->adt_audit_data, other_credentials->adt_audit_data_size)) return FALSE;
  }
  return TRUE;
}
void _dbus_credentials_clear(DBusCredentials *credentials) {
  credentials->pid = DBUS_PID_UNSET;
  credentials->unix_uid = DBUS_UID_UNSET;
  dbus_free (credentials->windows_sid);
  credentials->windows_sid = NULL;
  dbus_free (credentials->linux_security_label);
  credentials->linux_security_label = NULL;
  dbus_free (credentials->adt_audit_data);
  credentials->adt_audit_data = NULL;
  credentials->adt_audit_data_size = 0;
}
DBusCredentials* _dbus_credentials_copy(DBusCredentials *credentials) {
  DBusCredentials *copy;
  copy = _dbus_credentials_new();
  if (copy == NULL) return NULL;
  if (!_dbus_credentials_add_credentials(copy, credentials)) {
      _dbus_credentials_unref(copy);
      return NULL;
  }
  return copy;
}
dbus_bool_t _dbus_credentials_same_user(DBusCredentials *credentials, DBusCredentials *other_credentials) {
  return credentials->unix_uid == other_credentials->unix_uid && ((!(credentials->windows_sid || other_credentials->windows_sid)) ||
         (credentials->windows_sid && other_credentials->windows_sid && strcmp (credentials->windows_sid, other_credentials->windows_sid) == 0));
}
dbus_bool_t _dbus_credentials_to_string_append(DBusCredentials *credentials, DBusString *string) {
  dbus_bool_t join;
  join = FALSE;
  if (credentials->unix_uid != DBUS_UID_UNSET) {
      if (!_dbus_string_append_printf(string, "uid=" DBUS_UID_FORMAT, credentials->unix_uid)) goto oom;
      join = TRUE;
  }
  if (credentials->pid != DBUS_PID_UNSET) {
      if (!_dbus_string_append_printf(string, "%spid=" DBUS_PID_FORMAT, join ? " " : "", credentials->pid)) goto oom;
      join = TRUE;
  } else join = FALSE;
  if (credentials->windows_sid != NULL) {
      if (!_dbus_string_append_printf(string, "%ssid=%s", join ? " " : "", credentials->windows_sid)) goto oom;
      join = TRUE;
  } else join = FALSE;
  if (credentials->linux_security_label != NULL) {
      if (!_dbus_string_append_printf(string, "%slsm='%s'", join ? " " : "", credentials->linux_security_label)) goto oom;
      join = TRUE;
  }
  return TRUE;
oom:
  return FALSE;
}
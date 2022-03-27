#include <errno.h>
#include "../config.h"
#include "../dbus-internals.h"
#ifdef DBUS_UNIX
#include "../dbus-userdb.h"
#endif
#include "audit.h"

#ifdef HAVE_LIBAUDIT
static int audit_fd = -1;
#endif
void bus_audit_init(BusContext *context) {
#ifdef HAVE_LIBAUDIT
  int i;
  capng_get_caps_process();
  for (i = 3; i < 42; i++) _dbus_fd_set_close_on_exec(i);
  if (!capng_have_capability(CAPNG_EFFECTIVE, CAP_AUDIT_WRITE)) return;
  audit_fd = audit_open();
  if (audit_fd < 0) {
      int e = errno;
      if (e == EINVAL || e == EPROTONOSUPPORT || e == EAFNOSUPPORT) return;
      bus_context_log(context, DBUS_SYSTEM_LOG_WARNING, "Failed to open connection to the audit subsystem: %s", _dbus_strerror (e));
  }
#endif
}
int bus_audit_get_fd(void) {
#ifdef HAVE_LIBAUDIT
  if (audit_fd >= 0) return audit_fd;
#endif
  return -1;
}
void bus_audit_shutdown(void) {
#ifdef HAVE_LIBAUDIT
  audit_close(audit_fd);
#endif
}
#ifdef HAVE_LIBAUDIT
dbus_bool_t _dbus_change_to_daemon_user(const char *user, DBusError *error) {
  dbus_uid_t uid;
  dbus_gid_t gid;
  DBusString u;
  _dbus_string_init_const(&u, user);
  if (!_dbus_get_user_id_and_primary_group(&u, &uid, &gid)) {
      dbus_set_error(error, DBUS_ERROR_FAILED, "User '%s' does not appear to exist?", user);
      return FALSE;
  }
  if (_dbus_geteuid() == 0) {
      int rc;
      int have_audit_write;
      have_audit_write = capng_have_capability(CAPNG_PERMITTED, CAP_AUDIT_WRITE);
      capng_clear(CAPNG_SELECT_BOTH);
      if (have_audit_write) capng_update(CAPNG_ADD, CAPNG_EFFECTIVE | CAPNG_PERMITTED, CAP_AUDIT_WRITE);
      rc = capng_change_id(uid, gid, CAPNG_DROP_SUPP_GRP);
      if (rc) {
          switch(rc) {
              default: dbus_set_error (error, DBUS_ERROR_FAILED, "Failed to drop capabilities: %s\n", _dbus_strerror (errno));
              case -4: dbus_set_error (error, _dbus_error_from_errno (errno), "Failed to set GID to %lu: %s", gid, _dbus_strerror (errno)); break;
              case -5: dbus_set_error (error, _dbus_error_from_errno (errno), "Failed to drop supplementary groups: %s", _dbus_strerror (errno)); break;
              case -6: dbus_set_error (error, _dbus_error_from_errno (errno), "Failed to set UID to %lu: %s", uid, _dbus_strerror (errno)); break;
              case -7: dbus_set_error (error, _dbus_error_from_errno (errno), "Failed to unset keep-capabilities: %s\n", _dbus_strerror (errno)); break;
          }
          return FALSE;
      }
  }
  return TRUE;
}
#endif
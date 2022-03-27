#include "apparmor.h"

#ifdef HAVE_APPARMOR
#include <dbus/dbus-internals.h>
#include <dbus/dbus-string.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/apparmor.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#ifdef HAVE_LIBAUDIT
#include <libaudit.h>
#endif
#include "activation.h"
#include "audit.h"
#include "connection.h"
#include "utils.h"

static dbus_bool_t apparmor_enabled = FALSE;
typedef enum {
  APPARMOR_DISABLED,
  APPARMOR_ENABLED,
  APPARMOR_REQUIRED
} AppArmorConfigMode;
static AppArmorConfigMode apparmor_config_mode = APPARMOR_ENABLED;
struct BusAppArmorConfinement {
  int refcount;
  char *label;
  const char *mode;
};
static BusAppArmorConfinement *bus_con = NULL;
static BusAppArmorConfinement* bus_apparmor_confinement_new(char *label, const char *mode) {
  BusAppArmorConfinement *confinement;
  confinement = dbus_new0(BusAppArmorConfinement, 1);
  if (confinement != NULL) {
      confinement->refcount = 1;
      confinement->label = label;
      confinement->mode = mode;
  }
  return confinement;
}
static dbus_bool_t _bus_apparmor_detect_aa_dbus_support(dbus_bool_t *is_supported) {
  int mask_file;
  DBusString aa_dbus;
  char *aa_securityfs = NULL;
  dbus_bool_t retval = FALSE;
  *is_supported = FALSE;
  if (!_dbus_string_init(&aa_dbus)) return FALSE;
  if (aa_find_mountpoint(&aa_securityfs) != 0) goto out;
  if (!_dbus_string_append(&aa_dbus, aa_securityfs) || !_dbus_string_append(&aa_dbus, "/features/dbus/mask")) goto out;
  mask_file = open(_dbus_string_get_const_data(&aa_dbus), O_RDONLY | O_CLOEXEC);
  if (mask_file != -1) {
      *is_supported = TRUE;
      close(mask_file);
  }
  retval = TRUE;
out:
  free(aa_securityfs);
  _dbus_string_free(&aa_dbus);
  return retval;
}
static dbus_bool_t modestr_is_complain(const char *mode) {
  if (mode && strcmp(mode, "complain") == 0) return TRUE;
  return FALSE;
}
static void log_message(dbus_bool_t allow, const char *op, DBusString *data) {
  const char *mstr;
#ifdef HAVE_LIBAUDIT
  int audit_fd;
#endif
  if (allow) mstr = "ALLOWED";
  else mstr = "DENIED";
#ifdef HAVE_LIBAUDIT
  audit_fd = bus_audit_get_fd ();
  if (audit_fd >= 0) {
      DBusString avc;
      if (!_dbus_string_init(&avc)) goto syslog;
      if (!_dbus_string_append_printf(&avc, "apparmor=\"%s\" operation=\"dbus_%s\" %s\n", mstr, op, _dbus_string_get_const_data(data))) {
          _dbus_string_free(&avc);
          goto syslog;
      }
      audit_log_user_avc_message(audit_fd, AUDIT_USER_AVC, _dbus_string_get_const_data(&avc), NULL, NULL, NULL, getuid ());
      _dbus_string_free(&avc);
      return;
  }
syslog:
#endif
  syslog(LOG_USER | LOG_NOTICE, "apparmor=\"%s\" operation=\"dbus_%s\" %s\n", mstr, op, _dbus_string_get_const_data(data));
}
static dbus_bool_t _dbus_append_pair_uint(DBusString *auxdata, const char *name, unsigned long value) {
  return _dbus_string_append(auxdata, " ") && _dbus_string_append(auxdata, name) && _dbus_string_append(auxdata, "=") && _dbus_string_append_uint(auxdata, value);
}
static dbus_bool_t _dbus_append_pair_str (DBusString *auxdata, const char *name, const char *value) {
  return _dbus_string_append(auxdata, " ") && _dbus_string_append(auxdata, name) && _dbus_string_append(auxdata, "=\"") && _dbus_string_append(auxdata, value) &&
         _dbus_string_append(auxdata, "\"");
}
static dbus_bool_t _dbus_append_mask(DBusString *auxdata, uint32_t mask) {
  const char *mask_str;
  if (mask == AA_DBUS_SEND) mask_str = "send";
  else if (mask == AA_DBUS_RECEIVE) mask_str = "receive";
  else if (mask == AA_DBUS_BIND) mask_str = "bind";
  else return FALSE;
  return _dbus_append_pair_str(auxdata, "mask", mask_str);
  if ((!mode && con && strcmp(con, "unconfined") == 0) || strcmp (mode, "unconfined") == 0) return TRUE;
  return FALSE;
}
static dbus_bool_t query_append(DBusString *query, const char *buffer) {
  if (!_dbus_string_append_byte(query, '\0')) return FALSE;
  if (buffer && !_dbus_string_append(query, buffer)) return FALSE;
  return TRUE;
}
static dbus_bool_t build_common_query(DBusString *query, const char *con, const char *bustype) {
  return _dbus_string_insert_bytes(query, 0, AA_QUERY_CMD_LABEL_SIZE, 0) && _dbus_string_append(query, con) && _dbus_string_append_byte (query, '\0') &&
                                   _dbus_string_append_byte(query, AA_CLASS_DBUS) && _dbus_string_append(query, bustype ? bustype : "");
}
static dbus_bool_t build_service_query(DBusString *query, const char *con, const char *bustype, const char *name) {
  return build_common_query(query, con, bustype) && query_append (query, name);
}
static dbus_bool_t build_message_query(DBusString *query, const char *src_con, const char *bustype, const char *name, const char *dst_con, const char *path,
                                       const char *interface, const char *member) {
  return build_common_query (query, src_con, bustype) && query_append(query, name) && query_append(query, dst_con) && query_append(query, path) &&
                            query_append(query, interface) && query_append(query, member);
}
static dbus_bool_t build_eavesdrop_query (DBusString *query, const char *con, const char *bustype) {
  return build_common_query(query, con, bustype);
}
static void set_error_from_query_errno(DBusError *error, int error_number) {
  dbus_set_error(error, _dbus_error_from_errno(error_number), "Failed to query AppArmor policy: %s", _dbus_strerror(error_number));
}
static void set_error_from_denied_message(DBusError *error, DBusConnection *sender, DBusConnection *proposed_recipient, dbus_bool_t requested_reply,
                                          const char *msgtype, const char *path, const char *interface, const char *member, const char *error_name,
                                          const char *destination) {
  const char *proposed_recipient_loginfo;
  const char *unset = "(unset)";
  proposed_recipient_loginfo = proposed_recipient ? bus_connection_get_loginfo(proposed_recipient) : "bus";
  dbus_set_error(error, DBUS_ERROR_ACCESS_DENIED, "An AppArmor policy prevents this sender from sending this message to this recipient; type=\"%s\", "
                 "sender=\"%s\" (%s) interface=\"%s\" member=\"%s\" error name=\"%s\" requested_reply=\"%d\" destination=\"%s\" (%s)", msgtype,
                 bus_connection_get_name(sender), bus_connection_get_loginfo(sender), interface ? interface : unset, member ? member : unset,
                 error_name ? error_name : unset, requested_reply, destination, proposed_recipient_loginfo);
}
#endif
dbus_bool_t bus_apparmor_pre_init(void) {
#ifdef HAVE_APPARMOR
  apparmor_enabled = FALSE;
  if (!aa_is_enabled()) return TRUE;
  if (!_bus_apparmor_detect_aa_dbus_support(&apparmor_enabled)) return FALSE;
#endif
  return TRUE;
}
dbus_bool_t bus_apparmor_set_mode_from_config(const char *mode, DBusError *error) {
#ifdef HAVE_APPARMOR
  if (mode != NULL) {
      if (strcmp(mode, "disabled") == 0) apparmor_config_mode = APPARMOR_DISABLED;
      else if (strcmp(mode, "enabled") == 0) apparmor_config_mode = APPARMOR_ENABLED;
      else if (strcmp(mode, "required") == 0) apparmor_config_mode = APPARMOR_REQUIRED;
      else {
          dbus_set_error(error, DBUS_ERROR_FAILED, "Mode attribute on <apparmor> must have value \"required\", \"enabled\" or \"disabled\", not \"%s\"", mode);
          return FALSE;
      }
  }
  return TRUE;
#else
  if (mode == NULL || strcmp (mode, "disabled") == 0 || strcmp (mode, "enabled") == 0) return TRUE;
  dbus_set_error(error, DBUS_ERROR_FAILED,"Mode attribute on <apparmor> must have value \"enabled\" or \"disabled\" but cannot be \"%s\" when D-Bus is built "
                 "without AppArmor support", mode);
  return FALSE;
#endif
}
dbus_bool_t bus_apparmor_full_init(DBusError *error) {
#ifdef HAVE_APPARMOR
  char *label, *mode;
  if (apparmor_enabled) {
      if (apparmor_config_mode == APPARMOR_DISABLED) {
          apparmor_enabled = FALSE;
          return TRUE;
      }
      if (bus_con == NULL) {
          if (aa_getcon(&label, &mode) == -1) {
              dbus_set_error(error, DBUS_ERROR_FAILED, "Error getting AppArmor context of bus: %s", _dbus_strerror(errno));
              return FALSE;
          }
          bus_con = bus_apparmor_confinement_new(label, mode);
          if (bus_con == NULL) {
              BUS_SET_OOM(error);
              free(label);
              return FALSE;
          }
      }
  } else {
      if (apparmor_config_mode == APPARMOR_REQUIRED) {
          dbus_set_error(error, DBUS_ERROR_FAILED, "AppArmor mediation required but not present");
          return FALSE;
      } else if (apparmor_config_mode == APPARMOR_ENABLED) return TRUE;
  }
#endif
  return TRUE;
}
void bus_apparmor_shutdown(void) {
#ifdef HAVE_APPARMOR
  if (!apparmor_enabled) return;
  _dbus_verbose("AppArmor shutdown\n");
  bus_apparmor_confinement_unref(bus_con);
  bus_con = NULL;
#endif
}
dbus_bool_t bus_apparmor_enabled(void) {
#ifdef HAVE_APPARMOR
  return apparmor_enabled;
#else
  return FALSE;
#endif
}
void bus_apparmor_confinement_unref(BusAppArmorConfinement *confinement) {
#ifdef HAVE_APPARMOR
  if (!apparmor_enabled) return;
  _dbus_assert(confinement != NULL);
  _dbus_assert(confinement->refcount > 0);
  confinement->refcount -= 1;
  if (confinement->refcount == 0) {
      free(confinement->label);
      dbus_free(confinement);
  }
#endif
}
void bus_apparmor_confinement_ref(BusAppArmorConfinement *confinement) {
#ifdef HAVE_APPARMOR
  if (!apparmor_enabled) return;
  _dbus_assert(confinement != NULL);
  _dbus_assert(confinement->refcount > 0);
  confinement->refcount += 1;
#endif
}
BusAppArmorConfinement* bus_apparmor_init_connection_confinement(DBusConnection *connection, DBusError *error) {
#ifdef HAVE_APPARMOR
  BusAppArmorConfinement *confinement;
  char *label, *mode;
  int fd;
  if (!apparmor_enabled) return NULL;
  _dbus_assert(connection != NULL);
  if (!dbus_connection_get_socket(connection, &fd)) {
      dbus_set_error(error, DBUS_ERROR_FAILED, "Failed to get socket file descriptor of connection");
      return NULL;
  }
  if (aa_getpeercon(fd, &label, &mode) == -1) {
      if (errno == ENOMEM) BUS_SET_OOM(error);
      else dbus_set_error(error, _dbus_error_from_errno(errno), "Failed to get AppArmor confinement information of socket peer: %s", _dbus_strerror(errno));
      return NULL;
  }
  confinement = bus_apparmor_confinement_new(label, mode);
  if (confinement == NULL) {
      BUS_SET_OOM(error);
      free(label);
      return NULL;
  }
  return confinement;
#else
  return NULL;
#endif
}
dbus_bool_t bus_apparmor_allows_acquire_service(DBusConnection *connection, const char *bustype, const char *service_name, DBusError *error) {
#ifdef HAVE_APPARMOR
  BusAppArmorConfinement *con = NULL;
  DBusString qstr, auxdata;
  dbus_bool_t free_auxdata = FALSE;
  int allow = FALSE, audit = TRUE;
  unsigned long pid;
  int res, serrno = 0;
  if (!apparmor_enabled) return TRUE;
  _dbus_assert (connection != NULL);
  con = bus_connection_dup_apparmor_confinement(connection);
  if (is_unconfined(con->label, con->mode)) {
      allow = TRUE;
      audit = FALSE;
      goto out;
  }
  if (!_dbus_string_init(&qstr)) goto oom;
  if (!build_service_query(&qstr, con->label, bustype, service_name)) {
      _dbus_string_free(&qstr);
      goto oom;
  }
  res = aa_query_label(AA_DBUS_BIND, _dbus_string_get_data(&qstr), _dbus_string_get_length(&qstr), &allow, &audit);
  _dbus_string_free(&qstr);
  if (res == -1) {
      serrno = errno;
      set_error_from_query_errno(error, serrno);
      goto audit;
  }
  if (modestr_is_complain(con->mode)) allow = TRUE;
  if (!allow) {
      dbus_set_error(error, DBUS_ERROR_ACCESS_DENIED, "Connection \"%s\" is not allowed to own the service \"%s\" due to AppArmor policy",
                     bus_connection_is_active(connection) ? bus_connection_get_name(connection) : "(inactive)", service_name);
  }
  if (!audit) goto out;
audit:
  if (!_dbus_string_init(&auxdata)) goto oom;
  free_auxdata = TRUE;
  if (!_dbus_append_pair_str(&auxdata, "bus", bustype ? bustype : "unknown")) goto oom;
  if (!_dbus_append_pair_str(&auxdata, "name", service_name)) goto oom;
  if (serrno && !_dbus_append_pair_str(&auxdata, "info", strerror(serrno))) goto oom;
  if (!_dbus_append_mask(&auxdata, AA_DBUS_BIND)) goto oom;
  if (connection && dbus_connection_get_unix_process_id(connection, &pid) && !_dbus_append_pair_uint(&auxdata, "pid", pid)) goto oom;
  if (con->label && !_dbus_append_pair_str(&auxdata, "label", con->label)) goto oom;
  log_message(allow, "bind", &auxdata);
out:
  if (con != NULL) bus_apparmor_confinement_unref(con);
  if (free_auxdata) _dbus_string_free(&auxdata);
  return allow;
oom:
  if (error != NULL && !dbus_error_is_set(error)) BUS_SET_OOM(error);
  allow = FALSE;
  goto out;
#else
  return TRUE;
#endif
}
dbus_bool_t bus_apparmor_allows_send(DBusConnection *sender, DBusConnection *proposed_recipient, dbus_bool_t requested_reply, const char *bustype, int msgtype,
                                     const char *path, const char *interface, const char *member, const char *error_name, const char *destination, const char *source,
                                     BusActivationEntry *activation_entry, DBusError *error) {
#ifdef HAVE_APPARMOR
  BusAppArmorConfinement *src_con = NULL, *dst_con = NULL;
  DBusString qstr, auxdata;
  int src_allow = FALSE, dst_allow = FALSE;
  int src_audit = TRUE, dst_audit = TRUE;
  dbus_bool_t free_auxdata = FALSE;
  unsigned long pid;
  int len, res, src_errno = 0, dst_errno = 0;
  uint32_t src_perm = AA_DBUS_SEND, dst_perm = AA_DBUS_RECEIVE;
  const char *msgtypestr = dbus_message_type_to_string(msgtype);
  const char *dst_label = NULL;
  const char *dst_mode = NULL;
  if (!apparmor_enabled) return TRUE;
  _dbus_assert (sender != NULL);
  src_con = bus_connection_dup_apparmor_confinement(sender);
  if (proposed_recipient) dst_con = bus_connection_dup_apparmor_confinement(proposed_recipient);
  else if (activation_entry != NULL) dst_label = bus_activation_entry_get_assumed_apparmor_label(activation_entry);
  else {
      dst_con = bus_con;
      bus_apparmor_confinement_ref(dst_con);
  }
  if (dst_con != NULL) {
      dst_label = dst_con->label;
      dst_mode = dst_con->mode;
  }
  if (requested_reply) {
      src_allow = TRUE;
      dst_allow = TRUE;
      goto out;
  }
  if (is_unconfined(src_con->label, src_con->mode)) {
      src_allow = TRUE;
      src_audit = FALSE;
  } else {
      if (!_dbus_string_init(&qstr)) goto oom;
      if (!build_message_query(&qstr, src_con->label, bustype, destination, dst_label, path, interface, member)) {
          _dbus_string_free(&qstr);
          goto oom;
      }
      res = aa_query_label(src_perm, _dbus_string_get_data(&qstr), _dbus_string_get_length(&qstr), &src_allow, &src_audit);
      _dbus_string_free(&qstr);
      if (res == -1) {
          src_errno = errno;
          set_error_from_query_errno(error, src_errno);
          goto audit;
      }
  }
  if (activation_entry != NULL || is_unconfined(dst_label, dst_mode)) {
      dst_allow = TRUE;
      dst_audit = FALSE;
  } else {
      if (!_dbus_string_init(&qstr)) goto oom;
      if (!build_message_query(&qstr, dst_label, bustype, source, src_con->label, path, interface, member)) {
          _dbus_string_free(&qstr);
          goto oom;
      }
      res = aa_query_label(dst_perm, _dbus_string_get_data(&qstr), _dbus_string_get_length (&qstr), &dst_allow, &dst_audit);
      _dbus_string_free(&qstr);
      if (res == -1) {
          dst_errno = errno;
          set_error_from_query_errno(error, dst_errno);
          goto audit;
      }
  }
  if (modestr_is_complain(src_con->mode)) src_allow = TRUE;
  if (modestr_is_complain(dst_mode)) dst_allow = TRUE;
  if (!src_allow || !dst_allow) {
      set_error_from_denied_message(error, sender, proposed_recipient, requested_reply, msgtypestr, path, interface, member, error_name, destination);
  }
  if ((!src_audit && !dst_audit) || (msgtype == DBUS_MESSAGE_TYPE_METHOD_RETURN || msgtype == DBUS_MESSAGE_TYPE_ERROR)) goto out;
audit:
  if (!_dbus_string_init(&auxdata)) goto oom;
  free_auxdata = TRUE;
  if (!_dbus_append_pair_str(&auxdata, "bus", bustype ? bustype : "unknown")) goto oom;
  if (path && !_dbus_append_pair_str(&auxdata, "path", path)) goto oom;
  if (interface && !_dbus_append_pair_str(&auxdata, "interface", interface)) goto oom;
  if (member && !_dbus_append_pair_str(&auxdata, "member", member)) goto oom;
  if (error_name && !_dbus_append_pair_str(&auxdata, "error_name", error_name)) goto oom;
  len = _dbus_string_get_length(&auxdata);
  if (src_audit) {
      if (!_dbus_append_mask(&auxdata, src_perm)) goto oom;
      if (destination && !_dbus_append_pair_str(&auxdata, "name", destination)) goto oom;
      if (sender && dbus_connection_get_unix_process_id(sender, &pid) && !_dbus_append_pair_uint(&auxdata, "pid", pid)) goto oom;
      if (src_con->label && !_dbus_append_pair_str(&auxdata, "label", src_con->label)) goto oom;
      if (proposed_recipient && dbus_connection_get_unix_process_id(proposed_recipient, &pid) && !_dbus_append_pair_uint(&auxdata, "peer_pid", pid)) goto oom;
      if (dst_label && !_dbus_append_pair_str(&auxdata, "peer_label", dst_label)) goto oom;
      if (src_errno && !_dbus_append_pair_str(&auxdata, "info", strerror(src_errno))) goto oom;
      if (dst_errno && !_dbus_append_pair_str(&auxdata, "peer_info", strerror(dst_errno))) goto oom;
      log_message(src_allow, msgtypestr, &auxdata);
  }
  if (dst_audit) {
      _dbus_string_set_length(&auxdata, len);
      if (source && !_dbus_append_pair_str(&auxdata, "name", source)) goto oom;
      if (!_dbus_append_mask(&auxdata, dst_perm)) goto oom;
      if (proposed_recipient && dbus_connection_get_unix_process_id(proposed_recipient, &pid) && !_dbus_append_pair_uint(&auxdata, "pid", pid)) goto oom;
      if (dst_label && !_dbus_append_pair_str(&auxdata, "label", dst_label)) goto oom;
      if (sender && dbus_connection_get_unix_process_id(sender, &pid) && !_dbus_append_pair_uint(&auxdata, "peer_pid", pid)) goto oom;
      if (src_con->label && !_dbus_append_pair_str(&auxdata, "peer_label", src_con->label)) goto oom;
      if (dst_errno && !_dbus_append_pair_str(&auxdata, "info", strerror(dst_errno))) goto oom;
      if (src_errno && !_dbus_append_pair_str(&auxdata, "peer_info", strerror(src_errno))) goto oom;
      log_message(dst_allow, msgtypestr, &auxdata);
  }
out:
  if (src_con != NULL) bus_apparmor_confinement_unref(src_con);
  if (dst_con != NULL) bus_apparmor_confinement_unref(dst_con);
  if (free_auxdata) _dbus_string_free(&auxdata);
  return src_allow && dst_allow;
oom:
  if (error != NULL && !dbus_error_is_set(error)) BUS_SET_OOM(error);
  src_allow = FALSE;
  dst_allow = FALSE;
  goto out;
#else
  return TRUE;
#endif
}
dbus_bool_t bus_apparmor_allows_eavesdropping(DBusConnection *connection, const char *bustype, DBusError *error) {
#ifdef HAVE_APPARMOR
  BusAppArmorConfinement *con = NULL;
  DBusString qstr, auxdata;
  int allow = FALSE, audit = TRUE;
  dbus_bool_t free_auxdata = FALSE;
  unsigned long pid;
  int res, serrno = 0;
  if (!apparmor_enabled) return TRUE;
  con = bus_connection_dup_apparmor_confinement(connection);
  if (is_unconfined(con->label, con->mode)) {
      allow = TRUE;
      audit = FALSE;
      goto out;
  }
  if (!_dbus_string_init(&qstr)) goto oom;
  if (!build_eavesdrop_query(&qstr, con->label, bustype)) {
      _dbus_string_free(&qstr);
      goto oom;
  }
  res = aa_query_label(AA_DBUS_EAVESDROP, _dbus_string_get_data(&qstr), _dbus_string_get_length(&qstr), &allow, &audit);
  _dbus_string_free(&qstr);
  if (res == -1) {
      serrno = errno;
      set_error_from_query_errno(error, serrno);
      goto audit;
  }
  if (modestr_is_complain(con->mode)) allow = TRUE;
  if (!allow) {
      dbus_set_error(error, DBUS_ERROR_ACCESS_DENIED, "Connection \"%s\" is not allowed to eavesdrop due to " "AppArmor policy",
                     bus_connection_is_active(connection) ? bus_connection_get_name(connection) : "(inactive)");
  }
  if (!audit) goto out;
audit:
  if (!_dbus_string_init(&auxdata)) goto oom;
  free_auxdata = TRUE;
  if (!_dbus_append_pair_str(&auxdata, "bus", bustype ? bustype : "unknown")) goto oom;
  if (serrno && !_dbus_append_pair_str(&auxdata, "info", strerror(serrno))) goto oom;
  if (!_dbus_append_pair_str(&auxdata, "mask", "eavesdrop")) goto oom;
  if (connection && dbus_connection_get_unix_process_id(connection, &pid) && !_dbus_append_pair_uint(&auxdata, "pid", pid)) goto oom;
  if (con->label && !_dbus_append_pair_str(&auxdata, "label", con->label)) goto oom;
  log_message(allow, "eavesdrop", &auxdata);
out:
  if (con != NULL) bus_apparmor_confinement_unref(con);
  if (free_auxdata) _dbus_string_free(&auxdata);
  return allow;
 oom:
  if (error != NULL && !dbus_error_is_set (error)) BUS_SET_OOM(error);
  allow = FALSE;
  goto out;
#else
  return TRUE;
#endif
}
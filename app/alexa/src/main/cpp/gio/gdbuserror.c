#include <stdlib.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gdbuserror.h"
#include "gioenums.h"
#include "gioenumtypes.h"
#include "gioerror.h"
#include "gdbusprivate.h"

static const GDBusErrorEntry g_dbus_error_entries[] = {
  {G_DBUS_ERROR_FAILED,"org.freedesktop.DBus.Error.Failed" },
  {G_DBUS_ERROR_NO_MEMORY,"org.freedesktop.DBus.Error.NoMemory" },
  {G_DBUS_ERROR_SERVICE_UNKNOWN,"org.freedesktop.DBus.Error.ServiceUnknown" },
  {G_DBUS_ERROR_NAME_HAS_NO_OWNER,"org.freedesktop.DBus.Error.NameHasNoOwner" },
  {G_DBUS_ERROR_NO_REPLY,"org.freedesktop.DBus.Error.NoReply" },
  {G_DBUS_ERROR_IO_ERROR,"org.freedesktop.DBus.Error.IOError" },
  {G_DBUS_ERROR_BAD_ADDRESS,"org.freedesktop.DBus.Error.BadAddress" },
  {G_DBUS_ERROR_NOT_SUPPORTED,"org.freedesktop.DBus.Error.NotSupported" },
  {G_DBUS_ERROR_LIMITS_EXCEEDED,"org.freedesktop.DBus.Error.LimitsExceeded" },
  {G_DBUS_ERROR_ACCESS_DENIED,"org.freedesktop.DBus.Error.AccessDenied" },
  {G_DBUS_ERROR_AUTH_FAILED,"org.freedesktop.DBus.Error.AuthFailed" },
  {G_DBUS_ERROR_NO_SERVER,"org.freedesktop.DBus.Error.NoServer" },
  {G_DBUS_ERROR_TIMEOUT,"org.freedesktop.DBus.Error.Timeout" },
  {G_DBUS_ERROR_NO_NETWORK,"org.freedesktop.DBus.Error.NoNetwork" },
  {G_DBUS_ERROR_ADDRESS_IN_USE,"org.freedesktop.DBus.Error.AddressInUse" },
  {G_DBUS_ERROR_DISCONNECTED,"org.freedesktop.DBus.Error.Disconnected" },
  {G_DBUS_ERROR_INVALID_ARGS,"org.freedesktop.DBus.Error.InvalidArgs" },
  {G_DBUS_ERROR_FILE_NOT_FOUND,"org.freedesktop.DBus.Error.FileNotFound" },
  {G_DBUS_ERROR_FILE_EXISTS,"org.freedesktop.DBus.Error.FileExists" },
  {G_DBUS_ERROR_UNKNOWN_METHOD,"org.freedesktop.DBus.Error.UnknownMethod" },
  {G_DBUS_ERROR_TIMED_OUT,"org.freedesktop.DBus.Error.TimedOut" },
  {G_DBUS_ERROR_MATCH_RULE_NOT_FOUND,"org.freedesktop.DBus.Error.MatchRuleNotFound" },
  {G_DBUS_ERROR_MATCH_RULE_INVALID,"org.freedesktop.DBus.Error.MatchRuleInvalid" },
  {G_DBUS_ERROR_SPAWN_EXEC_FAILED,"org.freedesktop.DBus.Error.Spawn.ExecFailed" },
  {G_DBUS_ERROR_SPAWN_FORK_FAILED,"org.freedesktop.DBus.Error.Spawn.ForkFailed" },
  {G_DBUS_ERROR_SPAWN_CHILD_EXITED,"org.freedesktop.DBus.Error.Spawn.ChildExited" },
  {G_DBUS_ERROR_SPAWN_CHILD_SIGNALED,"org.freedesktop.DBus.Error.Spawn.ChildSignaled" },
  {G_DBUS_ERROR_SPAWN_FAILED,"org.freedesktop.DBus.Error.Spawn.Failed" },
  {G_DBUS_ERROR_SPAWN_SETUP_FAILED,"org.freedesktop.DBus.Error.Spawn.FailedToSetup" },
  {G_DBUS_ERROR_SPAWN_CONFIG_INVALID,"org.freedesktop.DBus.Error.Spawn.ConfigInvalid" },
  {G_DBUS_ERROR_SPAWN_SERVICE_INVALID,"org.freedesktop.DBus.Error.Spawn.ServiceNotValid" },
  {G_DBUS_ERROR_SPAWN_SERVICE_NOT_FOUND,"org.freedesktop.DBus.Error.Spawn.ServiceNotFound" },
  {G_DBUS_ERROR_SPAWN_PERMISSIONS_INVALID,"org.freedesktop.DBus.Error.Spawn.PermissionsInvalid" },
  {G_DBUS_ERROR_SPAWN_FILE_INVALID,"org.freedesktop.DBus.Error.Spawn.FileInvalid" },
  {G_DBUS_ERROR_SPAWN_NO_MEMORY,"org.freedesktop.DBus.Error.Spawn.NoMemory" },
  {G_DBUS_ERROR_UNIX_PROCESS_ID_UNKNOWN,"org.freedesktop.DBus.Error.UnixProcessIdUnknown" },
  {G_DBUS_ERROR_INVALID_SIGNATURE,"org.freedesktop.DBus.Error.InvalidSignature" },
  {G_DBUS_ERROR_INVALID_FILE_CONTENT,"org.freedesktop.DBus.Error.InvalidFileContent" },
  {G_DBUS_ERROR_SELINUX_SECURITY_CONTEXT_UNKNOWN,"org.freedesktop.DBus.Error.SELinuxSecurityContextUnknown" },
  {G_DBUS_ERROR_ADT_AUDIT_DATA_UNKNOWN,"org.freedesktop.DBus.Error.AdtAuditDataUnknown" },
  {G_DBUS_ERROR_OBJECT_PATH_IN_USE,"org.freedesktop.DBus.Error.ObjectPathInUse" }
};
GQuark g_dbus_error_quark(void) {
  G_STATIC_ASSERT(G_N_ELEMENTS(g_dbus_error_entries) - 1 == G_DBUS_ERROR_OBJECT_PATH_IN_USE);
  static volatile gsize quark_volatile = 0;
  g_dbus_error_register_error_domain("g-dbus-error-quark", &quark_volatile, g_dbus_error_entries,G_N_ELEMENTS(g_dbus_error_entries));
  return (GQuark)quark_volatile;
}
void g_dbus_error_register_error_domain(const gchar *error_domain_quark_name, volatile gsize *quark_volatile, const GDBusErrorEntry *entries, guint num_entries) {
  g_return_if_fail(error_domain_quark_name != NULL);
  g_return_if_fail(quark_volatile != NULL);
  g_return_if_fail(entries != NULL);
  g_return_if_fail(num_entries > 0);
  if (g_once_init_enter(quark_volatile)) {
      guint n;
      GQuark quark;
      quark = g_quark_from_static_string(error_domain_quark_name);
      for (n = 0; n < num_entries; n++) g_warn_if_fail(g_dbus_error_register_error(quark, entries[n].error_code, entries[n].dbus_error_name));
      g_once_init_leave(quark_volatile, quark);
  }
}
static gboolean _g_dbus_error_decode_gerror(const gchar *dbus_name, GQuark *out_error_domain, gint *out_error_code) {
  gboolean ret;
  guint n;
  GString *s;
  gchar *domain_quark_string;
  ret = FALSE;
  s = NULL;
  if (g_str_has_prefix(dbus_name, "org.gtk.GDBus.UnmappedGError.Quark._")) {
      s = g_string_new(NULL);
      for (n = sizeof "org.gtk.GDBus.UnmappedGError.Quark._" - 1; dbus_name[n] != '.' && dbus_name[n] != '\0'; n++) {
          if (g_ascii_isalnum(dbus_name[n])) { g_string_append_c(s, dbus_name[n]); }
          else if (dbus_name[n] == '_') {
              guint nibble_top;
              guint nibble_bottom;
              n++;
              nibble_top = dbus_name[n];
              if (nibble_top >= '0' && nibble_top <= '9') nibble_top -= '0';
              else if (nibble_top >= 'a' && nibble_top <= 'f') nibble_top -= ('a' - 10);
              else goto not_mapped;
              n++;
              nibble_bottom = dbus_name[n];
              if (nibble_bottom >= '0' && nibble_bottom <= '9') nibble_bottom -= '0';
              else if (nibble_bottom >= 'a' && nibble_bottom <= 'f') nibble_bottom -= ('a' - 10);
              else goto not_mapped;
              g_string_append_c(s, (nibble_top<<4) | nibble_bottom);
          } else goto not_mapped;
      }
      if (!g_str_has_prefix(dbus_name + n, ".Code")) goto not_mapped;
      domain_quark_string = g_string_free(s, FALSE);
      s = NULL;
      if (out_error_domain != NULL) *out_error_domain = g_quark_from_string(domain_quark_string);
      g_free(domain_quark_string);
      if (out_error_code != NULL) *out_error_code = atoi(dbus_name + n + sizeof ".Code" - 1);
      ret = TRUE;
  }
not_mapped:
  if (s != NULL) g_string_free(s, TRUE);
  return ret;
}
typedef struct {
  GQuark error_domain;
  gint error_code;
} QuarkCodePair;
typedef struct {
  QuarkCodePair pair;
  gchar *gdbus_error_name;
  gchar *dbus_error_name;
} RegisteredError;
static guint quark_code_pair_hash_func(const QuarkCodePair *pair) {
  gint val;
  val = pair->error_domain + pair->error_code;
  return g_int_hash(&val);
}
static gboolean quark_code_pair_equal_func(const QuarkCodePair *a, const QuarkCodePair *b) {
  return (a->error_domain == b->error_domain) && (a->error_code == b->error_code);
}
static void registered_error_free(RegisteredError *re) {
  g_free(re->dbus_error_name);
  g_free(re);
}
G_LOCK_DEFINE_STATIC(error_lock);
static GHashTable *quark_code_pair_to_re = NULL;
static GHashTable *dbus_error_name_to_re = NULL;
gboolean g_dbus_error_register_error(GQuark error_domain, gint error_code, const gchar *dbus_error_name) {
  gboolean ret;
  QuarkCodePair pair;
  RegisteredError *re;
  g_return_val_if_fail(dbus_error_name != NULL, FALSE);
  ret = FALSE;
  G_LOCK(error_lock);
  if (quark_code_pair_to_re == NULL) {
      g_assert(dbus_error_name_to_re == NULL);
      quark_code_pair_to_re = g_hash_table_new((GHashFunc) quark_code_pair_hash_func, (GEqualFunc)quark_code_pair_equal_func);
      dbus_error_name_to_re = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)registered_error_free);
  }
  if (g_hash_table_lookup(dbus_error_name_to_re, dbus_error_name) != NULL) goto out;
  pair.error_domain = error_domain;
  pair.error_code = error_code;
  if (g_hash_table_lookup(quark_code_pair_to_re, &pair) != NULL) goto out;
  re = g_new0(RegisteredError, 1);
  re->pair = pair;
  re->dbus_error_name = g_strdup(dbus_error_name);
  g_hash_table_insert(quark_code_pair_to_re, &(re->pair), re);
  g_hash_table_insert(dbus_error_name_to_re, re->dbus_error_name, re);
  ret = TRUE;
out:
  G_UNLOCK(error_lock);
  return ret;
}
gboolean g_dbus_error_unregister_error(GQuark error_domain, gint error_code, const gchar *dbus_error_name) {
  gboolean ret;
  RegisteredError *re;
  guint hash_size;
  g_return_val_if_fail(dbus_error_name != NULL, FALSE);
  ret = FALSE;
  G_LOCK(error_lock);
  if (dbus_error_name_to_re == NULL) {
      g_assert(quark_code_pair_to_re == NULL);
      goto out;
  }
  re = g_hash_table_lookup(dbus_error_name_to_re, dbus_error_name);
  if (re == NULL) {
      QuarkCodePair pair;
      pair.error_domain = error_domain;
      pair.error_code = error_code;
      g_warn_if_fail(g_hash_table_lookup (quark_code_pair_to_re, &pair) == NULL);
      goto out;
  }
  ret = TRUE;
  g_warn_if_fail(g_hash_table_lookup(quark_code_pair_to_re, &(re->pair)) == re);
  g_warn_if_fail(g_hash_table_remove(quark_code_pair_to_re, &(re->pair)));
  g_warn_if_fail(g_hash_table_remove(dbus_error_name_to_re, re->dbus_error_name));
  hash_size = g_hash_table_size(dbus_error_name_to_re);
  if (hash_size == 0) {
      g_warn_if_fail(g_hash_table_size(quark_code_pair_to_re) == 0);
      g_hash_table_unref(dbus_error_name_to_re);
      dbus_error_name_to_re = NULL;
      g_hash_table_unref(quark_code_pair_to_re);
      quark_code_pair_to_re = NULL;
  } else { g_warn_if_fail(g_hash_table_size(quark_code_pair_to_re) == hash_size); }
out:
  G_UNLOCK(error_lock);
  return ret;
}
gboolean g_dbus_error_is_remote_error(const GError *error) {
  g_return_val_if_fail(error != NULL, FALSE);
  return g_str_has_prefix(error->message, "GDBus.Error:");
}
gchar *g_dbus_error_get_remote_error(const GError *error) {
  RegisteredError *re;
  gchar *ret;
  g_return_val_if_fail(error != NULL, NULL);
  _g_dbus_initialize();
  ret = NULL;
  G_LOCK(error_lock);
  re = NULL;
  if (quark_code_pair_to_re != NULL) {
      QuarkCodePair pair;
      pair.error_domain = error->domain;
      pair.error_code = error->code;
      g_assert(dbus_error_name_to_re != NULL);
      re = g_hash_table_lookup(quark_code_pair_to_re, &pair);
  }
  if (re != NULL) ret = g_strdup(re->dbus_error_name);
  else {
      if (g_str_has_prefix(error->message, "GDBus.Error:")) {
          const gchar *begin;
          const gchar *end;
          begin = error->message + sizeof("GDBus.Error:") -1;
          end = strstr(begin, ":");
          if (end != NULL && end[1] == ' ') ret = g_strndup(begin, end - begin);
      }
  }
  G_UNLOCK(error_lock);
  return ret;
}
GError *g_dbus_error_new_for_dbus_error(const gchar *dbus_error_name, const gchar *dbus_error_message) {
  GError *error;
  RegisteredError *re;
  g_return_val_if_fail(dbus_error_name != NULL, NULL);
  g_return_val_if_fail(dbus_error_message != NULL, NULL);
  _g_dbus_initialize();
  G_LOCK(error_lock);
  re = NULL;
  if (dbus_error_name_to_re != NULL) {
      g_assert(quark_code_pair_to_re != NULL);
      re = g_hash_table_lookup(dbus_error_name_to_re, dbus_error_name);
  }
  if (re != NULL) error = g_error_new(re->pair.error_domain, re->pair.error_code,"GDBus.Error:%s: %s", dbus_error_name, dbus_error_message);
  else {
      GQuark error_domain = 0;
      gint error_code = 0;
      if (_g_dbus_error_decode_gerror(dbus_error_name, &error_domain, &error_code)) {
          error = g_error_new(error_domain, error_code,"GDBus.Error:%s: %s", dbus_error_name, dbus_error_message);
      } else error = g_error_new(G_IO_ERROR,G_IO_ERROR_DBUS_ERROR,"GDBus.Error:%s: %s", dbus_error_name, dbus_error_message);
  }
  G_UNLOCK(error_lock);
  return error;
}
void g_dbus_error_set_dbus_error(GError **error, const gchar *dbus_error_name, const gchar *dbus_error_message, const gchar *format, ...) {
  g_return_if_fail(error == NULL || *error == NULL);
  g_return_if_fail(dbus_error_name != NULL);
  g_return_if_fail(dbus_error_message != NULL);
  if (error == NULL) return;
  if (format == NULL) *error = g_dbus_error_new_for_dbus_error(dbus_error_name, dbus_error_message);
  else {
      va_list var_args;
      va_start(var_args, format);
      g_dbus_error_set_dbus_error_valist(error, dbus_error_name, dbus_error_message, format, var_args);
      va_end(var_args);
  }
}
void g_dbus_error_set_dbus_error_valist(GError **error, const gchar *dbus_error_name, const gchar *dbus_error_message, const gchar *format, va_list var_args) {
  g_return_if_fail(error == NULL || *error == NULL);
  g_return_if_fail(dbus_error_name != NULL);
  g_return_if_fail(dbus_error_message != NULL);
  if (error == NULL) return;
  if (format != NULL) {
      gchar *message;
      gchar *s;
      message = g_strdup_vprintf(format, var_args);
      s = g_strdup_printf("%s: %s", message, dbus_error_message);
      *error = g_dbus_error_new_for_dbus_error(dbus_error_name, s);
      g_free(s);
      g_free(message);
  } else *error = g_dbus_error_new_for_dbus_error(dbus_error_name, dbus_error_message);
}
gboolean g_dbus_error_strip_remote_error(GError *error) {
  gboolean ret;
  g_return_val_if_fail(error != NULL, FALSE);
  ret = FALSE;
  if (g_str_has_prefix(error->message, "GDBus.Error:")) {
      const gchar *begin;
      const gchar *end;
      gchar *new_message;
      begin = error->message + sizeof("GDBus.Error:") -1;
      end = strstr(begin, ":");
      if (end != NULL && end[1] == ' ') {
          new_message = g_strdup(end + 2);
          g_free(error->message);
          error->message = new_message;
          ret = TRUE;
      }
  }
  return ret;
}
gchar *g_dbus_error_encode_gerror(const GError *error) {
  RegisteredError *re;
  gchar *error_name;
  g_return_val_if_fail(error != NULL, NULL);
  _g_dbus_initialize();
  error_name = NULL;
  G_LOCK (error_lock);
  re = NULL;
  if (quark_code_pair_to_re != NULL) {
      QuarkCodePair pair;
      pair.error_domain = error->domain;
      pair.error_code = error->code;
      g_assert(dbus_error_name_to_re != NULL);
      re = g_hash_table_lookup(quark_code_pair_to_re, &pair);
  }
  if (re != NULL) {
      error_name = g_strdup(re->dbus_error_name);
      G_UNLOCK(error_lock);
  } else {
      const gchar *domain_as_string;
      GString *s;
      guint n;
      G_UNLOCK(error_lock);
      domain_as_string = g_quark_to_string(error->domain);
      s = g_string_new("org.gtk.GDBus.UnmappedGError.Quark._");
      for (n = 0; domain_as_string[n] != 0; n++) {
          gint c = domain_as_string[n];
          if (g_ascii_isalnum(c)) { g_string_append_c(s, c); }
          else {
              guint nibble_top;
              guint nibble_bottom;
              g_string_append_c(s, '_');
              nibble_top = ((int)domain_as_string[n]) >> 4;
              nibble_bottom = ((int)domain_as_string[n]) & 0x0f;
              if (nibble_top < 10) nibble_top += '0';
              else nibble_top += 'a' - 10;
              if (nibble_bottom < 10) nibble_bottom += '0';
              else nibble_bottom += 'a' - 10;
              g_string_append_c(s, nibble_top);
              g_string_append_c(s, nibble_bottom);
          }
      }
      g_string_append_printf(s, ".Code%d", error->code);
      error_name = g_string_free(s, FALSE);
  }
  return error_name;
}
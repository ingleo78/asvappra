#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include "../glib/glibintl.h"
#include "../gobject/gvaluecollector.h"
#include "config.h"
#include "gcredentials.h"
#include "gnetworkingprivate.h"
#include "gioerror.h"

struct _GCredentials {
  GObject parent_instance;
#ifdef __linux__
  struct ucred native;
#elif defined(__FreeBSD__)
  struct cmsgcred native;
#else
#ifdef __GNUC__
#warning Please add GCredentials support for your OS
#endif
#endif
};
struct _GCredentialsClass {
  GObjectClass parent_class;
};
G_DEFINE_TYPE(GCredentials, g_credentials, G_TYPE_OBJECT);
static void g_credentials_finalize(GObject *object) {
  G_GNUC_UNUSED GCredentials *credentials = G_CREDENTIALS (object);
  if (G_OBJECT_CLASS(g_credentials_parent_class)->finalize != NULL) G_OBJECT_CLASS(g_credentials_parent_class)->finalize(object);
}
static void g_credentials_class_init(GCredentialsClass *klass) {
  GObjectClass *gobject_class;
  gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_credentials_finalize;
}
static void g_credentials_init(GCredentials *credentials) {
#ifdef __linux__
  credentials->native.pid = getpid();
  credentials->native.uid = geteuid();
  credentials->native.gid = getegid();
#elif defined(__FreeBSD__)
  memset(&credentials->native, 0, sizeof(struct cmsgcred));
  credentials->native.cmcred_pid  = getpid();
  credentials->native.cmcred_euid = geteuid();
  credentials->native.cmcred_gid  = getegid();
#endif
}
GCredentials *g_credentials_new(void) {
  return g_object_new(G_TYPE_CREDENTIALS, NULL);
}
gchar *g_credentials_to_string(GCredentials *credentials) {
  GString *ret;
  g_return_val_if_fail(G_IS_CREDENTIALS(credentials), NULL);
  ret = g_string_new("GCredentials:");
#ifdef __linux__
  g_string_append(ret, "linux-ucred:");
  if (credentials->native.pid != -1) g_string_append_printf(ret, "pid=%" G_GINT64_FORMAT ",", (gint64)credentials->native.pid);
  if (credentials->native.uid != -1) g_string_append_printf(ret, "uid=%" G_GINT64_FORMAT ",", (gint64)credentials->native.uid);
  if (credentials->native.gid != -1) g_string_append_printf(ret, "gid=%" G_GINT64_FORMAT ",", (gint64)credentials->native.gid);
  if (ret->str[ret->len - 1] == ',') ret->str[ret->len - 1] = '\0';
#elif defined(__FreeBSD__)
  g_string_append (ret, "freebsd-cmsgcred:");
  if (credentials->native.cmcred_pid != -1) g_string_append_printf(ret, "pid=%" G_GINT64_FORMAT ",", (gint64)credentials->native.cmcred_pid);
  if (credentials->native.cmcred_euid != -1) g_string_append_printf(ret, "uid=%" G_GINT64_FORMAT ",", (gint64)credentials->native.cmcred_euid);
  if (credentials->native.cmcred_gid != -1) g_string_append_printf(ret, "gid=%" G_GINT64_FORMAT ",", (gint64)credentials->native.cmcred_gid);
#else
  g_string_append(ret, "unknown");
#endif
  return g_string_free(ret, FALSE);
}
gboolean g_credentials_is_same_user(GCredentials *credentials, GCredentials *other_credentials, GError **error) {
  gboolean ret;
  g_return_val_if_fail(G_IS_CREDENTIALS(credentials), FALSE);
  g_return_val_if_fail(G_IS_CREDENTIALS(other_credentials), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = FALSE;
#ifdef __linux__
  if (credentials->native.uid == other_credentials->native.uid) ret = TRUE;
#elif defined(__FreeBSD__)
  if (credentials->native.cmcred_euid == other_credentials->native.cmcred_euid) ret = TRUE;
#else
  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("GCredentials is not implemented on this OS"));
#endif
  return ret;
}
gpointer g_credentials_get_native(GCredentials *credentials, GCredentialsType native_type) {
  gpointer ret;
  g_return_val_if_fail(G_IS_CREDENTIALS(credentials), NULL);
  ret = NULL;
#ifdef __linux__
  if (native_type != G_CREDENTIALS_TYPE_LINUX_UCRED) {
      g_warning("g_credentials_get_native: Trying to get credentials of type %d but only G_CREDENTIALS_TYPE_LINUX_UCRED is supported.", native_type);
  } else ret = &credentials->native;
#elif defined(__FreeBSD__)
  if (native_type != G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED) {
      g_warning("g_credentials_get_native: Trying to get credentials of type %d but only G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED is supported.", native_type);
  } else ret = &credentials->native;
#else
  g_warning("g_credentials_get_native: Trying to get credentials but GLib has no support for the native credentials type. Please add support.");
#endif
  return ret;
}
void g_credentials_set_native(GCredentials *credentials, GCredentialsType native_type, gpointer native) {
#ifdef __linux__
  if (native_type != G_CREDENTIALS_TYPE_LINUX_UCRED) {
      g_warning("g_credentials_set_native: Trying to set credentials of type %d but only G_CREDENTIALS_TYPE_LINUX_UCRED is supported.", native_type);
  } else memcpy(&credentials->native, native, sizeof(struct ucred));
#elif defined(__FreeBSD__)
  if (native_type != G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED) {
      g_warning("g_credentials_set_native: Trying to set credentials of type %d but only G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED is supported.", native_type);
  } else memcpy(&credentials->native, native, sizeof (struct cmsgcred));
#else
  g_warning("g_credentials_set_native: Trying to set credentials but GLib has no support for the native credentials type. Please add support.");
#endif
}
#ifndef G_OS_UNIX
uid_t g_credentials_get_unix_user(GCredentials *credentials, GError **error) {
  uid_t ret;
  g_return_val_if_fail(G_IS_CREDENTIALS(credentials), -1);
  g_return_val_if_fail(error == NULL || *error == NULL, -1);
#ifdef __linux__
  ret = credentials->native.uid;
#elif defined(__FreeBSD__)
  ret = credentials->native.cmcred_euid;
#else
  ret = -1;
  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("There is no GCredentials support for your platform"));
#endif
  return ret;
}
gboolean g_credentials_set_unix_user(GCredentials *credentials, uid_t uid, GError **error) {
  gboolean ret;
  g_return_val_if_fail(G_IS_CREDENTIALS(credentials), FALSE);
  g_return_val_if_fail(uid != -1, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = FALSE;
#ifdef __linux__
  credentials->native.uid = uid;
  ret = TRUE;
#elif defined(__FreeBSD__)
  credentials->native.cmcred_euid = uid;
  ret = TRUE;
#else
  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("GCredentials is not implemented on this OS"));
#endif
  return ret;
}
#endif
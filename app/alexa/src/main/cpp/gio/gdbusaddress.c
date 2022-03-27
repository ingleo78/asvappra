#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include "../gio/gunixsocketaddress.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gioerror.h"
#include "gdbusutils.h"
#include "gdbusaddress.h"
#include "gdbuserror.h"
#include "gioenumtypes.h"
#include "gnetworkaddress.h"
#include "gsocketclient.h"
#include "giostream.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "gdbusprivate.h"
#include "gsocketconnectable.h"

static gchar *get_session_address_platform_specific (GError **error);
gboolean g_dbus_is_address(const gchar *string) {
  guint n;
  gchar **a;
  gboolean ret;
  ret = FALSE;
  g_return_val_if_fail(string != NULL, FALSE);
  a = g_strsplit(string, ";", 0);
  if (a[0] == NULL) goto out;
  for (n = 0; a[n] != NULL; n++) {
      if (!_g_dbus_address_parse_entry(a[n],NULL,NULL,NULL)) goto out;
  }
  ret = TRUE;
out:
  g_strfreev(a);
  return ret;
}
static gboolean is_valid_unix(const gchar *address_entry, GHashTable *key_value_pairs, GError **error) {
  gboolean ret;
  GList *keys;
  GList *l;
  const gchar *path;
  const gchar *tmpdir;
  const gchar *abstract;
  ret = FALSE;
  keys = NULL;
  path = NULL;
  tmpdir = NULL;
  abstract = NULL;
  keys = g_hash_table_get_keys(key_value_pairs);
  for (l = keys; l != NULL; l = l->next) {
      const gchar *key = l->data;
      if (g_strcmp0(key, "path") == 0) path = g_hash_table_lookup(key_value_pairs, key);
      else if (g_strcmp0(key, "tmpdir") == 0) tmpdir = g_hash_table_lookup(key_value_pairs, key);
      else if (g_strcmp0(key, "abstract") == 0) abstract = g_hash_table_lookup(key_value_pairs, key);
      else {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Unsupported key `%s' in address entry `%s'"), key, address_entry);
          goto out;
      }
  }
  if (path != NULL) {
      if (tmpdir != NULL || abstract != NULL) goto meaningless;
  } else if (tmpdir != NULL) {
      if (path != NULL || abstract != NULL) goto meaningless;
  } else if (abstract != NULL) {
      if (path != NULL || tmpdir != NULL) goto meaningless;
  } else {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Address `%s' is invalid (need exactly one of path, tmpdir or abstract keys)"),
                  address_entry);
      goto out;
  }
  ret= TRUE;
  goto out;
meaningless:
  g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Meaningless key/value pair combination in address entry `%s'"), address_entry);
out:
  g_list_free(keys);
  return ret;
}
static gboolean is_valid_nonce_tcp(const gchar *address_entry, GHashTable *key_value_pairs, GError **error) {
  gboolean ret;
  GList *keys;
  GList *l;
  const gchar *host;
  const gchar *port;
  const gchar *family;
  const gchar *nonce_file;
  gint port_num;
  gchar *endp;
  ret = FALSE;
  keys = NULL;
  host = NULL;
  port = NULL;
  family = NULL;
  nonce_file = NULL;
  keys = g_hash_table_get_keys(key_value_pairs);
  for (l = keys; l != NULL; l = l->next) {
      const gchar *key = l->data;
      if (g_strcmp0(key, "host") == 0) host = g_hash_table_lookup(key_value_pairs, key);
      else if (g_strcmp0(key, "port") == 0) port = g_hash_table_lookup(key_value_pairs, key);
      else if (g_strcmp0(key, "family") == 0) family = g_hash_table_lookup(key_value_pairs, key);
      else if (g_strcmp0(key, "noncefile") == 0) nonce_file = g_hash_table_lookup(key_value_pairs, key);
      else {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Unsupported key `%s' in address entry `%s'"), key, address_entry);
          goto out;
      }
  }
  if (port != NULL) {
      port_num = strtol(port, &endp, 10);
      if ((*port == '\0' || *endp != '\0') || port_num < 0 || port_num >= 65536) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the port attribute is malformed"), address_entry);
          goto out;
      }
  }
  if (family != NULL && !(g_strcmp0(family, "ipv4") == 0 || g_strcmp0(family, "ipv6") == 0)) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the family attribute is malformed"), address_entry);
      goto out;
  }
  ret= TRUE;
out:
  g_list_free(keys);
  return ret;
}
static gboolean is_valid_tcp(const gchar  *address_entry, GHashTable *key_value_pairs, GError **error) {
  gboolean ret;
  GList *keys;
  GList *l;
  const gchar *host;
  const gchar *port;
  const gchar *family;
  gint port_num;
  gchar *endp;
  ret = FALSE;
  keys = NULL;
  host = NULL;
  port = NULL;
  family = NULL;
  keys = g_hash_table_get_keys(key_value_pairs);
  for (l = keys; l != NULL; l = l->next) {
      const gchar *key = l->data;
      if (g_strcmp0(key, "host") == 0) host = g_hash_table_lookup(key_value_pairs, key);
      else if (g_strcmp0(key, "port") == 0) port = g_hash_table_lookup(key_value_pairs, key);
      else if (g_strcmp0(key, "family") == 0) family = g_hash_table_lookup(key_value_pairs, key);
      else {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Unsupported key `%s' in address entry `%s'"), key, address_entry);
          goto out;
      }
  }
  if (port != NULL) {
      port_num = strtol(port, &endp, 10);
      if ((*port == '\0' || *endp != '\0') || port_num < 0 || port_num >= 65536) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the port attribute is malformed"), address_entry);
          goto out;
      }
  }
  if (family != NULL && !(g_strcmp0(family, "ipv4") == 0 || g_strcmp0(family, "ipv6") == 0)) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the family attribute is malformed"), address_entry);
      goto out;
  }
  ret= TRUE;
out:
  g_list_free(keys);
  return ret;
}
gboolean g_dbus_is_supported_address(const gchar *string, GError **error) {
  guint n;
  gchar **a;
  gboolean ret;
  ret = FALSE;
  g_return_val_if_fail(string != NULL, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  a = g_strsplit(string, ";", 0);
  for (n = 0; a[n] != NULL; n++) {
      gchar *transport_name;
      GHashTable *key_value_pairs;
      gboolean supported;
      if (!_g_dbus_address_parse_entry (a[n], &transport_name, &key_value_pairs, error)) goto out;
      supported = FALSE;
      if (g_strcmp0(transport_name, "unix") == 0) supported = is_valid_unix(a[n], key_value_pairs, error);
      else if (g_strcmp0(transport_name, "tcp") == 0) supported = is_valid_tcp(a[n], key_value_pairs, error);
      else if (g_strcmp0(transport_name, "nonce-tcp") == 0) supported = is_valid_nonce_tcp(a[n], key_value_pairs, error);
      else if (g_strcmp0(a[n], "autolaunch:") == 0) supported = TRUE;
      g_free(transport_name);
      g_hash_table_unref(key_value_pairs);
      if (!supported) goto out;
  }
  ret = TRUE;
out:
  g_strfreev(a);
  g_assert(ret || (!ret && (error == NULL || *error != NULL)));
  return ret;
}
gboolean _g_dbus_address_parse_entry(const gchar *address_entry, gchar **out_transport_name, GHashTable **out_key_value_pairs, GError **error) {
  gboolean ret;
  GHashTable *key_value_pairs;
  gchar *transport_name;
  gchar **kv_pairs;
  const gchar *s;
  guint n;
  ret = FALSE;
  kv_pairs = NULL;
  transport_name = NULL;
  key_value_pairs = NULL;
  s = strchr (address_entry, ':');
  if (s == NULL) {
      g_set_error (error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Address element `%s', does not contain a colon (:)"), address_entry);
      goto out;
  }
  transport_name = g_strndup (address_entry, s - address_entry);
  key_value_pairs = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  kv_pairs = g_strsplit (s + 1, ",", 0);
  for (n = 0; kv_pairs != NULL && kv_pairs[n] != NULL; n++) {
      const gchar *kv_pair = kv_pairs[n];
      gchar *key;
      gchar *value;
      s = strchr (kv_pair, '=');
      if (s == NULL) {
          g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Key/Value pair %d, `%s', in address element `%s', does not contain an equal sign"),
                      n, kv_pair, address_entry);
          goto out;
      }
      key = g_uri_unescape_segment(kv_pair, s, NULL);
      value = g_uri_unescape_segment(s + 1, kv_pair + strlen(kv_pair), NULL);
      if (key == NULL || value == NULL) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error unescaping key or value in Key/Value pair %d, `%s', in address element"
                      " `%s'"), n, kv_pair, address_entry);
          g_free(key);
          g_free(value);
          goto out;
      }
      g_hash_table_insert(key_value_pairs, key, value);
  }
  ret = TRUE;
out:
  g_strfreev(kv_pairs);
  if (ret) {
      if (out_transport_name != NULL) *out_transport_name = transport_name;
      else g_free(transport_name);
      if (out_key_value_pairs != NULL) *out_key_value_pairs = key_value_pairs;
      else if (key_value_pairs != NULL) g_hash_table_unref(key_value_pairs);
  } else {
      g_free(transport_name);
      if (key_value_pairs != NULL) g_hash_table_unref(key_value_pairs);
  }
  return ret;
}
static GIOStream *g_dbus_address_try_connect_one(const gchar *address_entry, gchar **out_guid, GCancellable *cancellable, GError **error);
static GIOStream *g_dbus_address_connect(const gchar *address_entry, const gchar *transport_name, GHashTable *key_value_pairs, GCancellable *cancellable,
                                         GError **error) {
  GIOStream *ret;
  GSocketConnectable *connectable;
  const gchar *nonce_file;
  connectable = NULL;
  ret = NULL;
  nonce_file = NULL;
  if (g_strcmp0(transport_name, "unix") == 0) {
      const gchar *path;
      const gchar *abstract;
      path = g_hash_table_lookup(key_value_pairs, "path");
      abstract = g_hash_table_lookup(key_value_pairs, "abstract");
      if ((path == NULL && abstract == NULL) || (path != NULL && abstract != NULL)) {
          g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the unix transport requires exactly one of the "
                      "keys `path' or `abstract' to be set"), address_entry);
      } else if (path != NULL) connectable = G_SOCKET_CONNECTABLE(g_unix_socket_address_new(path));
      else if (abstract != NULL) connectable = G_SOCKET_CONNECTABLE(g_unix_socket_address_new_with_type(abstract, -1, G_UNIX_SOCKET_ADDRESS_ABSTRACT));
      else { g_assert_not_reached(); }
  } else if (g_strcmp0 (transport_name, "tcp") == 0 || g_strcmp0 (transport_name, "nonce-tcp") == 0) {
      const gchar *s;
      const gchar *host;
      guint port;
      gchar *endp;
      gboolean is_nonce;
      is_nonce = (g_strcmp0(transport_name, "nonce-tcp") == 0);
      host = g_hash_table_lookup(key_value_pairs, "host");
      if (host == NULL) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the host attribute is missing or malformed"),
                      address_entry);
          goto out;
      }
      s = g_hash_table_lookup(key_value_pairs, "port");
      if (s == NULL) s = "0";
      port = strtol(s, &endp, 10);
      if ((*s == '\0' || *endp != '\0') || port < 0 || port >= 65536) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the port attribute is missing or malformed"),
                      address_entry);
          goto out;
      }
      if (is_nonce) {
          nonce_file = g_hash_table_lookup(key_value_pairs, "noncefile");
          if (nonce_file == NULL) {
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error in address `%s' - the noncefile attribute is missing or malformed"),
                          address_entry);
              goto out;
          }
      }
      connectable = g_network_address_new(host, port);
  } else if (g_strcmp0(address_entry, "autolaunch:") == 0) {
      gchar *autolaunch_address;
      autolaunch_address = get_session_address_platform_specific(error);
      if (autolaunch_address != NULL) {
          ret = g_dbus_address_try_connect_one(autolaunch_address, NULL, cancellable, error);
          g_free(autolaunch_address);
          goto out;
      } else g_prefix_error(error, _("Error auto-launching: "));
  } else {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Unknown or unsupported transport `%s' for address `%s'"), transport_name,
                  address_entry);
  }
  if (connectable != NULL) {
      GSocketClient *client;
      GSocketConnection *connection;
      g_assert(ret == NULL);
      client = g_socket_client_new();
      connection = g_socket_client_connect(client, connectable, cancellable, error);
      g_object_unref(connectable);
      g_object_unref(client);
      if (connection == NULL) goto out;
      ret = G_IO_STREAM(connection);
      if (nonce_file != NULL) {
          gchar nonce_contents[16 + 1];
          size_t num_bytes_read;
          FILE *f;
          f = fopen(nonce_file, "rb");
          if (f == NULL) {
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error opening nonce file `%s': %s"), nonce_file, g_strerror(errno));
              g_object_unref(ret);
              ret = NULL;
              goto out;
          }
          num_bytes_read = fread(nonce_contents, sizeof (gchar),16 + 1, f);
          if (num_bytes_read != 16) {
              if (num_bytes_read == 0) {
                  g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error reading from nonce file `%s': %s"), nonce_file, g_strerror(errno));
              } else {
                  g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Error reading from nonce file `%s', expected 16 bytes, got %d"),
                              nonce_file, (gint)num_bytes_read);
              }
              g_object_unref(ret);
              ret = NULL;
              fclose(f);
              goto out;
          }
          fclose(f);
          if (!g_output_stream_write_all(g_io_stream_get_output_stream(ret), nonce_contents,16,NULL, cancellable, error)) {
              g_prefix_error(error, _("Error writing contents of nonce file `%s' to stream:"), nonce_file);
              g_object_unref(ret);
              ret = NULL;
              goto out;
          }
      }
  }
out:
  return ret;
}
static GIOStream *g_dbus_address_try_connect_one(const gchar *address_entry, gchar **out_guid, GCancellable *cancellable, GError **error) {
  GIOStream *ret;
  GHashTable *key_value_pairs;
  gchar *transport_name;
  const gchar *guid;
  ret = NULL;
  transport_name = NULL;
  key_value_pairs = NULL;
  if (!_g_dbus_address_parse_entry(address_entry, &transport_name, &key_value_pairs, error)) goto out;
  ret = g_dbus_address_connect(address_entry, transport_name, key_value_pairs, cancellable, error);
  if (ret == NULL) goto out;
  guid = g_hash_table_lookup(key_value_pairs, "guid");
  if (guid != NULL && out_guid != NULL) *out_guid = g_strdup(guid);
out:
  g_free(transport_name);
  if (key_value_pairs != NULL) g_hash_table_unref(key_value_pairs);
  return ret;
}
typedef struct {
  gchar *address;
  GIOStream *stream;
  gchar *guid;
} GetStreamData;
static void get_stream_data_free(GetStreamData *data) {
  g_free (data->address);
  if (data->stream != NULL) g_object_unref(data->stream);
  g_free(data->guid);
  g_free(data);
}
static void get_stream_thread_func(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GetStreamData *data;
  GError *error;
  data = g_simple_async_result_get_op_res_gpointer(res);
  error = NULL;
  data->stream = g_dbus_address_get_stream_sync(data->address, &data->guid, cancellable, &error);
  if (data->stream == NULL) g_simple_async_result_take_error(res, error);
}
void g_dbus_address_get_stream(const gchar *address, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  GetStreamData *data;
  g_return_if_fail(address != NULL);
  res = g_simple_async_result_new(NULL, callback, user_data, g_dbus_address_get_stream);
  data = g_new0(GetStreamData, 1);
  data->address = g_strdup(address);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)get_stream_data_free);
  g_simple_async_result_run_in_thread(res, get_stream_thread_func, G_PRIORITY_DEFAULT, cancellable);
  g_object_unref(res);
}
GIOStream *g_dbus_address_get_stream_finish(GAsyncResult *res, gchar **out_guid, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  GetStreamData *data;
  GIOStream *ret;
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_dbus_address_get_stream);
  ret = NULL;
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (g_simple_async_result_propagate_error(simple, error)) goto out;
  ret = g_object_ref(data->stream);
  if (out_guid != NULL) *out_guid = g_strdup(data->guid);
out:
  return ret;
}
GIOStream *g_dbus_address_get_stream_sync(const gchar *address, gchar **out_guid, GCancellable *cancellable, GError **error) {
  GIOStream *ret;
  gchar **addr_array;
  guint n;
  GError *last_error;
  g_return_val_if_fail(address != NULL, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  ret = NULL;
  last_error = NULL;
  addr_array = g_strsplit(address, ";", 0);
  if (addr_array[0] == NULL) {
      last_error = g_error_new_literal(G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("The given address is empty"));
      goto out;
  }
  for (n = 0; addr_array != NULL && addr_array[n] != NULL; n++) {
      const gchar *addr = addr_array[n];
      GError *this_error;
      this_error = NULL;
      ret = g_dbus_address_try_connect_one(addr, out_guid, cancellable, &this_error);
      if (ret != NULL) goto out;
      else {
          g_assert(this_error != NULL);
          if (last_error != NULL) g_error_free(last_error);
          last_error = this_error;
      }
  }
out:
  if (ret != NULL) {
      if (last_error != NULL) g_error_free(last_error);
  } else {
      g_assert(last_error != NULL);
      g_propagate_error(error, last_error);
  }
  g_strfreev(addr_array);
  return ret;
}
#ifndef G_OS_UNIX
static gchar * get_session_address_dbus_launch(GError **error) {
  gchar *ret;
  gchar *machine_id;
  gchar *command_line;
  gchar *launch_stdout;
  gchar *launch_stderr;
  gint exit_status;
  gchar *old_dbus_verbose;
  gboolean restore_dbus_verbose;
  ret = NULL;
  machine_id = NULL;
  command_line = NULL;
  launch_stdout = NULL;
  launch_stderr = NULL;
  restore_dbus_verbose = FALSE;
  old_dbus_verbose = NULL;
  machine_id = _g_dbus_get_machine_id(error);
  if (machine_id == NULL) {
      g_prefix_error(error, _("Cannot spawn a message bus without a machine-id: "));
      goto out;
  }
  command_line = g_strdup_printf("dbus-launch --autolaunch=%s --binary-syntax --close-stderr", machine_id);
  if (G_UNLIKELY(_g_dbus_debug_address())) {
      _g_dbus_debug_print_lock();
      g_print("GDBus-debug:Address: Running `%s' to get bus address (possibly autolaunching)\n", command_line);
      old_dbus_verbose = g_strdup(g_getenv("DBUS_VERBOSE"));
      restore_dbus_verbose = TRUE;
      g_setenv("DBUS_VERBOSE", "1", TRUE);
      _g_dbus_debug_print_unlock();
  }
  if (!g_spawn_command_line_sync(command_line, &launch_stdout, &launch_stderr, &exit_status, error)) {
      g_prefix_error(error, _("Error spawning command line `%s': "), command_line);
      goto out;
  }
  if (!WIFEXITED(exit_status)) {
      gchar *escaped_stderr;
      escaped_stderr = g_strescape(launch_stderr, "");
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Abnormal program termination spawning command line `%s': %s"), command_line, escaped_stderr);
      g_free(escaped_stderr);
      goto out;
  }
  if (WEXITSTATUS(exit_status) != 0) {
      gchar *escaped_stderr;
      escaped_stderr = g_strescape(launch_stderr, "");
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Command line `%s' exited with non-zero exit status %d: %s"), command_line,
                  WEXITSTATUS(exit_status), escaped_stderr);
      g_free(escaped_stderr);
      goto out;
  }
  ret = g_strdup(launch_stdout);
out:
  if (G_UNLIKELY(_g_dbus_debug_address())) {
      gchar *s;
      _g_dbus_debug_print_lock();
      g_print("GDBus-debug:Address: dbus-launch output:");
      if (launch_stdout != NULL) {
          s = _g_dbus_hexdump(launch_stdout, strlen(launch_stdout) + 1 + sizeof(pid_t) + sizeof(long), 2);
          g_print("\n%s", s);
          g_free(s);
      } else g_print(" (none)\n");
      g_print("GDBus-debug:Address: dbus-launch stderr output:");
      if (launch_stderr != NULL) g_print("\n%s", launch_stderr);
      else g_print(" (none)\n");
      _g_dbus_debug_print_unlock();
  }
  g_free(machine_id);
  g_free(command_line);
  g_free(launch_stdout);
  g_free(launch_stderr);
  if (G_UNLIKELY(restore_dbus_verbose)) {
      if (old_dbus_verbose != NULL) g_setenv("DBUS_VERBOSE", old_dbus_verbose, TRUE);
      else g_unsetenv("DBUS_VERBOSE");
  }
  g_free(old_dbus_verbose);
  return ret;
}
#endif
static gchar *get_session_address_platform_specific(GError **error) {
  gchar *ret;
#ifndef G_OS_UNIX
  ret = get_session_address_dbus_launch(error);
#else
  ret = NULL;
  g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Cannot determine session bus address (not implemented for this OS)"));
#endif
  return ret;
}
gchar *g_dbus_address_get_for_bus_sync(GBusType bus_type, GCancellable *cancellable, GError **error) {
  gchar *ret;
  const gchar *starter_bus;
  GError *local_error;
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  ret = NULL;
  local_error = NULL;
  if (G_UNLIKELY(_g_dbus_debug_address())) {
      guint n;
      _g_dbus_debug_print_lock();
      g_print("GDBus-debug:Address: In g_dbus_address_get_for_bus_sync() for bus type `%s'\n", _g_dbus_enum_to_string (G_TYPE_BUS_TYPE, bus_type));
      for (n = 0; n < 3; n++) {
          const gchar *k;
          const gchar *v;
          switch(n) {
              case 0: k = "DBUS_SESSION_BUS_ADDRESS"; break;
              case 1: k = "DBUS_SYSTEM_BUS_ADDRESS"; break;
              case 2: k = "DBUS_STARTER_BUS_TYPE"; break;
              default: g_assert_not_reached();
          }
          v = g_getenv(k);
          g_print("GDBus-debug:Address: env var %s", k);
          if (v != NULL) g_print("=`%s'\n", v);
          else g_print(" is not set\n");
      }
      _g_dbus_debug_print_unlock();
  }
  switch(bus_type) {
      case G_BUS_TYPE_SYSTEM:
          ret = g_strdup(g_getenv("DBUS_SYSTEM_BUS_ADDRESS"));
          if (ret == NULL) ret = g_strdup ("unix:path=/var/run/dbus/system_bus_socket");
          break;
      case G_BUS_TYPE_SESSION:
          ret = g_strdup (g_getenv ("DBUS_SESSION_BUS_ADDRESS"));
          if (ret == NULL) ret = get_session_address_platform_specific (&local_error);
          break;
      case G_BUS_TYPE_STARTER:
          starter_bus = g_getenv ("DBUS_STARTER_BUS_TYPE");
          if (g_strcmp0 (starter_bus, "session") == 0) {
              ret = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SESSION, cancellable, &local_error);
              goto out;
          } else if (g_strcmp0(starter_bus, "system") == 0) {
              ret = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, cancellable, &local_error);
              goto out;
          } else {
              if (starter_bus != NULL) {
                  g_set_error(&local_error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Cannot determine bus address from DBUS_STARTER_BUS_TYPE environment variable"
                              " - unknown value `%s'"), starter_bus);
              } else {
                  g_set_error_literal(&local_error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Cannot determine bus address because the DBUS_STARTER_BUS_TYPE "
                                      "environment variable is not set"));
              }
          }
          break;
      default: g_set_error(&local_error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Unknown bus type %d"), bus_type); break;
  }
out:
  if (G_UNLIKELY(_g_dbus_debug_address())) {
      _g_dbus_debug_print_lock();
      if (ret != NULL) g_print("GDBus-debug:Address: Returning address `%s' for bus type `%s'\n", ret, _g_dbus_enum_to_string(G_TYPE_BUS_TYPE, bus_type));
      else g_print("GDBus-debug:Address: Cannot look-up address bus type `%s': %s\n", _g_dbus_enum_to_string(G_TYPE_BUS_TYPE, bus_type), local_error->message);
      _g_dbus_debug_print_unlock();
  }
  if (local_error != NULL) g_propagate_error (error, local_error);
  return ret;
}
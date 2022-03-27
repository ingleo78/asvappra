#include "../gio.h"

int main(int argc, char *argv[]) {
  GDBusConnection *c;
  GError *error;
  gboolean ret;
  g_type_init ();
  error = NULL;
  c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  error = NULL;
  g_dbus_connection_emit_signal(c, NULL, "/org/gtk/GDBus/FlushObject", "org.gtk.GDBus.FlushInterface", "SomeSignal", NULL, &error);
  g_assert_no_error(error);
  error = NULL;
  ret = g_dbus_connection_flush_sync(c, NULL, &error);
  g_assert_no_error(error);
  g_assert(ret);
  return 0;
}
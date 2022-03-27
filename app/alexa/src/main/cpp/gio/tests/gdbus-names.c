#include <unistd.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GMainLoop *loop;
typedef struct {
  GMainLoop *loop;
  gboolean expect_null_connection;
  guint num_bus_acquired;
  guint num_acquired;
  guint num_lost;
  guint num_free_func;
} OwnNameData;
static void own_name_data_free_func(OwnNameData *data) {
  data->num_free_func++;
  g_main_loop_quit(loop);
}
static void bus_acquired_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  OwnNameData *data = user_data;
  g_dbus_connection_set_exit_on_close(connection, FALSE);
  data->num_bus_acquired += 1;
  g_main_loop_quit(loop);
}
static void name_acquired_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  OwnNameData *data = user_data;
  data->num_acquired += 1;
  g_main_loop_quit(loop);
}
static void name_lost_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  OwnNameData *data = user_data;
  if (data->expect_null_connection) { g_assert(connection == NULL); }
  else {
      g_assert(connection != NULL);
      g_dbus_connection_set_exit_on_close(connection, FALSE);
  }
  data->num_lost += 1;
  g_main_loop_quit(loop);
}
static void test_bus_own_name(void) {
  guint id;
  guint id2;
  OwnNameData data;
  OwnNameData data2;
  const gchar *name;
  GDBusConnection *c;
  GError *error;
  gboolean name_has_owner_reply;
  GDBusConnection *c2;
  GVariant *result;
  error = NULL;
  name = "org.gtk.GDBus.Name1";
  data.num_bus_acquired = 0;
  data.num_free_func = 0;
  data.num_acquired = 0;
  data.num_lost = 0;
  data.expect_null_connection = TRUE;
  id = g_bus_own_name(G_BUS_TYPE_SESSION, name, G_BUS_NAME_OWNER_FLAGS_NONE, bus_acquired_handler, name_acquired_handler, name_lost_handler, &data,
                      (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data.num_bus_acquired, ==, 0);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_bus_acquired, ==, 0);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost,     ==, 1);
  g_bus_unown_name(id);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost,     ==, 1);
  g_assert_cmpint(data.num_free_func, ==, 1);
  session_bus_up();
  data.num_bus_acquired = 0;
  data.num_acquired = 0;
  data.num_lost = 0;
  data.expect_null_connection = FALSE;
  id = g_bus_own_name(G_BUS_TYPE_SESSION, name, G_BUS_NAME_OWNER_FLAGS_NONE, bus_acquired_handler, name_acquired_handler, name_lost_handler, &data,
                      (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data.num_bus_acquired, ==, 0);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_bus_acquired, ==, 1);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_bus_acquired, ==, 1);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_lost,     ==, 0);
  c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(c != NULL);
  g_assert(!g_dbus_connection_is_closed(c));
  result = g_dbus_connection_call_sync(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameHasOwner", g_variant_new("(s)", name),
                                       G_VARIANT_TYPE("(b)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_variant_get(result, "(b)", &name_has_owner_reply);
  g_assert(name_has_owner_reply);
  g_variant_unref(result);
  g_bus_unown_name(id);
  g_assert_cmpint(data.num_free_func, ==, 2);
  result = g_dbus_connection_call_sync(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameHasOwner", g_variant_new("(s)", name),
                                       G_VARIANT_TYPE("(b)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_variant_get(result, "(b)", &name_has_owner_reply);
  g_assert(!name_has_owner_reply);
  g_variant_unref(result);
  data.num_bus_acquired = 0;
  data.num_acquired = 0;
  data.num_lost = 0;
  data.expect_null_connection = FALSE;
  id = g_bus_own_name_with_closures(G_BUS_TYPE_SESSION, name, G_BUS_NAME_OWNER_FLAGS_NONE, g_cclosure_new(G_CALLBACK(bus_acquired_handler), &data, NULL),
                                    g_cclosure_new(G_CALLBACK(name_acquired_handler), &data, NULL), g_cclosure_new(G_CALLBACK(name_lost_handler), &data,
                                    (GClosureNotify)own_name_data_free_func));
  g_assert_cmpint(data.num_bus_acquired, ==, 0);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost, ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_bus_acquired, ==, 1);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost, ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_bus_acquired, ==, 1);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_lost,     ==, 0);
  data2.num_free_func = 0;
  data2.num_bus_acquired = 0;
  data2.num_acquired = 0;
  data2.num_lost = 0;
  data2.expect_null_connection = FALSE;
  id2 = g_bus_own_name(G_BUS_TYPE_SESSION, name, G_BUS_NAME_OWNER_FLAGS_NONE, bus_acquired_handler, name_acquired_handler, name_lost_handler, &data2,
                       (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost, ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data2.num_bus_acquired, ==, 1);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost, ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data2.num_bus_acquired, ==, 1);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost, ==, 1);
  g_bus_unown_name(id2);
  g_assert_cmpint(data2.num_bus_acquired, ==, 1);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost, ==, 1);
  g_assert_cmpint(data2.num_free_func, ==, 1);
  c2 = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(c2 != NULL);
  g_assert(!g_dbus_connection_is_closed(c2));
  data2.num_bus_acquired = 0;
  data2.num_acquired = 0;
  data2.num_lost = 0;
  data2.expect_null_connection = FALSE;
  data2.num_free_func = 0;
  id2 = g_bus_own_name_on_connection(c2, name, G_BUS_NAME_OWNER_FLAGS_NONE, name_acquired_handler, name_lost_handler, &data2,
                                     (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 1);
  g_bus_unown_name(id2);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 1);
  g_assert_cmpint(data2.num_free_func, ==, 1);
  data2.num_bus_acquired = 0;
  data2.num_acquired = 0;
  data2.num_lost = 0;
  data2.expect_null_connection = FALSE;
  data2.num_free_func = 0;
  id2 = g_bus_own_name_on_connection(c2, name, G_BUS_NAME_OWNER_FLAGS_REPLACE, name_acquired_handler, name_lost_handler, &data2,
                                     (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 1);
  g_bus_unown_name(id2);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 1);
  g_assert_cmpint(data2.num_free_func, ==, 1);
  data.expect_null_connection = FALSE;
  g_bus_unown_name(id);
  g_assert_cmpint(data.num_bus_acquired, ==, 1);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_free_func, ==, 3);
  data.num_bus_acquired = 0;
  data.num_acquired = 0;
  data.num_lost = 0;
  data.expect_null_connection = FALSE;
  id = g_bus_own_name(G_BUS_TYPE_SESSION, name, G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT, bus_acquired_handler, name_acquired_handler, name_lost_handler, &data,
                      (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data.num_bus_acquired, ==, 0);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_bus_acquired, ==, 1);
  g_assert_cmpint(data.num_acquired, ==, 0);
  g_assert_cmpint(data.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_bus_acquired, ==, 1);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_lost,     ==, 0);
  data2.num_bus_acquired = 0;
  data2.num_acquired = 0;
  data2.num_lost = 0;
  data2.expect_null_connection = FALSE;
  data2.num_free_func = 0;
  id2 = g_bus_own_name_on_connection(c2, name, G_BUS_NAME_OWNER_FLAGS_NONE, name_acquired_handler, name_lost_handler, &data2,
                                     (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 1);
  g_bus_unown_name(id2);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 1);
  g_assert_cmpint(data2.num_free_func, ==, 1);
  data2.num_bus_acquired = 0;
  data2.num_acquired = 0;
  data2.num_lost = 0;
  data2.expect_null_connection = FALSE;
  data2.num_free_func = 0;
  id2 = g_bus_own_name_on_connection(c2, name, G_BUS_NAME_OWNER_FLAGS_REPLACE, name_acquired_handler, name_lost_handler, &data2,
                                     (GDestroyNotify)own_name_data_free_func);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_lost,     ==, 0);
  g_assert_cmpint(data2.num_acquired, ==, 0);
  g_assert_cmpint(data2.num_lost,     ==, 0);
  while(data.num_lost == 0 || data2.num_acquired == 0) g_main_loop_run(loop);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_lost,     ==, 1);
  g_assert_cmpint(data2.num_acquired, ==, 1);
  g_assert_cmpint(data2.num_lost,     ==, 0);
  g_assert_cmpint(data2.num_bus_acquired, ==, 0);
  g_bus_unown_name(id2);
  g_assert_cmpint(data2.num_free_func, ==, 1);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_acquired, ==, 2);
  g_assert_cmpint(data.num_lost,     ==, 1);
  data.expect_null_connection = TRUE;
  session_bus_down();
  while(data.num_lost != 2) g_main_loop_run(loop);
  g_assert_cmpint(data.num_acquired, ==, 2);
  g_assert_cmpint(data.num_lost,     ==, 2);
  g_bus_unown_name(id);
  g_assert_cmpint(data.num_free_func, ==, 4);
  _g_object_wait_for_single_ref(c);
  g_object_unref(c);
  g_object_unref(c2);
}
typedef struct {
  gboolean expect_null_connection;
  guint num_acquired;
  guint num_lost;
  guint num_appeared;
  guint num_vanished;
  guint num_free_func;
} WatchNameData;
static void watch_name_data_free_func(WatchNameData *data) {
  data->num_free_func++;
  g_main_loop_quit(loop);
}
static void w_bus_acquired_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {}
static void w_name_acquired_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  WatchNameData *data = user_data;
  data->num_acquired += 1;
  g_main_loop_quit(loop);
}
static void w_name_lost_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  WatchNameData *data = user_data;
  data->num_lost += 1;
  g_main_loop_quit(loop);
}
static void name_appeared_handler(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data) {
  WatchNameData *data = user_data;
  if (data->expect_null_connection) { g_assert(connection == NULL); }
  else {
      g_assert(connection != NULL);
      g_dbus_connection_set_exit_on_close(connection, FALSE);
  }
  data->num_appeared += 1;
  g_main_loop_quit(loop);
}
static void name_vanished_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  WatchNameData *data = user_data;
  if (data->expect_null_connection) { g_assert(connection == NULL); }
  else {
      g_assert(connection != NULL);
      g_dbus_connection_set_exit_on_close(connection, FALSE);
  }
  data->num_vanished += 1;
  g_main_loop_quit(loop);
}
static void test_bus_watch_name(void) {
  WatchNameData data;
  guint id;
  guint owner_id;
  GDBusConnection *connection;
  data.num_free_func = 0;
  data.num_appeared = 0;
  data.num_vanished = 0;
  data.expect_null_connection = TRUE;
  id = g_bus_watch_name(G_BUS_TYPE_SESSION, "org.gtk.GDBus.Name1", G_BUS_NAME_WATCHER_FLAGS_NONE, name_appeared_handler, name_vanished_handler, &data,
                        (GDestroyNotify)watch_name_data_free_func);
  g_assert_cmpint(data.num_appeared, ==, 0);
  g_assert_cmpint(data.num_vanished, ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_appeared, ==, 0);
  g_assert_cmpint(data.num_vanished, ==, 1);
  g_bus_unwatch_name(id);
  g_assert_cmpint(data.num_appeared, ==, 0);
  g_assert_cmpint(data.num_vanished, ==, 1);
  g_assert_cmpint(data.num_free_func, ==, 1);
  session_bus_up();
  data.num_free_func = 0;
  data.num_acquired = 0;
  data.num_lost = 0;
  data.expect_null_connection = FALSE;
  owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, "org.gtk.GDBus.Name1", G_BUS_NAME_OWNER_FLAGS_NONE, w_bus_acquired_handler, w_name_acquired_handler,
                            w_name_lost_handler, &data, (GDestroyNotify)watch_name_data_free_func);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_lost,     ==, 0);
  connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(connection != NULL);
  data.num_appeared = 0;
  data.num_vanished = 0;
  id = g_bus_watch_name_on_connection(connection, "org.gtk.GDBus.Name1", G_BUS_NAME_WATCHER_FLAGS_NONE, name_appeared_handler, name_vanished_handler, &data,
                                      (GDestroyNotify)watch_name_data_free_func);
  g_assert_cmpint(data.num_appeared, ==, 0);
  g_assert_cmpint(data.num_vanished, ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_appeared, ==, 1);
  g_assert_cmpint(data.num_vanished, ==, 0);
  g_bus_unwatch_name (id);
  g_assert_cmpint(data.num_free_func, ==, 1);
  g_object_unref(connection);
  g_bus_unown_name(owner_id);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_free_func, ==, 2);
  data.num_appeared = 0;
  data.num_vanished = 0;
  data.num_free_func = 0;
  id = g_bus_watch_name_with_closures(G_BUS_TYPE_SESSION, "org.gtk.GDBus.Name1", G_BUS_NAME_WATCHER_FLAGS_NONE, g_cclosure_new(G_CALLBACK(name_appeared_handler),
                                      &data, NULL), g_cclosure_new(G_CALLBACK(name_vanished_handler), &data, (GClosureNotify)watch_name_data_free_func));
  g_assert_cmpint(data.num_appeared, ==, 0);
  g_assert_cmpint(data.num_vanished, ==, 0);
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_appeared, ==, 0);
  g_assert_cmpint(data.num_vanished, ==, 1);
  data.num_acquired = 0;
  data.num_lost = 0;
  data.expect_null_connection = FALSE;
  owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, "org.gtk.GDBus.Name1", G_BUS_NAME_OWNER_FLAGS_NONE, w_bus_acquired_handler, w_name_acquired_handler,
                            w_name_lost_handler, &data, (GDestroyNotify)watch_name_data_free_func);
  while(data.num_acquired == 0 || data.num_appeared == 0) g_main_loop_run(loop);
  g_assert_cmpint(data.num_acquired, ==, 1);
  g_assert_cmpint(data.num_lost, ==, 0);
  g_assert_cmpint(data.num_appeared, ==, 1);
  g_assert_cmpint(data.num_vanished, ==, 1);
  data.expect_null_connection = TRUE;
  session_bus_down();
  g_main_loop_run(loop);
  g_assert_cmpint(data.num_lost, ==, 1);
  g_assert_cmpint(data.num_vanished, ==, 2);
  g_bus_unwatch_name(id);
  g_assert_cmpint(data.num_free_func, ==, 1);
  g_bus_unown_name(owner_id);
  g_assert_cmpint(data.num_free_func, ==, 2);
}
static void test_validate_names(void) {
  guint n;
  static const struct {
    gboolean name;
    gboolean unique;
    gboolean interface;
    const gchar *string;
  } names[] = {
      { 1, 0, 1, "valid.well_known.name" },
      { 1, 0, 0, "valid.well-known.name" },
      { 1, 1, 0, ":valid.unique.name" },
      { 0, 0, 0, "invalid.5well_known.name" },
      { 0, 0, 0, "4invalid.5well_known.name" },
      { 1, 1, 0, ":4valid.5unique.name" },
      { 0, 0, 0, "" },
      { 1, 0, 1, "very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.name1" },
      { 0, 0, 0, "very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.very.long.name12" },
      { 0, 0, 0, ".starts.with.a.dot" },
      { 0, 0, 0, "contains.invalid;.characters" },
      { 0, 0, 0, "contains.inva/lid.characters" },
      { 0, 0, 0, "contains.inva[lid.characters" },
      { 0, 0, 0, "contains.inva]lid.characters" },
      { 0, 0, 0, "contains.inva_æøå_lid.characters" },
      { 1, 1, 0, ":1.1" },
  };
  for (n = 0; n < G_N_ELEMENTS(names); n++) {
      if (names[n].name) { g_assert(g_dbus_is_name(names[n].string)); }
      else { g_assert(!g_dbus_is_name(names[n].string)); }
      if (names[n].unique) { g_assert(g_dbus_is_unique_name(names[n].string)); }
      else { g_assert(!g_dbus_is_unique_name(names[n].string)); }
      if (names[n].interface) { g_assert(g_dbus_is_interface_name(names[n].string)); }
      else { g_assert(!g_dbus_is_interface_name(names[n].string)); }
    }
}
int main(int argc, char *argv[]) {
  gint ret;
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  loop = g_main_loop_new(NULL, FALSE);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  g_test_add_func("/gdbus/validate-names", test_validate_names);
  g_test_add_func("/gdbus/bus-own-name", test_bus_own_name);
  g_test_add_func("/gdbus/bus-watch-name", test_bus_watch_name);
  ret = g_test_run();
  g_main_loop_unref(loop);
  return ret;
}
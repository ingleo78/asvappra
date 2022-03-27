#include <unistd.h>
#include <string.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GDBusConnection *c = NULL;
static GMainLoop *loop = NULL;
typedef struct {
  GThread *thread;
  GMainLoop *thread_loop;
  guint signal_count;
} DeliveryData;
static void msg_cb_expect_success(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  DeliveryData *data = user_data;
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_connection_call_finish(connection, res, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_variant_unref(result);
  g_assert(g_thread_self () == data->thread);
  g_main_loop_quit(data->thread_loop);
}
static void msg_cb_expect_error_cancelled(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  DeliveryData *data = user_data;
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_connection_call_finish(connection, res, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_CANCELLED);
  g_assert(!g_dbus_error_is_remote_error (error));
  g_error_free(error);
  g_assert(result == NULL);
  g_assert(g_thread_self() == data->thread);
  g_main_loop_quit(data->thread_loop);
}
static void signal_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name,
                           GVariant *parameters, gpointer user_data) {
  DeliveryData *data = user_data;
  g_assert(g_thread_self() == data->thread);
  data->signal_count++;
  g_main_loop_quit(data->thread_loop);
}
static gpointer test_delivery_in_thread_func(gpointer _data) {
  GMainLoop *thread_loop;
  GMainContext *thread_context;
  DeliveryData data;
  GCancellable *ca;
  guint subscription_id;
  GDBusConnection *priv_c;
  GError *error;
  error = NULL;
  thread_context = g_main_context_new();
  thread_loop = g_main_loop_new(thread_context, FALSE);
  g_main_context_push_thread_default(thread_context);
  data.thread = g_thread_self();
  data.thread_loop = thread_loop;
  data.signal_count = 0;
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                         (GAsyncReadyCallback)msg_cb_expect_success, &data);
  g_main_loop_run(thread_loop);
  ca = g_cancellable_new();
  g_cancellable_cancel(ca);
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, ca,
                         (GAsyncReadyCallback)msg_cb_expect_error_cancelled, &data);
  g_main_loop_run(thread_loop);
  g_object_unref(ca);
  ca = g_cancellable_new();
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, ca,
                         (GAsyncReadyCallback)msg_cb_expect_error_cancelled, &data);
  g_cancellable_cancel(ca);
  g_main_loop_run(thread_loop);
  g_object_unref(ca);
  subscription_id = g_dbus_connection_signal_subscribe(c, "org.freedesktop.DBus", "org.freedesktop.DBus", "NameOwnerChanged", "/org/freedesktop/DBus", NULL,
                                                       G_DBUS_SIGNAL_FLAGS_NONE, signal_handler, &data, NULL);
  g_assert(subscription_id != 0);
  g_assert(data.signal_count == 0);
  priv_c = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(priv_c != NULL);
  g_main_loop_run(thread_loop);
  g_assert(data.signal_count == 1);
  g_object_unref(priv_c);
  g_dbus_connection_signal_unsubscribe(c, subscription_id);
  g_main_context_pop_thread_default(thread_context);
  g_main_loop_unref(thread_loop);
  g_main_context_unref(thread_context);
  g_main_loop_quit(loop);
  return NULL;
}
static void test_delivery_in_thread(void) {
  GError *error;
  GThread *thread;
  error = NULL;
  thread = g_thread_create(test_delivery_in_thread_func, NULL, TRUE,&error);
  g_assert_no_error(error);
  g_assert(thread != NULL);
  g_main_loop_run(loop);
  g_thread_join(thread);
}
typedef struct {
  GDBusProxy *proxy;
  gint msec;
  guint num;
  gboolean async;
  GMainLoop *thread_loop;
  GThread *thread;
  gboolean done;
} SyncThreadData;
static void sleep_cb(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data) {
  SyncThreadData *data = user_data;
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_proxy_call_finish(proxy, res, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_assert_cmpstr(g_variant_get_type_string(result), ==, "()");
  g_variant_unref(result);
  g_assert(data->thread == g_thread_self());
  g_main_loop_quit(data->thread_loop);
}
static gpointer test_sleep_in_thread_func(gpointer _data) {
  SyncThreadData *data = _data;
  GMainContext *thread_context;
  guint n;
  thread_context = g_main_context_new();
  data->thread_loop = g_main_loop_new(thread_context, FALSE);
  g_main_context_push_thread_default(thread_context);
  data->thread = g_thread_self();
  for (n = 0; n < data->num; n++) {
      if (data->async) {
          g_dbus_proxy_call(data->proxy, "Sleep", g_variant_new("(i)", data->msec), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback)sleep_cb, data);
          g_main_loop_run(data->thread_loop);
          g_print("A");
      } else {
          GError *error;
          GVariant *result;
          error = NULL;
          result = g_dbus_proxy_call_sync(data->proxy, "Sleep", g_variant_new("(i)", data->msec), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
          g_print("S");
          g_assert_no_error(error);
          g_assert(result != NULL);
          g_assert_cmpstr(g_variant_get_type_string(result), ==, "()");
          g_variant_unref(result);
      }
  }
  g_main_context_pop_thread_default(thread_context);
  g_main_loop_unref(data->thread_loop);
  g_main_context_unref(thread_context);
  data->done = TRUE;
  g_main_loop_quit(loop);
  return NULL;
}
static void test_method_calls_on_proxy(GDBusProxy *proxy) {
  guint n;
  for (n = 0; n < 2; n++) {
      gboolean do_async;
      GThread *thread1;
      GThread *thread2;
      GThread *thread3;
      SyncThreadData data1;
      SyncThreadData data2;
      SyncThreadData data3;
      GError *error;
      GTimeVal start_time;
      GTimeVal end_time;
      guint elapsed_msec;
      error = NULL;
      do_async = (n == 0);
      g_get_current_time (&start_time);
      data1.proxy = proxy;
      data1.msec = 40;
      data1.num = 100;
      data1.async = do_async;
      data1.done = FALSE;
      thread1 = g_thread_create(test_sleep_in_thread_func,&data1, TRUE,&error);
      g_assert_no_error(error);
      g_assert(thread1 != NULL);
      data2.proxy = proxy;
      data2.msec = 20;
      data2.num = 200;
      data2.async = do_async;
      data2.done = FALSE;
      thread2 = g_thread_create(test_sleep_in_thread_func,&data2, TRUE,&error);
      g_assert_no_error(error);
      g_assert(thread2 != NULL);
      data3.proxy = proxy;
      data3.msec = 100;
      data3.num = 40;
      data3.async = do_async;
      data3.done = FALSE;
      thread3 = g_thread_create(test_sleep_in_thread_func,&data3, TRUE,&error);
      g_assert_no_error(error);
      g_assert(thread3 != NULL);
      while(!(data1.done && data2.done && data3.done)) g_main_loop_run(loop);
      g_thread_join(thread1);
      g_thread_join(thread2);
      g_thread_join(thread3);
      g_get_current_time (&end_time);
      elapsed_msec = ((end_time.tv_sec * G_USEC_PER_SEC + end_time.tv_usec) - (start_time.tv_sec * G_USEC_PER_SEC + start_time.tv_usec)) / 1000;
      g_assert_cmpint(elapsed_msec, >=, 3950);
      g_assert_cmpint(elapsed_msec,  <, 6000);
      g_print(" ");
  }
  g_main_loop_quit (loop);
}
static void test_method_calls_in_thread(void) {
  GDBusProxy *proxy;
  GDBusConnection *connection;
  GError *error;
  gchar *name_owner;
  error = NULL;
  connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  error = NULL;
  proxy = g_dbus_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, NULL, "com.example.TestService", "/com/example/TestObject", "com.example.Frob", NULL, &error);
  g_assert_no_error(error);
  name_owner = g_dbus_proxy_get_name_owner(proxy);
  g_assert_cmpstr(name_owner, !=, NULL);
  g_free(name_owner);
  test_method_calls_on_proxy(proxy);
  g_object_unref(proxy);
  g_object_unref(connection);
}
int main(int argc, char *argv[]) {
  GError *error;
  gint ret;
  g_type_init();
  g_thread_init(NULL);
  g_test_init(&argc, &argv, NULL);
  loop = g_main_loop_new(NULL, FALSE);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  session_bus_up();
  usleep(500 * 1000);
  //g_assert(g_spawn_command_line_async(SRCDIR "/gdbus-testserver.py", NULL));
  usleep(500 * 1000);
  error = NULL;
  c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(c != NULL);
  g_test_add_func("/gdbus/delivery-in-thread", test_delivery_in_thread);
  g_test_add_func("/gdbus/method-calls-in-thread", test_method_calls_in_thread);
  ret = g_test_run();
  g_object_unref(c);
  session_bus_down();
  return ret;
}
#include <unistd.h>
#include <string.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GMainLoop *loop = NULL;
static gboolean
nuke_session_bus_cb(gpointer data) {
  g_main_loop_quit(loop);
  return FALSE;
}
static void test_exit_on_close(void) {
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
      GDBusConnection *c;
      session_bus_up();
      c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
      g_assert(c != NULL);
      g_assert(!g_dbus_connection_is_closed(c));
      g_timeout_add (50, nuke_session_bus_cb,NULL);
      g_main_loop_run(loop);
      session_bus_down();
      g_main_loop_run (loop);
  }
  g_test_trap_assert_stdout("*Remote peer vanished with error: Underlying GIOStream returned 0 bytes on an async read (g-io-error-quark, 0). Exiting.*");
  g_test_trap_assert_failed();
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  loop = g_main_loop_new(NULL, FALSE);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  g_test_add_func("/gdbus/exit-on-close", test_exit_on_close);
  return g_test_run();
}
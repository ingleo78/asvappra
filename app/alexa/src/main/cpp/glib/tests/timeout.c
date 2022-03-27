#include "../glib.h"

static GMainLoop *loop;
static gboolean stop_waiting(gpointer data) {
  g_main_loop_quit(loop);
  return FALSE;
}
static gboolean function(gpointer data) {
  g_assert_not_reached();
}
static void test_seconds(void) {
  loop = g_main_loop_new(NULL, FALSE);
  g_timeout_add(2100, stop_waiting, NULL);
  g_timeout_add_seconds(21475, function, NULL);
  g_main_loop_run(loop);
}
int main(int argc, char *argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/timeout/seconds", test_seconds);
  return g_test_run();
}
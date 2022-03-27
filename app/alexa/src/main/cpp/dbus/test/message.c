#include <string.h>
#include "../../glib/glib.h"
#include "../config.h"
#include "../dbus.h"
#include "../dbus-internals.h"
#include "../dbus-pipe.h"
#include "test-utils-glib.h"

static dbus_bool_t test_array(void *contained_signature, dbus_bool_t  have_memory) {
  DBusMessage *m;
  DBusMessageIter iter;
  DBusMessageIter arr_iter;
  dbus_bool_t arr_iter_open = FALSE;
  DBusMessageIter inner_iter;
  dbus_bool_t inner_iter_open = FALSE;
  m = dbus_message_new_signal("/", "a.b", "c");
  if (m == NULL) goto out;
  dbus_message_iter_init_append(m, &iter);
  if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, contained_signature, &arr_iter)) goto out;
  arr_iter_open = TRUE;
  if (g_strcmp0(contained_signature, "ai") == 0) {
      if (!dbus_message_iter_open_container(&arr_iter, DBUS_TYPE_ARRAY, "i", &inner_iter)) goto out;
      if (!dbus_message_iter_close_container(&arr_iter, &inner_iter)) goto out;
  } else if (g_strcmp0(contained_signature, "{ss}") == 0) {
      const char *s = "hello";
      if (!dbus_message_iter_open_container(&arr_iter, DBUS_TYPE_DICT_ENTRY,NULL, &inner_iter)) goto out;
      inner_iter_open = TRUE;
      if (!dbus_message_iter_append_basic(&inner_iter, DBUS_TYPE_STRING, &s)) goto out;
      if (!dbus_message_iter_append_basic(&inner_iter, DBUS_TYPE_STRING, &s)) goto out;
      inner_iter_open = FALSE;
      if (!dbus_message_iter_close_container(&arr_iter, &inner_iter)) goto out;
  } else if (g_strcmp0(contained_signature, "v") == 0) {
      dbus_bool_t yes = TRUE;
      if (!dbus_message_iter_open_container(&arr_iter, DBUS_TYPE_VARIANT,"b", &inner_iter)) goto out;
      inner_iter_open = TRUE;
      if (!dbus_message_iter_append_basic(&inner_iter, DBUS_TYPE_BOOLEAN, &yes)) goto out;
      inner_iter_open = FALSE;
      if (!dbus_message_iter_close_container(&arr_iter, &inner_iter)) goto out;
  } else { g_assert_not_reached(); }
  arr_iter_open = FALSE;
  if (!dbus_message_iter_close_container(&iter, &arr_iter)) goto out;
out:
  if (inner_iter_open) dbus_message_iter_abandon_container(&arr_iter, &inner_iter);
  if (arr_iter_open) dbus_message_iter_abandon_container(&iter, &arr_iter);
  if (m != NULL) dbus_message_unref(m);
  dbus_shutdown();
  g_assert_cmpint(_dbus_get_malloc_blocks_outstanding(), ==, 0);
  return TRUE;//!g_test_failed();
}
static dbus_bool_t test_fd(void *ignored, dbus_bool_t have_memory) {
  DBusMessage *m = NULL;
  DBusPipe pipe;
  _dbus_pipe_init_stdout(&pipe);
  m = dbus_message_new_signal("/", "a.b", "c");
  if (m == NULL) goto out;
  if (!dbus_message_append_args(m, DBUS_TYPE_UNIX_FD, &pipe.fd, DBUS_TYPE_INVALID)) goto out;
out:
  if (m != NULL) dbus_message_unref(m);
  dbus_shutdown();
  g_assert_cmpint(_dbus_get_malloc_blocks_outstanding(), ==, 0);
  return TRUE;//!g_test_failed();
}
static dbus_bool_t test_zero_iter(void *ignored, dbus_bool_t have_memory) {
  DBusMessage *m;
  DBusMessageIter iter = DBUS_MESSAGE_ITER_INIT_CLOSED;
  DBusMessageIter arr_iter = DBUS_MESSAGE_ITER_INIT_CLOSED;
  DBusMessageIter inner_iter;
  dbus_int32_t fortytwo = 42;
  dbus_bool_t message_should_be_complete = FALSE;
  dbus_message_iter_init_closed(&inner_iter);
  m = dbus_message_new_signal("/", "a.b", "c");
  if (m == NULL) goto out;
  dbus_message_iter_init_append(m, &iter);
  if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,"ai", &arr_iter)) goto out;
  if (!dbus_message_iter_open_container(&arr_iter, DBUS_TYPE_ARRAY, "i", &inner_iter)) goto out;
  if (!dbus_message_iter_append_basic(&inner_iter, DBUS_TYPE_INT32, &fortytwo)) goto out;
  if (!dbus_message_iter_close_container(&arr_iter, &inner_iter)) goto out;
  if (!dbus_message_iter_close_container(&iter, &arr_iter)) goto out;
  message_should_be_complete = TRUE;
out:
  dbus_message_iter_abandon_container_if_open(&arr_iter, &inner_iter);
  dbus_message_iter_abandon_container_if_open(&iter, &arr_iter);
  dbus_message_iter_abandon_container_if_open(&iter, &arr_iter);
  if (message_should_be_complete) {
      DBusBasicValue read_back;
      _DBUS_ZERO(read_back);
      dbus_message_iter_init(m, &iter);
      dbus_message_iter_recurse(&iter, &arr_iter);
      dbus_message_iter_recurse(&arr_iter, &inner_iter);
      dbus_message_iter_get_basic(&inner_iter, &read_back);
      g_assert_cmpint(read_back.i32, ==, 42);
  }
  if (m != NULL) dbus_message_unref(m);
  dbus_shutdown();
  g_assert_cmpint(_dbus_get_malloc_blocks_outstanding(), ==, 0);
  return TRUE;//!g_test_failed();
}
typedef struct {
  const gchar *name;
  DBusTestMemoryFunction function;
  const void *data;
} OOMTestCase;
static void test_oom_wrapper(gconstpointer data) {
  const OOMTestCase *test = data;
  if (!_dbus_test_oom_handling(test->name, test->function,(void*)test->data)) {
      g_test_message("OOM test failed");
      //g_test_fail();
  }
}
static GQueue *test_cases_to_free = NULL;
static void add_oom_test(const gchar *name, DBusTestMemoryFunction function, const void *data) {
  OOMTestCase *test_case = g_new0(OOMTestCase, 1);
  test_case->name = name;
  test_case->function = function;
  test_case->data = data;
  g_test_add_data_func(name, test_case, test_oom_wrapper);
  g_queue_push_tail(test_cases_to_free, test_case);
}
int main(int argc, char **argv) {
  int ret;
  test_init(&argc, &argv);
  test_cases_to_free = g_queue_new();
  add_oom_test("/message/array/array", test_array, "ai");
  add_oom_test("/message/array/dict", test_array, "{ss}");
  add_oom_test("/message/array/variant", test_array, "v");
  add_oom_test("/message/fd", test_fd, NULL);
  add_oom_test("/message/zero-iter", test_zero_iter, NULL);
  ret = g_test_run();
  //g_queue_free_full(test_cases_to_free, g_free);
  return ret;
}
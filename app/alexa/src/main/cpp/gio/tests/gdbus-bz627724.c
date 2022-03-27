#include <unistd.h>
#include <string.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GDBusConnection *the_connection = NULL;
#define MY_TYPE_OBJECT  (my_object_get_type ())
#define MY_OBJECT(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), MY_TYPE_OBJECT, MyObject))
#define MY_IS_OBJECT(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), MY_TYPE_OBJECT))
typedef struct {
  GObject parent_instance;
} MyObject;
typedef struct {
  GObjectClass parent_class;
} MyObjectClass;
GType my_object_get_type(void) G_GNUC_CONST;
G_DEFINE_TYPE(MyObject, my_object, G_TYPE_OBJECT);
static void my_object_init(MyObject *object) {}
static void my_object_class_init(MyObjectClass *klass) {
  GError *error;
  error = NULL;
  the_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(G_IS_DBUS_CONNECTION(the_connection));
}
static void test_bz627724(void) {
  MyObject *object;
  session_bus_up();
  g_assert(the_connection == NULL);
  object = g_object_new(MY_TYPE_OBJECT, NULL);
  g_assert(the_connection != NULL);
  g_object_unref(the_connection);
  g_object_unref(object);
  session_bus_down();
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  g_test_add_func("/gdbus/bz627724", test_bz627724);
  return g_test_run();
}
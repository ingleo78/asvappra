#ifndef TEST_UTILS_GLIB_H
#define TEST_UTILS_GLIB_H

#include "../dbus/dbus.h"
#include "test-utils.h"

typedef enum {
    TEST_USER_ME,
    TEST_USER_ROOT,
    TEST_USER_MESSAGEBUS,
    TEST_USER_OTHER
} TestUser;
#define test_assert_no_error(e) _test_assert_no_error(e, __FILE__, __LINE__)
void _test_assert_no_error(const DBusError *e, const char *file, int line);
gchar *test_get_dbus_daemon(const gchar *config_file, TestUser user, const gchar *runtime_dir, GPid *daemon_pid);
DBusConnection *test_connect_to_bus(TestMainContext *ctx, const gchar *address);
DBusConnection *test_try_connect_to_bus(TestMainContext *ctx, const gchar *address, GError **error);
DBusConnection *test_try_connect_to_bus_as_user(TestMainContext *ctx, const char *address, TestUser user, GError **error);
GDBusConnection *test_try_connect_gdbus_as_user(const char *address, TestUser user, GError **error);
void test_kill_pid(GPid pid);
void test_init(int *argcp, char ***argvp);
void test_progress(char symbol);
void test_remove_if_exists(const gchar *path);
void test_rmdir_must_exist(const gchar *path);
void test_rmdir_if_exists(const gchar *path);
void test_mkdir(const gchar *path, gint mode);
void test_timeout_reset(guint factor);
void test_oom(void) _DBUS_GNUC_NORETURN;
DBusMessage *test_main_context_call_and_wait(TestMainContext *ctx, DBusConnection *connection, DBusMessage *call, int timeout);
#if GLIB_CHECK_VERSION(2, 44, 0)
static inline gpointer backported_g_steal_pointer(gpointer pointer_to_pointer) {
  gpointer *pp = &pointer_to_pointer;
  gpointer ret;
  ret = *pp;
  *pp = NULL;
  return ret;
}
#define g_steal_pointer(x) backported_g_steal_pointer(x)
#endif

#endif
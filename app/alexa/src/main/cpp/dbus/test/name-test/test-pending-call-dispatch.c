#include <stdio.h>
#include <stdlib.h>
#include "../../config.h"
#include "../../dbus.h"
#include "../../dbus-sysdeps.h"

static void _run_iteration(DBusConnection *conn) {
  DBusPendingCall *echo_pending;
  DBusPendingCall *dbus_pending;
  DBusMessage *method;
  DBusMessage *reply;
  const char *echo = "echo";
  method = dbus_message_new_method_call("org.freedesktop.DBus.TestSuiteEchoService","/org/freedesktop/TestSuite","org.freedesktop.TestSuite",
                                        "Echo");
  if (!dbus_message_append_args(method, DBUS_TYPE_STRING, &echo, NULL)) {
      fprintf(stderr, "Bail out! Failed to append arguments: OOM\n");
      exit(1);
  }
  dbus_connection_send_with_reply(conn, method, &echo_pending, -1);
  dbus_message_unref(method);
  method = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,"org.freedesktop.Introspectable","Introspect");
  dbus_connection_send_with_reply(conn, method, &dbus_pending, -1);
  dbus_message_unref(method);
  dbus_pending_call_block(dbus_pending);
  dbus_pending_call_block(echo_pending);
  reply = dbus_pending_call_steal_reply(echo_pending);
  if (reply == NULL) {
      printf("Bail out! Reply is NULL ***\n");
      exit(1);
  }
  if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
      printf("Bail out! Reply is error: %s ***\n", dbus_message_get_error_name(reply));
      exit(1);
  }
  dbus_message_unref(reply);
  dbus_pending_call_unref(dbus_pending);
  dbus_pending_call_unref(echo_pending);
}
int main(int argc, char *argv[]) {
  long start_tv_sec, start_tv_usec;
  long end_tv_sec, end_tv_usec;
  int i;
  DBusMessage *method;
  DBusConnection *conn;
  DBusError error;
  printf ("# Testing stuck in poll\n");
  dbus_error_init(&error);
  conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
  for (i = 0; i < 100; i++) {
      long delta;
      _dbus_get_monotonic_time(&start_tv_sec, &start_tv_usec);
      _run_iteration(conn);
      _dbus_get_monotonic_time(&end_tv_sec, &end_tv_usec);
      delta = end_tv_sec - start_tv_sec;
      printf("ok %d - %lis\n", i + 1, delta);
      if (delta >= 5) {
          printf("Bail out! Looks like we might have been be stuck in poll ***\n");
          exit(1);
	  }
  }
  method = dbus_message_new_method_call("org.freedesktop.TestSuiteEchoService","/org/freedesktop/TestSuite","org.freedesktop.TestSuite",
                                        "Exit");
  dbus_connection_send(conn, method, NULL);
  dbus_message_unref(method);
  printf("# Testing completed\n1..%d\n", i);
  exit(0);
}
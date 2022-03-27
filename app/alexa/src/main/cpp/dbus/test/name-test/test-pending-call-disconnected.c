#include <stdio.h>
#include <stdlib.h>
#include "../../config.h"
#include "../../dbus.h"
#include "../../dbus-sysdeps.h"

static size_t count = 0;
static void free_data(void *data) {
  --count;
  printf("# Freed: %s\n", (const char*)data);
}
int main(int argc, char *argv[]) {
  dbus_int32_t slot_connection = -1;
  dbus_int32_t slot_message = -1;
  dbus_int32_t slot_pending = -1;
  DBusError error;
  DBusConnection *conn;
  DBusMessage *method;
  DBusPendingCall *pending;
  DBusMessage *reply;
  printf("# Testing pending call error\n");
  dbus_connection_allocate_data_slot(&slot_connection);
  dbus_message_allocate_data_slot(&slot_message);
  dbus_pending_call_allocate_data_slot(&slot_pending);
  dbus_error_init(&error);
  conn = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
  dbus_connection_set_data(conn, slot_connection, (void*)"connection", free_data);
  ++count;
  dbus_connection_set_exit_on_disconnect(conn, FALSE);
  method = dbus_message_new_method_call("org.freedesktop.TestSuiteEchoService","/org/freedesktop/TestSuite","org.freedesktop.TestSuite",
                                        "Exit");
  dbus_message_set_data(method, slot_message, (void*)"method", free_data);
  ++count;
  dbus_connection_send_with_reply(conn, method, &pending, -1);
  dbus_message_unref(method);
  dbus_pending_call_set_data(pending, slot_pending, (void*)"pending", free_data);
  ++count;
  dbus_connection_close(conn);
  dbus_pending_call_block(pending);
  reply = dbus_pending_call_steal_reply(pending);
  dbus_pending_call_unref(pending);
  if (reply == NULL) {
      printf("Bail out! Reply is NULL ***\n");
      exit(1);
  }
  dbus_message_set_data(reply, slot_message, (void*)"reply", free_data);
  ++count;
  if (dbus_message_get_type(reply) != DBUS_MESSAGE_TYPE_ERROR) {
      printf("Bail out! Reply is not error ***\n");
      exit(1);
  }
  dbus_message_unref(reply);
  dbus_connection_unref(conn);
  dbus_connection_free_data_slot(&slot_connection);
  dbus_message_free_data_slot(&slot_message);
  dbus_pending_call_free_data_slot(&slot_pending);
  if (count != 0) {
      printf("not ok # Not all refs were unrefed ***\n");
      exit(1);
  } else {
      printf("ok\n# Testing completed\n1..1\n");
      exit(0);
  }
}

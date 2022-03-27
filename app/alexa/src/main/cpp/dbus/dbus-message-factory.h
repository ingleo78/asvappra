#ifndef DBUS_MESSAGE_FACTORY_H
#define DBUS_MESSAGE_FACTORY_H

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus-string.h"
#include "dbus-marshal-basic.h"
#include "dbus-marshal-validate.h"

DBUS_BEGIN_DECLS
typedef struct {
  DBusValidity expected_validity;
  DBusString data;
} DBusMessageData;
#define _DBUS_MESSAGE_DATA_MAX_NESTING 10
typedef struct {
  int sequence_nos[_DBUS_MESSAGE_DATA_MAX_NESTING];
  int depth;
  int count;
} DBusMessageDataIter;
void _dbus_message_data_free(DBusMessageData *data);
void _dbus_message_data_iter_init(DBusMessageDataIter *iter);
dbus_bool_t _dbus_message_data_iter_get_and_next(DBusMessageDataIter *iter, DBusMessageData *data);
DBUS_END_DECLS
#endif
#endif
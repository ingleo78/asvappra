#ifndef DBUS_MESSAGE_PRIVATE_H
#define DBUS_MESSAGE_PRIVATE_H

#include "dbus-message.h"
#include "dbus-message-internal.h"
#include "dbus-string.h"
#include "dbus-dataslot.h"
#include "dbus-marshal-header.h"

DBUS_BEGIN_DECLS
struct DBusMessageLoader {
  int refcount;
  DBusString data;
  DBusList *messages;
  long max_message_size;
  long max_message_unix_fds;
  DBusValidity corruption_reason;
  unsigned int corrupted : 1;
  unsigned int buffer_outstanding : 1;
#ifdef HAVE_UNIX_FD_PASSING
  unsigned int unix_fds_outstanding : 1;
  int *unix_fds;
  unsigned n_unix_fds_allocated;
  unsigned n_unix_fds;
  void (*unix_fds_change)(void*);
  void *unix_fds_change_data;
#endif
};
#define CHANGED_STAMP_BITS 21
struct DBusMessage {
  DBusAtomic refcount;
  DBusHeader header;
  DBusString body;
  unsigned int locked : 1;
#ifdef DBUS_DISABLE_CHECKS
  unsigned int in_cache : 1;
#endif
  DBusList *counters;
  long size_counter_delta;
  dbus_uint32_t changed_stamp : CHANGED_STAMP_BITS;
  DBusDataSlotList slot_list;
#ifdef DBUS_DISABLE_CHECKS
  int generation;
#endif
#ifdef HAVE_UNIX_FD_PASSING
  int *unix_fds;
  unsigned n_unix_fds;
  unsigned n_unix_fds_allocated;
  long unix_fd_counter_delta;
#endif
};
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_message_iter_get_args_valist(DBusMessageIter *iter, DBusError *error, int first_arg_type, va_list var_args);
DBUS_END_DECLS

#endif
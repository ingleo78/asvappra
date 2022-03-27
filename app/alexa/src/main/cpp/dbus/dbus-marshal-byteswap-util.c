#include "config.h"

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include "dbus-marshal-byteswap.h"
#include "dbus-test.h"
#include "dbus-test-tap.h"

static void do_byteswap_test(int byte_order) {
  int sequence;
  DBusString signature;
  DBusString body;
  int opposite_order;
  if (!_dbus_string_init(&signature) || !_dbus_string_init(&body)) _dbus_test_fatal("oom");
  opposite_order = byte_order == DBUS_LITTLE_ENDIAN ? DBUS_BIG_ENDIAN : DBUS_LITTLE_ENDIAN;
  sequence = 0;
  while(dbus_internal_do_not_use_generate_bodies(sequence, byte_order, &signature, &body)) {
      DBusString copy;
      DBusTypeReader body_reader;
      DBusTypeReader copy_reader;
      if (!_dbus_string_init(&copy)) _dbus_test_fatal("oom");
      if (!_dbus_string_copy(&body, 0, &copy, 0)) _dbus_test_fatal("oom");
      _dbus_marshal_byteswap(&signature, 0, byte_order, opposite_order, &copy, 0);
      _dbus_type_reader_init(&body_reader, byte_order, &signature, 0, &body, 0);
      _dbus_type_reader_init(&copy_reader, opposite_order, &signature, 0, &copy, 0);
      if (!_dbus_type_reader_equal_values(&body_reader, &copy_reader)) {
          _dbus_verbose_bytes_of_string(&signature, 0, _dbus_string_get_length(&signature));
          _dbus_verbose_bytes_of_string(&body, 0, _dbus_string_get_length(&body));
          _dbus_verbose_bytes_of_string(&copy, 0, _dbus_string_get_length(&copy));
          _dbus_test_fatal("Byte-swapped data did not have same values as original data");
      }
      _dbus_string_free(&copy);
      _dbus_string_set_length(&signature, 0);
      _dbus_string_set_length(&body, 0);
      ++sequence;
  }
  _dbus_string_free(&signature);
  _dbus_string_free(&body);
  _dbus_test_diag("  %d blocks swapped from order '%c' to '%c'", sequence, byte_order, opposite_order);
}
dbus_bool_t _dbus_marshal_byteswap_test(void) {
  do_byteswap_test(DBUS_LITTLE_ENDIAN);
  do_byteswap_test(DBUS_BIG_ENDIAN);
  return TRUE;
}
#endif
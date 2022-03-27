#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_SIGNATURES_H
#define DBUS_SIGNATURES_H

#include "dbus-macros.h"
#include "dbus-types.h"
#include "dbus-errors.h"

DBUS_BEGIN_DECLS
typedef struct {
  void *dummy1;
  void *dummy2;
  dbus_uint32_t dummy8;
  int dummy12;
  int dummy17;
} DBusSignatureIter;
DBUS_EXPORT void dbus_signature_iter_init(DBusSignatureIter *iter, const char *signature);
DBUS_EXPORT int dbus_signature_iter_get_current_type(const DBusSignatureIter *iter);
DBUS_EXPORT char *dbus_signature_iter_get_signature(const DBusSignatureIter *iter);
DBUS_EXPORT int dbus_signature_iter_get_element_type(const DBusSignatureIter *iter);
DBUS_EXPORT dbus_bool_t dbus_signature_iter_next(DBusSignatureIter *iter);
DBUS_EXPORT void dbus_signature_iter_recurse(const DBusSignatureIter *iter, DBusSignatureIter *subiter);
DBUS_EXPORT dbus_bool_t dbus_signature_validate(const char *signature, DBusError *error);
DBUS_EXPORT dbus_bool_t dbus_signature_validate_single(const char *signature, DBusError *error);
DBUS_EXPORT dbus_bool_t dbus_type_is_valid(int typecode);
DBUS_EXPORT dbus_bool_t dbus_type_is_basic(int typecode);
DBUS_EXPORT dbus_bool_t dbus_type_is_container(int typecode);
DBUS_EXPORT dbus_bool_t dbus_type_is_fixed(int typecode);
DBUS_END_DECLS

#endif
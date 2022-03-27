#ifndef BUS_POLICY_H
#define BUS_POLICY_H

#include "../dbus.h"
#include "../dbus-string.h"
#include "../dbus-list.h"
#include "../dbus-sysdeps.h"
#include "bus.h"

typedef enum {
  BUS_POLICY_RULE_SEND,
  BUS_POLICY_RULE_RECEIVE,
  BUS_POLICY_RULE_OWN,
  BUS_POLICY_RULE_USER,
  BUS_POLICY_RULE_GROUP
} BusPolicyRuleType;
typedef enum {
  BUS_POLICY_TRISTATE_ANY = 0,
  BUS_POLICY_TRISTATE_FALSE,
  BUS_POLICY_TRISTATE_TRUE
} BusPolicyTristate;
#define BUS_POLICY_RULE_IS_PER_CLIENT(rule)  (!((rule)->type == BUS_POLICY_RULE_USER || (rule)->type == BUS_POLICY_RULE_GROUP))
struct BusPolicyRule {
  int refcount;
  BusPolicyRuleType type;
  unsigned int allow : 1;
  union {
      struct {
          int message_type;
          char *path;
          char *interface;
          char *member;
          char *error;
          char *destination;
          unsigned int max_fds;
          unsigned int min_fds;
          unsigned int eavesdrop : 1;
          unsigned int requested_reply : 1;
          unsigned int log : 1;
          unsigned int broadcast : 2;
      } send;
      struct {
          int message_type;
          char *path;
          char *interface;
          char *member;
          char *error;
          char *origin;
          unsigned int max_fds;
          unsigned int min_fds;
          unsigned int eavesdrop : 1;
          unsigned int requested_reply : 1;
      } receive;
      struct {
          char *service_name;
          unsigned int prefix : 1;
      } own;
      struct {
          dbus_uid_t uid;
      } user;
      struct {
          dbus_gid_t gid;
      } group;
  } d;
};
BusPolicyRule* bus_policy_rule_new(BusPolicyRuleType type, dbus_bool_t allow);
BusPolicyRule* bus_policy_rule_ref(BusPolicyRule *rule);
void bus_policy_rule_unref(BusPolicyRule *rule);
BusPolicy* bus_policy_new(void);
BusPolicy* bus_policy_ref(BusPolicy *policy);
void bus_policy_unref(BusPolicy *policy);
BusClientPolicy* bus_policy_create_client_policy(BusPolicy *policy, DBusConnection *connection, DBusError *error);
dbus_bool_t bus_policy_allow_unix_user(BusPolicy *policy, unsigned long uid);
dbus_bool_t bus_policy_allow_windows_user(BusPolicy *policy, const char *windows_sid);
dbus_bool_t bus_policy_append_default_rule(BusPolicy *policy, BusPolicyRule *rule);
dbus_bool_t bus_policy_append_mandatory_rule(BusPolicy *policy, BusPolicyRule *rule);
dbus_bool_t bus_policy_append_user_rule(BusPolicy *policy, dbus_uid_t uid, BusPolicyRule *rule);
dbus_bool_t bus_policy_append_group_rule(BusPolicy *policy, dbus_gid_t gid, BusPolicyRule *rule);
dbus_bool_t bus_policy_append_console_rule(BusPolicy *policy, dbus_bool_t at_console, BusPolicyRule *rule);
dbus_bool_t bus_policy_merge(BusPolicy *policy, BusPolicy *to_absorb);
BusClientPolicy* bus_client_policy_new(void);
BusClientPolicy* bus_client_policy_ref(BusClientPolicy *policy);
void bus_client_policy_unref(BusClientPolicy *policy);
dbus_bool_t bus_client_policy_check_can_send(BusClientPolicy *policy, BusRegistry *registry, dbus_bool_t requested_reply, DBusConnection *receiver,
                                             DBusMessage *message, dbus_int32_t *toggles, dbus_bool_t *log);
dbus_bool_t bus_client_policy_check_can_receive(BusClientPolicy *policy, BusRegistry *registry, dbus_bool_t requested_reply, DBusConnection *sender,
                                                DBusConnection *addressed_recipient, DBusConnection *proposed_recipient, DBusMessage *message,
                                                dbus_int32_t *toggles);
dbus_bool_t bus_client_policy_check_can_own(BusClientPolicy  *policy, const DBusString *service_name);
dbus_bool_t bus_client_policy_append_rule(BusClientPolicy *policy, BusPolicyRule *rule);
void bus_client_policy_optimize (BusClientPolicy *policy);
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
dbus_bool_t bus_policy_check_can_own(BusPolicy *policy, const DBusString *service_name);
#endif

#endif
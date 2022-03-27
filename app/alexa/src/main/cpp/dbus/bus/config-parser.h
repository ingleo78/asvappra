#ifndef BUS_CONFIG_PARSER_H
#define BUS_CONFIG_PARSER_H

#include "../dbus.h"
#include "../dbus-string.h"
#include "../dbus-list.h"
#include "../dbus-hash.h"
#include "bus.h"

typedef struct BusConfigParser BusConfigParser;
BusConfigParser* bus_config_parser_new(const DBusString *basedir, dbus_bool_t is_toplevel, const BusConfigParser *parent);
BusConfigParser* bus_config_parser_ref(BusConfigParser *parser);
void bus_config_parser_unref(BusConfigParser *parser);
dbus_bool_t bus_config_parser_check_doctype(BusConfigParser *parser, const char *doctype, DBusError *error);
dbus_bool_t bus_config_parser_start_element(BusConfigParser *parser, const char *element_name, const char **attribute_names, const char **attribute_values,
                                            DBusError *error);
dbus_bool_t bus_config_parser_end_element(BusConfigParser *parser, const char *element_name, DBusError *error);
dbus_bool_t bus_config_parser_content(BusConfigParser *parser, const DBusString *content, DBusError *error);
dbus_bool_t bus_config_parser_finished(BusConfigParser *parser, DBusError *error);
const char* bus_config_parser_get_user(BusConfigParser *parser);
const char* bus_config_parser_get_type(BusConfigParser *parser);
DBusList** bus_config_parser_get_addresses(BusConfigParser *parser);
DBusList** bus_config_parser_get_mechanisms(BusConfigParser *parser);
dbus_bool_t bus_config_parser_get_fork(BusConfigParser *parser);
dbus_bool_t bus_config_parser_get_allow_anonymous(BusConfigParser *parser);
dbus_bool_t bus_config_parser_get_syslog(BusConfigParser *parser);
dbus_bool_t bus_config_parser_get_keep_umask(BusConfigParser *parser);
const char* bus_config_parser_get_pidfile(BusConfigParser *parser);
const char* bus_config_parser_get_servicehelper(BusConfigParser *parser);
DBusList** bus_config_parser_get_service_dirs(BusConfigParser *parser);
DBusList** bus_config_parser_get_conf_dirs(BusConfigParser *parser);
BusPolicy* bus_config_parser_steal_policy(BusConfigParser *parser);
void bus_config_parser_get_limits(BusConfigParser *parser, BusLimits *limits);
dbus_bool_t bus_config_parser_get_watched_dirs(BusConfigParser *parser, DBusList **watched_dirs);
DBusHashTable* bus_config_parser_steal_service_context_table(BusConfigParser *parser);
BusConfigParser* bus_config_load(const DBusString *file, dbus_bool_t is_toplevel, const BusConfigParser *parent, DBusError *error);
typedef enum {
  BUS_SERVICE_DIR_FLAGS_NO_WATCH = (1 << 0),
  BUS_SERVICE_DIR_FLAGS_STRICT_NAMING = (1 << 1),
  BUS_SERVICE_DIR_FLAGS_NONE = 0
} BusServiceDirFlags;
typedef struct {
  BusServiceDirFlags flags;
  char *path;
} BusConfigServiceDir;

#endif
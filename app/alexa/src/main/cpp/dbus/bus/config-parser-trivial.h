#ifndef BUS_CONFIG_PARSER_TRIVIAL_H
#define BUS_CONFIG_PARSER_TRIVIAL_H

#include "../dbus.h"
#include "../dbus-string.h"
#include "../dbus-list.h"
#include "../dbus-hash.h"

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
DBusList** bus_config_parser_get_service_paths(BusConfigParser *parser);
BusConfigParser* bus_config_load(const DBusString *file, dbus_bool_t is_toplevel, const BusConfigParser *parent, DBusError *error);

#endif
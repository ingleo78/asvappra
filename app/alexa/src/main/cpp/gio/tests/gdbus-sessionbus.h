#ifndef __SESSION_BUS_H__
#define __SESSION_BUS_H__

#include "../../glib/glib.h"
#include "../gio.h"

G_BEGIN_DECLS
const gchar *session_bus_up_with_address(const gchar *given_address);
void session_bus_down_with_address(const gchar *address);
const gchar *session_bus_get_temporary_address(void);
const gchar *session_bus_up(void);
void session_bus_down(void);
G_END_DECLS

#endif
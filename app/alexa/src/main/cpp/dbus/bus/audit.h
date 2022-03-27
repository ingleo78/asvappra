#ifndef BUS_AUDIT_H
#define BUS_AUDIT_H

#include "../dbus.h"
#include "bus.h"

void bus_audit_init (BusContext *context);
int bus_audit_get_fd (void);
void bus_audit_shutdown (void);

#endif
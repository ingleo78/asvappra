#ifndef BUS_EXPIRE_LIST_H
#define BUS_EXPIRE_LIST_H

#include "../dbus.h"
#include "../dbus-list.h"
#include "../dbus-mainloop.h"

typedef struct BusExpireList BusExpireList;
typedef struct BusExpireItem BusExpireItem;
typedef dbus_bool_t (* BusExpireFunc) (BusExpireList *list, DBusList *link, void *data);
struct BusExpireItem {
  long added_tv_sec;
  long added_tv_usec;
};
BusExpireList* bus_expire_list_new(DBusLoop *loop, int expire_after, BusExpireFunc expire_func, void *data);
void bus_expire_list_free(BusExpireList *list);
void bus_expire_list_recheck_immediately(BusExpireList *list);
void bus_expire_list_remove_link(BusExpireList *list, DBusList *link);
dbus_bool_t bus_expire_list_remove(BusExpireList *list, BusExpireItem *item);
DBusList* bus_expire_list_get_first_link(BusExpireList *list);
DBusList* bus_expire_list_get_next_link(BusExpireList *list, DBusList *link);
dbus_bool_t bus_expire_list_add(BusExpireList *list, BusExpireItem *item);
void bus_expire_list_add_link(BusExpireList *list, DBusList *link);
dbus_bool_t bus_expire_list_contains_item(BusExpireList *list, BusExpireItem *item);
void bus_expire_list_unlink(BusExpireList *list, DBusList *link);
#define ELAPSED_MILLISECONDS_SINCE(orig_tv_sec, orig_tv_usec, now_tv_sec, now_tv_usec)     \
 (((double)(now_tv_sec) - (double)(orig_tv_sec)) * 1000.0 + ((double)(now_tv_usec) - (double)(orig_tv_usec)) / 1000.0)
void bus_expire_timeout_set_interval(DBusTimeout *timeout, int next_interval);

#endif
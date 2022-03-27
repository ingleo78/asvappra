#include "config.h"

extern "C" {
#include "dbus-sysdeps-win.h"
}
class DBusInternalInit {
public:
  DBusInternalInit() {
      _dbus_threads_windows_init_global();
  }
  void must_not_be_omitted() {}
};
static DBusInternalInit init;
extern "C" void _dbus_threads_windows_ensure_ctor_linked() {
  init.must_not_be_omitted();
}
#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSPROXY_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSPROXY_H_

#include <memory>
#include <string>
#include <gio/gunixfdlist.h>
#include "BlueZUtils.h"


namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            class DBusProxy {
            public:
                virtual ~DBusProxy();
                static shared_ptr<DBusProxy> create(const string& interfaceName, const string& objectPath);
                virtual ManagedGVariant callMethod(const string& methodName, GVariant* parameters = nullptr, GError** error = nullptr);
                virtual ManagedGVariant callMethodWithFDList(const string& methodName, GVariant* parameters = nullptr, GUnixFDList** outlist = nullptr,
                                                             GError** error = nullptr);
                virtual string getObjectPath() const;
                virtual GDBusProxy* get();
            protected:
                DBusProxy(GDBusProxy* proxy, const string& objectPath);
            private:
                GDBusProxy* m_proxy;
                const string m_objectPath;
            };
        }
    }
}
#endif
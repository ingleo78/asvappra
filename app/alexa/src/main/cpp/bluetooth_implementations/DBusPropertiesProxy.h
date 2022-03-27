#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSPROPERTIESPROXY_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSPROPERTIESPROXY_H_

#include <memory>
#include <string>
#include "DBusProxy.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class DBusPropertiesProxy : public DBusProxy {
            public:
                static std::shared_ptr<DBusPropertiesProxy> create(const std::string& objectPath);
                bool getBooleanProperty(const std::string& interface, const std::string& property, bool* result);
                bool getStringProperty(const std::string& interface, const std::string& property, std::string* result);
                bool getVariantProperty(const std::string& interface, const std::string& property, ManagedGVariant* result);
                bool setProperty(const std::string& interface, const std::string& property, GVariant* value);
            private:
                explicit DBusPropertiesProxy(GDBusProxy* proxy, const std::string& objectPath);
            };
        }
    }
}
#endif
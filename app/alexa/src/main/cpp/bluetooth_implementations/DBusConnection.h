#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSCONNECTION_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSCONNECTION_H_

#include <memory>
#include <mutex>
#include <vector>
#include <gio/gio.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            class DBusConnection {
            public:
                ~DBusConnection();
                static unique_ptr<DBusConnection> create(GBusType connectionType = G_BUS_TYPE_SYSTEM);
                unsigned int subscribeToSignal(const char* serviceName, const char* interfaceName, const char* member, const char* firstArgumentFilter,
                                               GDBusSignalCallback callback, gpointer userData);
                GDBusConnection* getGDBusConnection();
                void close();
            private:
                explicit DBusConnection(GDBusConnection* connection);
                GDBusConnection* m_connection;
                mutex m_subscriptionsMutex;
                vector<guint> m_subscriptions;
            };
        }
    }
}
#endif
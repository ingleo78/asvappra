#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSOBJECT_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSOBJECT_H_

#include <memory>
#include <string>
#include <unordered_map>
#include "DBusObjectBase.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            template <class T> class DBusObject : public DBusObjectBase {
            public:
                virtual ~DBusObject() = default;
                using commandHandler_t = void (T::*)(GVariant* parameters, GDBusMethodInvocation* invocation);
            protected:
                DBusObject(shared_ptr<DBusConnection> connection, string xmlInterfaceIntrospection, string objectPath, unordered_map<string, commandHandler_t> methodMap) :
                           DBusObjectBase(connection, xmlInterfaceIntrospection, objectPath, &onMethodCallStatic), m_commands{methodMap} {}
            private:
                static void onMethodCallStatic(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path, const gchar* interface_name,
                                               const gchar* method_name, GVariant* parameters, GDBusMethodInvocation* invocation, gpointer data) {
                    auto thisPtr = static_cast<T*>(data);
                    thisPtr->onMethodCall(method_name, parameters, invocation);
                }
                void onMethodCall(const gchar* method_name, GVariant* parameters, GDBusMethodInvocation* invocation);
                unordered_map<string, commandHandler_t> m_commands;
            };
            template <class T> void DBusObject<T>::onMethodCall(const gchar* method_name, GVariant* parameters, GDBusMethodInvocation* invocation) {
                onMethodCalledInternal(method_name);
                auto iter = m_commands.find(method_name);
                if (iter != m_commands.end()) {
                    commandHandler_t handler = iter->second;
                    (static_cast<T*>(this)->*handler)(parameters, invocation);
                } else g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD, "Unknown method: ");
            }
        }
    }
}
#endif
#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSOBJECTBASE_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_DBUSOBJECTBASE_H_

#include <memory>
#include <string>
#include "DBusConnection.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            class DBusObjectBase {
            public:
                virtual ~DBusObjectBase();
                bool registerWithDBus();
                void unregisterObject();
            protected:
                DBusObjectBase(shared_ptr<DBusConnection> connection, const string& xmlInterfaceIntrospection, const string& objectPath,
                               GDBusInterfaceMethodCallFunc methodCallFunc);
                void onMethodCalledInternal(const char* methodName);
            private:
                const string m_xmlInterfaceIntrospection;
                unsigned int m_registrationId;
                GDBusInterfaceVTable m_interfaceVtable;
                shared_ptr<DBusConnection> m_connection;
                const string m_objectPath;
            };
        }
    }
}
#endif
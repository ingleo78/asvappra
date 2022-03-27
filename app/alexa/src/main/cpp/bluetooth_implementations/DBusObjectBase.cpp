#include <logger/Logger.h>
#include "BlueZUtils.h"
#include "DBusObjectBase.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            static const string TAG{"DBusObjectBase"};
            #define LX(event) LogEntry(TAG, event)
            DBusObjectBase::~DBusObjectBase() {
                ACSDK_DEBUG7(LX(__func__));
                unregisterObject();
            }
            DBusObjectBase::DBusObjectBase(shared_ptr<DBusConnection> connection, const string& xmlInterfaceIntrospection, const string& objectPath,
                                           GDBusInterfaceMethodCallFunc methodCallFunc) : m_xmlInterfaceIntrospection{xmlInterfaceIntrospection},
                                           m_registrationId{0}, m_interfaceVtable{methodCallFunc, nullptr, nullptr},
                                           m_connection{connection}, m_objectPath{objectPath} {}
            void DBusObjectBase::onMethodCalledInternal(const gchar* methodName) {
                ACSDK_DEBUG7(LX(__func__).d("methodName", methodName));
            }
            void DBusObjectBase::unregisterObject() {
                if (m_registrationId > 0) {
                    g_dbus_connection_unregister_object(m_connection->getGDBusConnection(), m_registrationId);
                    m_registrationId = 0;
                }
            }
            bool DBusObjectBase::registerWithDBus() {
                ACSDK_DEBUG5(LX(__func__));
                if (m_registrationId > 0) return true;
                ManagedGError error;
                GDBusNodeInfo* data = g_dbus_node_info_new_for_xml(m_xmlInterfaceIntrospection.c_str(), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("Failed to register object").d("error", error.getMessage()));
                    return false;
                }
                GDBusInterfaceInfo* interfaceInfo = data->interfaces[0];
                m_registrationId = g_dbus_connection_register_object(m_connection->getGDBusConnection(),m_objectPath.c_str(), interfaceInfo,
                                                              &m_interfaceVtable,this,nullptr,nullptr);
                ACSDK_DEBUG5(LX("Object registered").d("Object path", m_objectPath).d("Interface", interfaceInfo->name));
                return m_registrationId > 0;
            }
        }
    }
}
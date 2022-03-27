#include <logger/Logger.h>
#include "BlueZUtils.h"
#include "DBusConnection.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            static const std::string TAG{"DBusConnection"};
            #define LX(event) LogEntry(TAG, event)
            GDBusConnection* DBusConnection::getGDBusConnection() {
                return m_connection;
            }
            unique_ptr<DBusConnection> DBusConnection::create(GBusType connectionType) {
                ManagedGError error;
                GDBusConnection* connection = g_bus_get_sync(connectionType, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("createNewFailed").d("reason", error.getMessage()));
                    return nullptr;
                }
                g_dbus_connection_set_exit_on_close(connection, false);
                return unique_ptr<DBusConnection>(new DBusConnection(connection));
            }
            unsigned int DBusConnection::subscribeToSignal(const char* serviceName, const char* interfaceName, const char* member, const char* firstArgumentFilter,
                                                           GDBusSignalCallback callback, gpointer userData) {
                if (nullptr == serviceName) {
                    ACSDK_ERROR(LX("subscribeToSignalFailed").d("reason", "serviceName is null"));
                    return 0;
                }
                if (nullptr == interfaceName) {
                    ACSDK_ERROR(LX("subscribeToSignalFailed").d("reason", "interfaceName is null"));
                    return 0;
                }
                if (nullptr == member) {
                    ACSDK_ERROR(LX("subscribeToSignalFailed").d("reason", "member is null"));
                    return 0;
                }
                if (nullptr == callback) {
                    ACSDK_ERROR(LX("subscribeToSignalFailed").d("reason", "callback is null"));
                    return 0;
                }
                guint subId = g_dbus_connection_signal_subscribe(m_connection, serviceName, interfaceName, member,nullptr, firstArgumentFilter,
                                                           G_DBUS_SIGNAL_FLAGS_NONE, callback, userData,nullptr);
                if (0 == subId) {
                    ACSDK_ERROR(LX("subscribeToSignalFailed").d("reason", "failed to subscribe"));
                    return 0;
                }
                ACSDK_DEBUG7(LX("Subscribed to signal").d("service", serviceName).d("interface", interfaceName).d("member", member).d("result", subId));
                lock_guard<std::mutex> guard(m_subscriptionsMutex);
                m_subscriptions.push_back(subId);
                return subId;
            }
            DBusConnection::DBusConnection(GDBusConnection* connection) : m_connection{connection} {}
            void DBusConnection::close() {
                ACSDK_DEBUG5(LX(__func__));
                if (!m_connection)  return;
                {
                    lock_guard<std::mutex> guard(m_subscriptionsMutex);
                    for (auto subscriptionId : m_subscriptions) g_dbus_connection_signal_unsubscribe(m_connection, subscriptionId);
                    m_subscriptions.clear();
                }
                g_dbus_connection_flush_sync(m_connection, nullptr, nullptr);
                g_dbus_connection_close_sync(m_connection, nullptr, nullptr);
                g_object_unref(m_connection);
                m_connection = nullptr;
            }
            DBusConnection::~DBusConnection() {
                ACSDK_DEBUG7(LX(__func__));
                close();
            }
        }
    }
}
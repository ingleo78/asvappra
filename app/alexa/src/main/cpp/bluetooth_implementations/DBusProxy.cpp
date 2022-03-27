#include <gio/gunixfdlist.h>
#include <gio/gdbusproxy.h>
#include <logger/Logger.h>
#include "BlueZConstants.h"
#include "DBusProxy.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            static const string TAG{"BlueZUtils"};
            static const int PROXY_DEFAULT_TIMEOUT = -1;
            #define LX(event) LogEntry(TAG, event)
            DBusProxy::DBusProxy(GDBusProxy* proxy, const string& objectPath) : m_proxy{proxy}, m_objectPath{objectPath} {}
            DBusProxy::~DBusProxy() {
                ACSDK_DEBUG7(LX(__func__));
                if (m_proxy) g_object_unref(m_proxy);
            }
            shared_ptr<DBusProxy> DBusProxy::create(const string& interfaceName, const string& objectPath) {
                GError* error = nullptr;
                GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,G_DBUS_PROXY_FLAGS_NONE,nullptr,
                                                                  BlueZConstants::BLUEZ_SERVICE_NAME, objectPath.c_str(), interfaceName.c_str(), nullptr,
                                                                  &error);
                if (!proxy) {
                    ACSDK_ERROR(LX("createFailed").d("error", error->message));
                    g_error_free(error);
                    return nullptr;
                }
                return shared_ptr<DBusProxy>(new DBusProxy(proxy, objectPath));
            }
            ManagedGVariant DBusProxy::callMethod(const std::string& methodName, GVariant* parameters, GError** error) {
                GVariant* tempResult = g_dbus_proxy_call_sync(
                    m_proxy, methodName.c_str(), parameters, G_DBUS_CALL_FLAGS_NONE, PROXY_DEFAULT_TIMEOUT, nullptr, error);
                return ManagedGVariant(tempResult);
            }
            ManagedGVariant DBusProxy::callMethodWithFDList(const string& methodName, GVariant* parameters, GUnixFDList** outlist, GError** error) {
                GVariant* tempResult = g_dbus_proxy_call_with_unix_fd_list_sync(m_proxy, methodName.c_str(), parameters,G_DBUS_CALL_FLAGS_NONE,
                                                                                PROXY_DEFAULT_TIMEOUT,nullptr, outlist, nullptr,
                                                                                error);
                return ManagedGVariant(tempResult);
            }
            string DBusProxy::getObjectPath() const {
                return m_objectPath;
            }
            GDBusProxy* DBusProxy::get() {
                return m_proxy;
            }
        }
    }
}
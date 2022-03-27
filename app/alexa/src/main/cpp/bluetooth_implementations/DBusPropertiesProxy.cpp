#include <logger/Logger.h>
#include "BlueZConstants.h"
#include "BlueZDeviceManager.h"
#include "DBusPropertiesProxy.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            static const string TAG{"DBusPropertiesProxy"};
            #define LX(event) LogEntry(TAG, event)
            DBusPropertiesProxy::DBusPropertiesProxy(GDBusProxy* proxy, const string& objectPath) :
                    DBusProxy(proxy, objectPath) {
            }
            shared_ptr<DBusPropertiesProxy> DBusPropertiesProxy::create(const string& objectPath) {
                GError* error = nullptr;
                GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,G_DBUS_PROXY_FLAGS_NONE,nullptr, BlueZConstants::BLUEZ_SERVICE_NAME,
                                                                  objectPath.c_str(), BlueZConstants::PROPERTIES_INTERFACE,nullptr, &error);
                if (error) {
                    ACSDK_ERROR(LX("createFailed").d("error", error->message));
                    g_error_free(error);
                    return nullptr;
                }
                return shared_ptr<DBusPropertiesProxy>(new DBusPropertiesProxy(proxy, objectPath));
            }
            bool DBusPropertiesProxy::getBooleanProperty(const string& interface, const string& property, bool* result) {
                if (nullptr == result) {
                    ACSDK_ERROR(LX("getBooleanPropertyFailed").d("reason", "result is null"));
                    return false;
                }
                ManagedGError error;
                ManagedGVariant varResult = callMethod("Get", g_variant_new("(ss)", interface.c_str(), property.c_str()), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("Failed to get boolean property", property).d("error", error.getMessage()).d("interface", interface)
                                    .d("property", property)
                                    .d("path", getObjectPath()));
                    return false;
                }
                GVariantTupleReader tupleReader(varResult);
                ManagedGVariant unboxed = tupleReader.getVariant(0).unbox();
                *result = static_cast<bool>(g_variant_get_boolean(unboxed.get()));
                return true;
            }
            bool DBusPropertiesProxy::getVariantProperty(const string& interface, const string& property, ManagedGVariant* result) {
                ACSDK_DEBUG5(LX(__func__));
                if (nullptr == result) {
                    ACSDK_ERROR(LX("getVariantPropertyFailed").d("reason", "result is null"));
                    return false;
                }
                ManagedGError error;
                ManagedGVariant varResult = callMethod("Get", g_variant_new("(ss)", interface.c_str(), property.c_str()), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("getVariantPropertyFailed").d("Failed to get variant property", property).d("error", error.getMessage()));
                    return false;
                }
                result->swap(varResult);
                return true;
            }
            bool DBusPropertiesProxy::getStringProperty(const string& interface, const string& property, string* result) {
                ACSDK_DEBUG9(LX(__func__).d("object", getObjectPath()).d("interface", interface).d("property", property));
                if (nullptr == result) {
                    ACSDK_ERROR(LX("getStringPropertyFailed").d("reason", "result is null"));
                    return false;
                }
                ManagedGError error;
                ManagedGVariant varResult = callMethod("Get", g_variant_new("(ss)", interface.c_str(), property.c_str()), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("getStringPropertyFailed").d("Failed to get string property", property).d("error", error.getMessage()));
                    return false;
                }
                GVariantTupleReader tupleReader(varResult);
                ManagedGVariant unboxed = tupleReader.getVariant(0).unbox();
                *result = g_variant_get_string(unboxed.get(), nullptr);
                return true;
            }
            bool DBusPropertiesProxy::setProperty(const string& interface, const string& property, GVariant* value) {
                if (nullptr == value) {
                    ACSDK_ERROR(LX("setPropertyFailed").d("reason", "value is null"));
                    return false;
                }
                ManagedGError error;
                ManagedGVariant varResult = callMethod("Set", g_variant_new("(ssv)", interface.c_str(), property.c_str(), value), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("setPropertyFailed").d("Failed to set property value", property).d("error", error.getMessage()));
                    return false;
                }
                return true;
            }
        }
    }
}
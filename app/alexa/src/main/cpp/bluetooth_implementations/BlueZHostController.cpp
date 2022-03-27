#include <logger/Logger.h>
#include "BlueZHostController.h"
#include "BlueZConstants.h"
#include "BlueZDeviceManager.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            static const string TAG{"BlueZHostController"};
            #define LX(event) LogEntry(TAG, event)
            static const string ALIAS_PROPERTY = "Alias";
            static const string DISCOVERABLE_PROPERTY = "Discoverable";
            static const unsigned int MAC_SIZE = 17;
            static const string SCANNING_PROPERTY = "Discovering";
            static const string START_SCAN = "StartDiscovery";
            static const string STOP_SCAN = "StopDiscovery";
            static const string DEFAULT_NAME = "Device";
            static bool truncate(const unique_ptr<MacAddressString>& mac, string* truncatedMac) {
                ACSDK_DEBUG5(LX(__func__));
                if (!mac) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullMAC"));
                    return false;
                } else if (!truncatedMac) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullTruncatedMAC"));
                    return false;
                } else if (mac->getString().length() != MAC_SIZE) {
                    ACSDK_ERROR(LX(__func__).d("reason", "invalidMACLength"));
                    return false;
                }
                *truncatedMac = mac->getString();
                char X = 'X';
                truncatedMac->at(0) = X;
                truncatedMac->at(1) = X;
                truncatedMac->at(3) = X;
                truncatedMac->at(4) = X;
                truncatedMac->at(6) = X;
                truncatedMac->at(7) = X;
                truncatedMac->at(9) = X;
                truncatedMac->at(10) = X;
                return true;
            }
            unique_ptr<BlueZHostController> BlueZHostController::create(const string& adapterObjectPath) {
                ACSDK_DEBUG5(LX(__func__).d("adapterObjectPath", adapterObjectPath));
                if (adapterObjectPath.empty()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "emptyAdapterPath").m("Host controller requires adapter to operate!"));
                    return nullptr;
                }
                auto hostController = unique_ptr<BlueZHostController>(new BlueZHostController(adapterObjectPath));
                if (!hostController->init()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "initFailed"));
                    return nullptr;
                }
                return hostController;
            }
            BlueZHostController::BlueZHostController(const string& adapterObjectPath) : m_adapterObjectPath{adapterObjectPath} {}
            bool BlueZHostController::init() {
                ACSDK_DEBUG5(LX(__func__));
                m_adapter = DBusProxy::create(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, m_adapterObjectPath);
                if (!m_adapter) {
                    ACSDK_ERROR(LX(__func__).d("reason", "createAdapterProxyFailed"));
                    return false;
                }
                m_adapterProperties = DBusPropertiesProxy::create(m_adapterObjectPath);
                if (!m_adapterProperties) {
                    ACSDK_ERROR(LX(__func__).d("reason", "createPropertiesProxyFailed"));
                    return false;
                }
                string mac;
                if (!m_adapterProperties->getStringProperty(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, BlueZConstants::BLUEZ_DEVICE_INTERFACE_ADDRESS, &mac)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "noMACAddress"));
                    return false;
                }
                m_mac = MacAddressString::create(mac);
                if (!m_mac) {
                    ACSDK_ERROR(LX(__func__).d("reason", "invalidMAC"));
                    return false;
                }
                if (!m_adapterProperties->getStringProperty(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, BlueZConstants::BLUEZ_DEVICE_INTERFACE_ALIAS, &m_friendlyName)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "noValidFriendlyName").m("Falling back"));
                    string truncatedMac;
                    m_friendlyName = truncate(m_mac, &truncatedMac) ? truncatedMac : DEFAULT_NAME;
                }
                ACSDK_DEBUG5(LX("adapterProperties").d("mac", m_mac->getString()).d("friendlyName", m_friendlyName));
                return true;
            }
            string BlueZHostController::getMac() const {
                ACSDK_DEBUG5(LX(__func__));
                return m_mac->getString();
            }
            string BlueZHostController::getFriendlyName() const {
                ACSDK_DEBUG5(LX(__func__));
                return m_friendlyName;
            }
            future<bool> BlueZHostController::enterDiscoverableMode() {
                ACSDK_DEBUG5(LX(__func__));
                return setDiscoverable(true);
            }
            future<bool> BlueZHostController::exitDiscoverableMode() {
                ACSDK_DEBUG5(LX(__func__));
                return setDiscoverable(false);
            }
            std::future<bool> BlueZHostController::setDiscoverable(bool discoverable) {
                ACSDK_DEBUG5(LX(__func__).d("discoverable", discoverable));
                promise<bool> promise;
                bool success = false;
                {
                    lock_guard<mutex> lock(m_adapterMutex);
                    success = m_adapterProperties->setProperty(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, DISCOVERABLE_PROPERTY, g_variant_new_boolean(discoverable));
                }
                promise.set_value(success);
                if (!success) { ACSDK_ERROR(LX(__func__).d("reason", "setAdapterPropertyFailed").d("discoverable", discoverable)); }
                return promise.get_future();
            }
            bool BlueZHostController::isDiscoverable() const {
                ACSDK_DEBUG5(LX(__func__));
                bool result = false;
                lock_guard<std::mutex> lock(m_adapterMutex);
                m_adapterProperties->getBooleanProperty(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, DISCOVERABLE_PROPERTY, &result);
                return result;
            }
            future<bool> BlueZHostController::startScan() {
                ACSDK_DEBUG5(LX(__func__));
                return changeScanState(true);
            }
            future<bool> BlueZHostController::stopScan() {
                ACSDK_DEBUG5(LX(__func__));
                return changeScanState(false);
            }
            future<bool> BlueZHostController::changeScanState(bool scanning) {
                ACSDK_DEBUG5(LX(__func__).d("scanning", scanning));
                promise<bool> promise;
                ManagedGError error;
                {
                    lock_guard<mutex> lock(m_adapterMutex);
                    ManagedGVariant result = m_adapter->callMethod(scanning ? START_SCAN : STOP_SCAN, nullptr, error.toOutputParameter());
                }
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "callScanMethodFailed").d("error", error.getMessage()));
                    promise.set_value(false);
                } else promise.set_value(true);
                return promise.get_future();
            }
            bool BlueZHostController::isScanning() const {
                ACSDK_DEBUG5(LX(__func__));
                bool result = false;
                lock_guard<std::mutex> lock(m_adapterMutex);
                m_adapterProperties->getBooleanProperty(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, SCANNING_PROPERTY, &result);
                return result;
            }
            void BlueZHostController::onPropertyChanged(const GVariantMapReader& changesMap) {
                char* alias = nullptr;
                bool aliasChanged = changesMap.getCString(ALIAS_PROPERTY.c_str(), &alias);
                lock_guard<std::mutex> lock(m_adapterMutex);
                if (aliasChanged) {
                    if (!alias) { ACSDK_ERROR(LX(__func__).d("reason", "nullAlias")); }
                    else {
                        ACSDK_DEBUG5(LX("nameChanged").d("oldName", m_friendlyName).d("newName", alias));
                        m_friendlyName = alias;
                    }
                }
            }
        }
    }
}
#include <util/bluetooth/SDPRecords.h>
#include <logger/Logger.h>
#include <memory/Memory.h>
#include "BlueZA2DPSink.h"
#include "BlueZA2DPSource.h"
#include "BlueZAVRCPController.h"
#include "BlueZAVRCPTarget.h"
#include "BlueZBluetoothDevice.h"
#include "BlueZConstants.h"
#include "BlueZDeviceManager.h"
#include "BlueZHFP.h"
#include "BlueZHID.h"
#include "BlueZSPP.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace utils::bluetooth;
            using namespace logger;
            using namespace memory;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            static const std::string TAG{"BlueZBluetoothDevice"};
            #define LX(event) LogEntry(TAG, event)
            static const string BLUEZ_DEVICE_PROPERTY_ALIAS = "Alias";
            static const string BLUEZ_DEVICE_PROPERTY_UUIDS = "UUIDs";
            static const string BLUEZ_ERROR_NOTFOUND = "org.bluez.Error.DoesNotExist";
            static const string BLUEZ_ERROR_RESOURCE_UNAVAILABLE = "org.bluez.Error.Failed: Resource temporarily unavailable";
            static const string BLUEZ_DEVICE_METHOD_PAIR = "Pair";
            static const string BLUEZ_DEVICE_METHOD_CONNECT = "Connect";
            static const string BLUEZ_DEVICE_METHOD_DISCONNECT = "Disconnect";
            static const string BLUZ_DEVICE_METHOD_CONNECT_PROFILE = "ConnectProfile";
            static const string BLUZ_DEVICE_METHOD_DISCONNECT_PROFILE = "DisconnectProfile";
            static const string BLUEZ_DEVICE_PROPERTY_PAIRED = "Paired";
            static const string BLUEZ_DEVICE_PROPERTY_CONNECTED = "Connected";
            static const string BLUEZ_DEVICE_PROPERTY_CLASS = "Class";
            static const string BLUEZ_ADAPTER_REMOVE_DEVICE = "RemoveDevice";
            static const string MEDIA_CONTROL_INTERFACE = "org.bluez.MediaControl1";
            shared_ptr<BlueZBluetoothDevice> BlueZBluetoothDevice::create(const string& mac, const string& objectPath, shared_ptr<BlueZDeviceManager> deviceManager) {
                ACSDK_DEBUG5(LX(__func__));
                if (!g_variant_is_object_path(objectPath.c_str())) {
                    ACSDK_ERROR(LX(__func__).d("reason", "invalidObjectPath").d("objPath", objectPath));
                    return nullptr;
                }
                auto device = shared_ptr<BlueZBluetoothDevice>(new BlueZBluetoothDevice(mac, objectPath, deviceManager));
                if (!device->init()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "initFailed"));
                    return nullptr;
                }
                return device;
            }
            BlueZBluetoothDevice::BlueZBluetoothDevice(const string& mac, const string& objectPath, shared_ptr<BlueZDeviceManager> deviceManager) : m_mac{mac},
                                                       m_objectPath{objectPath}, m_deviceState{BlueZDeviceState::FOUND}, m_deviceManager{deviceManager} {}
            string BlueZBluetoothDevice::getMac() const {
                ACSDK_DEBUG5(LX(__func__));
                return m_mac;
            }
            string BlueZBluetoothDevice::getFriendlyName() const {
                ACSDK_DEBUG5(LX(__func__));
                return m_friendlyName;
            }
            bool BlueZBluetoothDevice::updateFriendlyName() {
                ACSDK_DEBUG5(LX(__func__));
                if (!m_propertiesProxy->getStringProperty(BlueZConstants::BLUEZ_DEVICE_INTERFACE, BLUEZ_DEVICE_PROPERTY_ALIAS, &m_friendlyName)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "getNameFailed"));
                    return false;
                }
                return true;
            }
            BlueZBluetoothDevice::~BlueZBluetoothDevice() {
                ACSDK_DEBUG5(LX(__func__).d("mac", getMac()));
                m_executor.shutdown();
                {
                    lock_guard<mutex> lock(m_servicesMapMutex);
                    for (auto& entry : m_servicesMap) entry.second->cleanup();
                    m_servicesMap.clear();
                }
            }
            bool BlueZBluetoothDevice::init() {
                ACSDK_DEBUG5(LX(__func__).d("path", m_objectPath));
                m_deviceProxy = DBusProxy::create(BlueZConstants::BLUEZ_DEVICE_INTERFACE, m_objectPath);
                if (!m_deviceProxy) {
                    ACSDK_ERROR(LX(__func__).d("reason", "createDeviceProxyFailed"));
                    return false;
                }
                m_propertiesProxy = DBusPropertiesProxy::create(m_objectPath);
                if (!m_propertiesProxy) {
                    ACSDK_ERROR(LX(__func__).d("reason", "createPropertyProxyFailed"));
                    return false;
                }
                updateFriendlyName();
                bool isPaired = false;
                if (queryDeviceProperty(BLUEZ_DEVICE_PROPERTY_PAIRED, &isPaired) && isPaired) {
                    ACSDK_DEBUG5(LX(__func__).m("deviceIsPaired"));
                    transitionToState(BlueZDeviceState::IDLE, true);
                }
                if (!initializeServices(getServiceUuids())) {
                    ACSDK_ERROR(LX(__func__).d("reason", "initializeServicesFailed"));
                    return false;
                }
                bool isConnected = executeIsConnectedToRelevantServices();
                if (isConnected) transitionToState(BlueZDeviceState::CONNECTED, true);
                int defaultUndefinedClass = BlueZBluetoothDevice::MetaData::UNDEFINED_CLASS_VALUE;
                m_metaData = memory::make_unique<BlueZBluetoothDevice::MetaData>(Optional<int>(), Optional<int>(), defaultUndefinedClass, Optional<int>(), Optional<std::string>());
                ManagedGVariant classOfDevice;
                if (m_propertiesProxy->getVariantProperty(BlueZConstants::BLUEZ_DEVICE_INTERFACE, BLUEZ_DEVICE_PROPERTY_CLASS, &classOfDevice)) {
                    GVariantTupleReader tupleReader(classOfDevice);
                    ManagedGVariant unboxed = tupleReader.getVariant(0).unbox();
                    if (g_variant_is_of_type(unboxed.get(), G_VARIANT_TYPE_UINT32) == TRUE) {
                        m_metaData->classOfDevice = g_variant_get_uint32(unboxed.get());
                        ACSDK_DEBUG5(LX(__func__).d("ClassOfDevice", g_variant_get_uint32(unboxed.get())));
                    }
                }
                return true;
            }
            template <typename BlueZServiceType> bool BlueZBluetoothDevice::initializeService() {
                ACSDK_DEBUG5(LX(__func__).d("supports", BlueZServiceType::NAME));
                shared_ptr<BlueZServiceType> service = BlueZServiceType::create();
                if (!service) {
                    ACSDK_ERROR(LX(__func__).d("createBlueZServiceFailed", BlueZServiceType::NAME));
                    return false;
                } else {
                    service->setup();
                    insertService(service);
                }
                return true;
            }
            bool BlueZBluetoothDevice::initializeServices(const unordered_set<string>& uuids) {
                ACSDK_DEBUG5(LX(__func__));
                for (const auto& uuid : uuids) {
                    ACSDK_DEBUG9(LX(__func__).d("supportedUUID", uuid));
                    if (A2DPSourceInterface::UUID == uuid && !serviceExists(uuid)) {
                        ACSDK_DEBUG5(LX(__func__).d("supports", A2DPSourceInterface::NAME));
                        auto a2dpSource = BlueZA2DPSource::create(m_deviceManager);
                        if (!a2dpSource) {
                            ACSDK_ERROR(LX(__func__).d("reason", "createA2DPFailed"));
                            return false;
                        } else {
                            a2dpSource->setup();
                            insertService(a2dpSource);
                        }
                    } else if (AVRCPTargetInterface::UUID == uuid && !serviceExists(uuid)) {
                        ACSDK_DEBUG5(LX(__func__).d("supports", AVRCPTargetInterface::NAME));
                        auto mediaControlProxy = DBusProxy::create(MEDIA_CONTROL_INTERFACE, m_objectPath);
                        if (!mediaControlProxy) {
                            ACSDK_ERROR(LX(__func__).d("reason", "nullMediaControlProxy"));
                            return false;
                        }
                        auto avrcpTarget = BlueZAVRCPTarget::create(mediaControlProxy);
                        if (!avrcpTarget) {
                            ACSDK_ERROR(LX(__func__).d("reason", "createAVRCPTargetFailed"));
                            return false;
                        } else {
                            avrcpTarget->setup();
                            insertService(avrcpTarget);
                        }
                    } else if (A2DPSinkInterface::UUID == uuid && !serviceExists(uuid)) {
                        if (!initializeService<BlueZA2DPSink>()) return false;
                    } else if (AVRCPControllerInterface::UUID == uuid && !serviceExists(uuid)) {
                        if (!initializeService<BlueZAVRCPController>()) return false;
                    } else if (HFPInterface::UUID == uuid && !serviceExists(uuid)) {
                        if (!initializeService<BlueZHFP>()) return false;
                    } else if (HIDInterface::UUID == uuid && !serviceExists(uuid)) {
                        if (!initializeService<BlueZHID>()) return false;
                    } else if (SPPInterface::UUID == uuid && !serviceExists(uuid)) {
                        if (!initializeService<BlueZSPP>()) return false;
                    }
                }
                return true;
            }
            bool BlueZBluetoothDevice::isPaired() {
                ACSDK_DEBUG5(LX(__func__));
                auto future = m_executor.submit([this] { return executeIsPaired(); });
                if (future.valid()) return future.get();
                else {
                    ACSDK_ERROR(LX(__func__).d("reason", "invalidFuture").d("action", "defaultingFalse"));
                    return false;
                }
            }
            bool BlueZBluetoothDevice::executeIsPaired() {
                ACSDK_DEBUG5(LX(__func__));
                return BlueZDeviceState::UNPAIRED != m_deviceState && BlueZDeviceState::FOUND != m_deviceState;
            }
            future<bool> BlueZBluetoothDevice::pair() {
                ACSDK_DEBUG5(LX(__func__));
                return m_executor.submit([this] { return executePair(); });
            }
            bool BlueZBluetoothDevice::executePair() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGError error;
                m_deviceProxy->callMethod(BLUEZ_DEVICE_METHOD_PAIR, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("error", error.getMessage()));
                    return false;
                }
                return true;
            }
            future<bool> BlueZBluetoothDevice::unpair() {
                ACSDK_DEBUG5(LX(__func__));
                return m_executor.submit([this] { return executeUnpair(); });
            }
            bool BlueZBluetoothDevice::executeUnpair() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGError error;
                auto adapterProxy = DBusProxy::create(BlueZConstants::BLUEZ_ADAPTER_INTERFACE, m_deviceManager->getAdapterPath());
                if (!adapterProxy) {
                    ACSDK_ERROR(LX(__func__).d("error", "createAdapterProxyFailed"));
                    return false;
                }
                adapterProxy->callMethod(BLUEZ_ADAPTER_REMOVE_DEVICE, g_variant_new("(o)", m_objectPath.c_str()), error.toOutputParameter());
                if (error.hasError()) {
                    string errorMsg = error.getMessage();
                    if (string::npos != errorMsg.find(BLUEZ_ERROR_NOTFOUND)) return true;
                    ACSDK_ERROR(LX(__func__).d("error", errorMsg));
                    return false;
                }
                transitionToState(BlueZDeviceState::UNPAIRED, true);
                transitionToState(BlueZDeviceState::FOUND, true);
                return true;
            }
            string BlueZBluetoothDevice::getObjectPath() const {
                ACSDK_DEBUG5(LX(__func__));
                return m_objectPath;
            }
            unordered_set<string> BlueZBluetoothDevice::getServiceUuids(GVariant* array) {
                ACSDK_DEBUG5(LX(__func__));
                unordered_set<string> uuids;
                if (!array) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullArray"));
                    return uuids;
                } else if (!g_variant_is_of_type(array, G_VARIANT_TYPE_ARRAY)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "invalidType").d("type", g_variant_get_type_string(array)));
                    return uuids;
                }
                GVariantTupleReader arrayReader(array);
                arrayReader.forEach([&uuids](GVariant* variant) {
                    if (!variant) {
                        ACSDK_ERROR(LX("iteratingArrayFailed").d("reason", "nullVariant"));
                        return false;
                    }
                    const gchar* temp = g_variant_get_string(variant, NULL);
                    string uuid(temp);
                    ACSDK_DEBUG5(LX(__func__).d("uuid", uuid));
                    uuids.insert(uuid);
                    return true;
                });
                return uuids;
            }
            unordered_set<string> BlueZBluetoothDevice::getServiceUuids() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGVariant uuidsTuple;
                if (!m_propertiesProxy->getVariantProperty(BlueZConstants::BLUEZ_DEVICE_INTERFACE, BLUEZ_DEVICE_PROPERTY_UUIDS, &uuidsTuple)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "getVariantPropertyFailed"));
                    return unordered_set<string>();
                }
                GVariantTupleReader tupleReader(uuidsTuple);
                ManagedGVariant array = tupleReader.getVariant(0).unbox();
                if (!array.hasValue()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "unexpectedVariantFormat").d("variant", uuidsTuple.dumpToString(false)));
                    return unordered_set<string>();
                }
                return getServiceUuids(array.get());
            }
            bool BlueZBluetoothDevice::isConnected() {
                ACSDK_DEBUG5(LX(__func__));
                auto future = m_executor.submit([this] { return executeIsConnected(); });
                if (future.valid()) return future.get();
                else {
                    ACSDK_ERROR(LX(__func__).d("reason", "invalidFuture").d("action", "defaultingFalse"));
                    return false;
                }
            }
            bool BlueZBluetoothDevice::executeIsConnected() {
                ACSDK_DEBUG5(LX(__func__));
                return BlueZDeviceState::CONNECTED == m_deviceState;
            }
            std::future<bool> BlueZBluetoothDevice::connect() {
                ACSDK_DEBUG5(LX(__func__));
                return m_executor.submit([this] { return executeConnect(); });
            }
            bool BlueZBluetoothDevice::executeConnect() {
                ACSDK_DEBUG5(LX(__func__));
                if (executeIsConnected()) return true;
                ManagedGError error;
                m_deviceProxy->callMethod(BLUEZ_DEVICE_METHOD_CONNECT, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    string errStr = error.getMessage() ? error.getMessage() : "";
                    ACSDK_ERROR(LX(__func__).d("error", errStr));
                    if (string::npos != errStr.find(BLUEZ_ERROR_RESOURCE_UNAVAILABLE)) {
                        transitionToState(BlueZDeviceState::CONNECTION_FAILED, false);
                    }
                    return false;
                }
                if (BlueZDeviceState::CONNECTION_FAILED == m_deviceState) transitionToState(BlueZDeviceState::CONNECTED, true);
                return true;
            }
            future<bool> BlueZBluetoothDevice::disconnect() {
                ACSDK_DEBUG5(LX(__func__));
                return m_executor.submit([this] { return executeDisconnect(); });
            }
            bool BlueZBluetoothDevice::executeDisconnect() {
                ACSDK_DEBUG5(LX(__func__));
                ManagedGError error;
                m_deviceProxy->callMethod(BLUEZ_DEVICE_METHOD_DISCONNECT, nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("error", error.getMessage()));
                    return false;
                }
                transitionToState(BlueZDeviceState::DISCONNECTED, true);
                transitionToState(BlueZDeviceState::IDLE, true);
                return true;
            }
            vector<shared_ptr<SDPRecordInterface>> BlueZBluetoothDevice::getSupportedServices() {
                ACSDK_DEBUG5(LX(__func__));
                vector<shared_ptr<SDPRecordInterface>> services;
                {
                    lock_guard<mutex> lock(m_servicesMapMutex);
                    for (auto& it : m_servicesMap) services.push_back(it.second->getRecord());
                }
                return services;
            }
            bool BlueZBluetoothDevice::serviceExists(const string& uuid) {
                lock_guard<mutex> lock(m_servicesMapMutex);
                return m_servicesMap.count(uuid) != 0;
            }
            bool BlueZBluetoothDevice::insertService(shared_ptr<BluetoothServiceInterface> service) {
                ACSDK_DEBUG5(LX(__func__));
                if (!service) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullService"));
                    return false;
                }
                shared_ptr<SDPRecordInterface> record = service->getRecord();
                if (!record) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullRecord"));
                    return false;
                }
                ACSDK_DEBUG5(LX(__func__).d("serviceUuid", record->getUuid()));
                bool success = false;
                {
                    lock_guard<mutex> lock(m_servicesMapMutex);
                    success = m_servicesMap.insert({record->getUuid(), service}).second;
                }
                if (!success) { ACSDK_ERROR(LX(__func__).d("reason", "serviceAlreadyExists")); }
                return success;
            }
            template <typename ServiceType> shared_ptr<ServiceType> BlueZBluetoothDevice::getService() {
                ACSDK_DEBUG5(LX(__func__).d("uuid", ServiceType::UUID));
                shared_ptr<ServiceType> service = nullptr;
                {
                    lock_guard<mutex> lock(m_servicesMapMutex);
                    auto it = m_servicesMap.find(ServiceType::UUID);
                    if (it == m_servicesMap.end()) { ACSDK_DEBUG(LX(__func__).d("reason", "serviceNotFound")); }
                    else service = static_pointer_cast<ServiceType>(it->second);
                }
                return service;
            }
            shared_ptr<BluetoothServiceInterface> BlueZBluetoothDevice::getService(std::string uuid) {
                if (A2DPSourceInterface::UUID == uuid) return getService<A2DPSourceInterface>();
                else if (A2DPSinkInterface::UUID == uuid) return getService<A2DPSinkInterface>();
                else if (AVRCPTargetInterface::UUID == uuid) return getService<AVRCPTargetInterface>();
                else if (AVRCPControllerInterface::UUID == uuid) return getService<AVRCPControllerInterface>();
                else if (HFPInterface::UUID == uuid) return getService<HFPInterface>();
                else if (HIDInterface::UUID == uuid) return getService<HIDInterface>();
                else if (SPPInterface::UUID == uuid) return getService<SPPInterface>();
                return nullptr;
            }
            DeviceState BlueZBluetoothDevice::getDeviceState() {
                ACSDK_DEBUG5(LX(__func__));
                return m_executor.submit([this] { return convertToDeviceState(m_deviceState); }).get();
            }
            BlueZBluetoothDevice::MetaData BlueZBluetoothDevice::getDeviceMetaData() {
                return *m_metaData;
            }
            MediaStreamingState BlueZBluetoothDevice::getStreamingState() {
                return m_executor.submit([this] {
                          if (!m_deviceManager) {
                              ACSDK_ERROR(LX(__func__).d("reason", "nullDeviceManager"));
                              return MediaStreamingState::IDLE;
                          }
                          return m_deviceManager->getMediaStreamingState();
                      }).get();
            }
            bool BlueZBluetoothDevice::toggleServiceConnection(bool enabled, std::shared_ptr<BluetoothServiceInterface> service) {
                ACSDK_DEBUG5(LX(__func__));
                return m_executor.submit([this, enabled, service] { return executeToggleServiceConnection(enabled, service); }).get();
            }
            bool BlueZBluetoothDevice::executeToggleServiceConnection(bool enabled, shared_ptr<BluetoothServiceInterface> service) {
                string uuid = service->getRecord()->getUuid();
                ManagedGError error;
                if (enabled) m_deviceProxy->callMethod(BLUZ_DEVICE_METHOD_CONNECT_PROFILE, g_variant_new_string(uuid.c_str()), error.toOutputParameter());
                else m_deviceProxy->callMethod(BLUZ_DEVICE_METHOD_DISCONNECT_PROFILE, g_variant_new_string(uuid.c_str()), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX(__func__).d("reason", error.getMessage()));
                    return false;
                }
                return true;
            }
            DeviceState BlueZBluetoothDevice::convertToDeviceState(BlueZDeviceState bluezDeviceState) {
                switch(bluezDeviceState) {
                    case BlueZDeviceState::FOUND: return DeviceState::FOUND;
                    case BlueZDeviceState::UNPAIRED: return DeviceState::UNPAIRED;
                    case BlueZDeviceState::PAIRED: return DeviceState::PAIRED;
                    case BlueZDeviceState::CONNECTION_FAILED: case BlueZDeviceState::IDLE: return DeviceState::IDLE;
                    case BlueZDeviceState::DISCONNECTED: return DeviceState::DISCONNECTED;
                    case BlueZDeviceState::CONNECTED: return DeviceState::CONNECTED;
                }
                ACSDK_ERROR(LX(__func__).d("reason", "noConversionFound").d("bluezDeviceState", bluezDeviceState).d("defaulting", DeviceState::FOUND));
                return DeviceState::FOUND;
            }
            bool BlueZBluetoothDevice::queryDeviceProperty(const string& name, bool* value) {
                ACSDK_DEBUG5(LX(__func__).d("name", name));
                if (!value) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullValue"));
                    return false;
                } else if (!m_propertiesProxy) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullPropertiesProxy"));
                    return false;
                }
                return m_propertiesProxy->getBooleanProperty(BlueZConstants::BLUEZ_DEVICE_INTERFACE, name.c_str(), value);
            }
            void BlueZBluetoothDevice::transitionToState(BlueZDeviceState newState, bool sendEvent) {
                ACSDK_DEBUG5(LX(__func__).d("oldState", m_deviceState).d("newState", newState).d("sendEvent", sendEvent));
                m_deviceState = newState;
                if (!m_deviceManager) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullDeviceManager"));
                    return;
                } else if (!m_deviceManager->getEventBus()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullEventBus"));
                    return;
                }
                if (sendEvent) {
                    m_deviceManager->getEventBus()->sendEvent(DeviceStateChangedEvent(shared_from_this(), convertToDeviceState(newState)));
                }
            }
            bool BlueZBluetoothDevice::executeIsConnectedToRelevantServices() {
                ACSDK_DEBUG5(LX(__func__));
                bool isPaired = false;
                bool isConnected = false;
                bool relevantServiceDiscovered = getService(A2DPSinkInterface::UUID) != nullptr || getService(A2DPSourceInterface::UUID) != nullptr;
                return relevantServiceDiscovered && queryDeviceProperty(BLUEZ_DEVICE_PROPERTY_PAIRED, &isPaired) && isPaired &&
                       queryDeviceProperty(BLUEZ_DEVICE_PROPERTY_CONNECTED, &isConnected) && isConnected;
            }
            void BlueZBluetoothDevice::onPropertyChanged(const GVariantMapReader& changesMap) {
                ACSDK_DEBUG5(LX(__func__).d("values", g_variant_print(changesMap.get(), true)));
                gboolean paired = false;
                bool pairedChanged = changesMap.getBoolean(BLUEZ_DEVICE_PROPERTY_PAIRED.c_str(), &paired);
                gboolean connected = false;
                bool connectedChanged = changesMap.getBoolean(BLUEZ_DEVICE_PROPERTY_CONNECTED.c_str(), &connected);
                char* alias = nullptr;
                bool aliasChanged = changesMap.getCString(BLUEZ_DEVICE_PROPERTY_ALIAS.c_str(), &alias);
                string aliasStr;
                if (aliasChanged) {
                    if (!alias) {
                        ACSDK_ERROR(LX(__func__).d("reason", "nullAlias"));
                        aliasChanged = false;
                    } else aliasStr = alias;
                }
                ManagedGVariant uuidsVariant = changesMap.getVariant(BLUEZ_DEVICE_PROPERTY_UUIDS.c_str());
                unordered_set<string> uuids;
                if (uuidsVariant.hasValue()) {
                    auto uuids = getServiceUuids(uuidsVariant.get());
                    initializeServices(uuids);
                }
                m_executor.submit([this, pairedChanged, paired, connectedChanged, connected, aliasChanged, aliasStr] {
                    if (aliasChanged) {
                        ACSDK_DEBUG5(LX("nameChanged").d("oldName", m_friendlyName).d("newName", aliasStr));
                        m_friendlyName = aliasStr;
                    }
                    switch(m_deviceState) {
                        case BlueZDeviceState::FOUND: {
                            if (pairedChanged && paired) {
                                transitionToState(BlueZDeviceState::PAIRED, true);
                                transitionToState(BlueZDeviceState::IDLE, true);
                                if (executeIsConnectedToRelevantServices()) transitionToState(BlueZDeviceState::CONNECTED, true);
                            }
                            break;
                        }
                        case BlueZDeviceState::IDLE: {
                            if (executeIsConnectedToRelevantServices()) transitionToState(BlueZDeviceState::CONNECTED, true);
                            else if (pairedChanged && !paired) {
                                transitionToState(BlueZDeviceState::UNPAIRED, true);
                                transitionToState(BlueZDeviceState::FOUND, true);
                            }
                            break;
                        }
                        case BlueZDeviceState::CONNECTED: {
                            if (pairedChanged && !paired) {
                                transitionToState(BlueZDeviceState::UNPAIRED, true);
                                transitionToState(BlueZDeviceState::FOUND, true);
                            } else if (connectedChanged && !connected) {
                                transitionToState(BlueZDeviceState::DISCONNECTED, true);
                                transitionToState(BlueZDeviceState::IDLE, true);
                            }
                            break;
                        }
                        case BlueZDeviceState::UNPAIRED: case BlueZDeviceState::PAIRED: case BlueZDeviceState::DISCONNECTED: {
                            ACSDK_ERROR(LX("onPropertyChanged").d("reason", "invalidState").d("state", m_deviceState));
                            break;
                        }
                        case BlueZDeviceState::CONNECTION_FAILED: {
                            if (pairedChanged && !paired) {
                                transitionToState(BlueZDeviceState::UNPAIRED, true);
                                transitionToState(BlueZDeviceState::FOUND, true);
                            }
                        }
                    }
                });
            }
        }
    }
}
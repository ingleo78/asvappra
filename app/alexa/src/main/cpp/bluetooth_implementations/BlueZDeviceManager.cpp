#include <algorithm>
#include <cstring>
#include <string>
#include <unordered_map>
#include <sdkinterfaces/Bluetooth/Services/A2DPSinkInterface.h>
#include <util/bluetooth/BluetoothEvents.h>
#include <util/bluetooth/SDPRecords.h>
#include <logger/Logger.h>
#include <uuid_generation/UUIDGeneration.h>
#include "BlueZBluetoothDevice.h"
#include "BlueZConstants.h"
#include "BlueZDeviceManager.h"
#include "BlueZHostController.h"
#include "MediaEndpoint.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::bluetooth;
            using namespace sdkInterfaces::bluetooth;
            using namespace services;
            using namespace utils;
            using namespace logger;
            using namespace utils::bluetooth;
            static const char* MEDIATRANSPORT_PROPERTY_STATE = "State";
            static const char* OBJECT_PATH_ROOT = "/";
            static const int A2DP_CODEC_SBC = 0x00;
            static const int SBC_CAPS_ALL = 0xff;
            static const int SBC_BITPOOL_MIN = 2;
            static const int SBC_BITPOOL_MAX = 64;
            static const char* DBUS_ENDPOINT_PATH_SINK = "/com/amazon/alexa/sdk/sinkendpoint";
            static const string STATE_PENDING = "pending";
            static const string STATE_IDLE = "idle";
            static const string STATE_ACTIVE = "active";
            static const string TAG{"BlueZDeviceManager"};
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<BlueZDeviceManager> BlueZDeviceManager::create(shared_ptr<BluetoothEventBus> eventBus) {
                ACSDK_DEBUG5(LX(__func__));
                if (!eventBus) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "eventBus is nullptr"));
                    return nullptr;
                }
                shared_ptr<BlueZDeviceManager> deviceManager(new BlueZDeviceManager(eventBus));
                if (!deviceManager->init()) {
                    return nullptr;
                }
                return deviceManager;
            }
            bool BlueZDeviceManager::init() {
                ACSDK_DEBUG5(LX(__func__));
                ACSDK_DEBUG5(LX("Creating connection..."));
                m_connection = DBusConnection::create();
                if (nullptr == m_connection) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "Failed to create DBus connection"));
                    return false;
                }
                ACSDK_DEBUG5(LX("Creating ObjectManagerProxy..."));
                m_objectManagerProxy = DBusProxy::create(BlueZConstants::OBJECT_MANAGER_INTERFACE, OBJECT_PATH_ROOT);
                if (nullptr == m_objectManagerProxy) {
                    ACSDK_ERROR(LX("initFailed").d("Failed to create ObjectManager proxy", ""));
                    return false;
                }
                ACSDK_DEBUG5(LX("Retrieving BlueZ state..."));
                if (!getStateFromBlueZ()) return false;
                ACSDK_DEBUG5(LX("Initializing Host Controller..."));
                m_hostController = initializeHostController();
                if (!m_hostController) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "nullHostController"));
                    return false;
                }
                m_mediaProxy = DBusProxy::create(BlueZConstants::BLUEZ_MEDIA_INTERFACE, m_adapterPath);
                if (nullptr == m_mediaProxy) {
                    ACSDK_ERROR(LX("initializeMediaFailed").d("reason", "Failed to create Media proxy"));
                    return false;
                }
                m_workerContext = g_main_context_new();
                if (nullptr == m_workerContext) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "Failed to create glib main context"));
                    return false;
                }
                m_eventLoop = g_main_loop_new(m_workerContext, false);
                if (nullptr == m_eventLoop) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "Failed to create glib main loop"));
                    return false;
                }
                m_eventThread = thread(&BlueZDeviceManager::mainLoopThread, this);
                if (!m_mainLoopInitPromise.get_future().get()) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "failed to initialize glib main loop"));
                    return false;
                }
                ACSDK_DEBUG5(LX("BlueZDeviceManager initialized..."));
                BluetoothDeviceManagerInitializedEvent event;
                m_eventBus->sendEvent(event);
                return true;
            }
            bool BlueZDeviceManager::initializeMedia() {
                m_mediaEndpoint = make_shared<MediaEndpoint>(m_connection, DBUS_ENDPOINT_PATH_SINK);
                if (!m_mediaEndpoint->registerWithDBus()) {
                    ACSDK_ERROR(LX("initializeMediaFailed").d("reason", "registerEndpointFailed"));
                    return false;
                }
                ManagedGError error;
                GVariantBuilder* b;
                GVariantBuilder* capBuilder;
                capBuilder = g_variant_builder_new(G_VARIANT_TYPE("ay"));
                g_variant_builder_add(capBuilder, "y", SBC_CAPS_ALL);
                g_variant_builder_add(capBuilder, "y", SBC_CAPS_ALL);
                g_variant_builder_add(capBuilder, "y", SBC_BITPOOL_MIN);
                g_variant_builder_add(capBuilder, "y", SBC_BITPOOL_MAX);
                GVariant* caps = g_variant_builder_end(capBuilder);
                b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
                string a2dpSinkUuid = A2DPSinkInterface::UUID;
                transform(a2dpSinkUuid.begin(), a2dpSinkUuid.end(), a2dpSinkUuid.begin(), ::toupper);
                g_variant_builder_add(b, "{sv}", "UUID", g_variant_new_string(a2dpSinkUuid.c_str()));
                g_variant_builder_add(b, "{sv}", "Codec", g_variant_new_byte(A2DP_CODEC_SBC));
                g_variant_builder_add(b, "{sv}", "Capabilities", caps);
                GVariant* parameters = g_variant_builder_end(b);
                m_mediaProxy->callMethod("RegisterEndpoint", g_variant_new("(o@a{sv})", DBUS_ENDPOINT_PATH_SINK, parameters), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("initializeMediaFailed").d("Failed to register MediaEndpoint", ""));
                    return false;
                }
                return true;
            }
            shared_ptr<BlueZBluetoothDevice> BlueZDeviceManager::getDeviceByPath(const string& path) const {
                {
                    lock_guard<mutex> guard(m_devicesMutex);
                    auto iter = m_devices.find(path);
                    if (iter != m_devices.end()) return iter->second;
                }
                ACSDK_ERROR(LX("getDeviceByPathFailed").d("reason", "deviceNotFound").d("path", path));
                return nullptr;
            }
            void BlueZDeviceManager::onMediaStreamPropertyChanged(const string& path, const GVariantMapReader& changesMap) {
                ACSDK_DEBUG7(LX(__func__).d("path", path));
                const string FD_KEY = "/fd";
                auto pos = path.rfind(FD_KEY);
                if (string::npos == pos) {
                    ACSDK_ERROR(LX(__func__).d("reason", "unexpectedPath").d("path", path));
                    return;
                }
                string devicePath = path.substr(0, pos);
                auto device = getDeviceByPath(devicePath);
                if (!device) {
                    ACSDK_ERROR(LX(__func__).d("reason", "deviceDoesNotExist").d("path", devicePath));
                    return;
                }
                auto mediaTransportProperties = DBusPropertiesProxy::create(path);
                if (!mediaTransportProperties) {
                    ACSDK_ERROR(LX(__func__).d("reason", "nullPropertiesProxy").d("path", path));
                    return;
                }
                string uuid;
                if (!mediaTransportProperties->getStringProperty(BlueZConstants::BLUEZ_MEDIATRANSPORT_INTERFACE, "UUID", &uuid)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "getPropertyFailed"));
                    return;
                }
                transform(uuid.begin(), uuid.end(), uuid.begin(), ::tolower);
                ACSDK_DEBUG5(LX(__func__).d("mediaStreamUuid", uuid));
                char* newStateStr;
                if (!changesMap.getCString(MEDIATRANSPORT_PROPERTY_STATE, &newStateStr)) {
                    ACSDK_DEBUG5(LX("mediaTransportStateUnchanged").d("action", "ignoringCallback"));
                    return;
                }
                ACSDK_DEBUG5(LX("mediaTransportStateChanged").d("newState", newStateStr));
                MediaStreamingState newState = MediaStreamingState::IDLE;
                if (STATE_ACTIVE == newStateStr) newState = MediaStreamingState::ACTIVE;
                else if (STATE_PENDING == newStateStr) newState = MediaStreamingState::PENDING;
                else if (STATE_IDLE == newStateStr) newState = MediaStreamingState::IDLE;
                else {
                    ACSDK_ERROR(LX("onMediaStreamPropertyChangedFailed").d("unknownState", newStateStr));
                    return;
                }
                if (A2DPSourceInterface::UUID == uuid) {
                    auto sink = device->getService(A2DPSinkInterface::UUID);
                    if (!sink) {
                        ACSDK_ERROR(LX(__func__).d("reason", "nullSink"));
                        return;
                    }
                    MediaStreamingStateChangedEvent event(newState, A2DPRole::SOURCE, device);
                    m_eventBus->sendEvent(event);
                    return;
                } else if (A2DPSinkInterface::UUID == uuid) {
                    if (path != m_mediaEndpoint->getStreamingDevicePath()) {
                        ACSDK_DEBUG5(LX(__func__).d("reason", "pathMismatch").d("path", path).d("streamingDevicePath", m_mediaEndpoint->getStreamingDevicePath()));
                        return;
                    }
                    if (m_streamingState == newState) return;
                    m_streamingState = newState;
                    m_mediaEndpoint->onMediaTransportStateChanged(newState, path);
                    MediaStreamingStateChangedEvent event(newState, A2DPRole::SINK, device);
                    m_eventBus->sendEvent(event);
                }
            }
            void BlueZDeviceManager::onDevicePropertyChanged(const string& path, const GVariantMapReader& changesMap) {
                ACSDK_DEBUG7(LX(__func__).d("path", path));
                shared_ptr<BlueZBluetoothDevice> device = getDeviceByPath(path);
                if (!device) {
                    ACSDK_ERROR(LX("onDevicePropertyChangedFailed").d("reason", "device not found"));
                    return;
                }
                device->onPropertyChanged(changesMap);
                ACSDK_DEBUG7(LX(__func__).d("finished", "ok"));
            }
            void BlueZDeviceManager::onAdapterPropertyChanged(const string& path, const GVariantMapReader& changesMap) {
                ACSDK_DEBUG7(LX(__func__).d("path", path));
                if (!m_hostController) {
                    ACSDK_ERROR(LX("onAdapterPropertyChangedFailed").d("reason", "nullHostController"));
                    return;
                }
                m_hostController->onPropertyChanged(changesMap);
            }
            string BlueZDeviceManager::getAdapterPath() const {
                return m_adapterPath;
            }
            MediaStreamingState BlueZDeviceManager::getMediaStreamingState() {
                return m_streamingState;
            }
            void BlueZDeviceManager::interfacesAddedCallback(
                GDBusConnection* conn,
                const gchar* sender_name,
                const gchar* object_path,
                const gchar* interface_name,
                const gchar* signal_name,
                GVariant* parameters,
                gpointer deviceManager) {
                ACSDK_DEBUG5(LX(__func__));
                if (nullptr == parameters) {
                    ACSDK_ERROR(LX("interfacesAddedCallbackFailed").d("reason", "parameters are null"));
                    return;
                }
                if (nullptr == deviceManager) {
                    ACSDK_ERROR(LX("interfacesAddedCallbackFailed").d("reason", "deviceManager is null"));
                    return;
                }
                GVariantTupleReader tupleReader(parameters);
                char* addedObjectPath = tupleReader.getObjectPath(0);
                ManagedGVariant interfacesChangedMap = tupleReader.getVariant(1);
                static_cast<BlueZDeviceManager*>(deviceManager)->onInterfaceAdded(addedObjectPath, interfacesChangedMap);
            }
            void BlueZDeviceManager::interfacesRemovedCallback(
                GDBusConnection* conn,
                const gchar* sender_name,
                const gchar* object_path,
                const gchar* interface_name,
                const gchar* signal_name,
                GVariant* variant,
                gpointer deviceManager) {
                ACSDK_DEBUG5(LX(__func__));
                char* interfaceRemovedPath;
                if (nullptr == variant) {
                    ACSDK_ERROR(LX("interfacesRemovedCallbackFailed").d("reason", "variant is null"));
                    return;
                }
                if (nullptr == deviceManager) {
                    ACSDK_ERROR(LX("interfacesRemovedCallbackFailed").d("reason", "deviceManager is null"));
                    return;
                }
                g_variant_get(variant, "(oas)", &interfaceRemovedPath, NULL);
                static_cast<BlueZDeviceManager*>(deviceManager)->onInterfaceRemoved(interfaceRemovedPath);
            }
            void BlueZDeviceManager::propertiesChangedCallback(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path, const gchar* interface_name,
                                                               const gchar* signal_name, GVariant* prop, gpointer deviceManager) {
                ACSDK_DEBUG5(LX(__func__));
                if (nullptr == prop) {
                    ACSDK_ERROR(LX("propertiesChangedCallbackFailed").d("reason", "variant is null"));
                    return;
                }
                if (nullptr == object_path) {
                    ACSDK_ERROR(LX("propertiesChangedCallbackFailed").d("reason", "object_path is null"));
                    return;
                }
                if (nullptr == deviceManager) {
                    ACSDK_ERROR(LX("propertiesChangedCallbackFailed").d("reason", "deviceManager is null"));
                    return;
                }
                ACSDK_DEBUG7(LX("Properties changed").d("objectPath", object_path));
                ACSDK_DEBUG9(LX("Details").d("", g_variant_print(prop, true)));
                char* propertyOwner;
                GVariantTupleReader tupleReader(prop);
                propertyOwner = tupleReader.getCString(0);
                ManagedGVariant propertyMapVariant = tupleReader.getVariant(1);
                GVariantMapReader mapReader(propertyMapVariant);
                static_cast<BlueZDeviceManager*>(deviceManager)->onPropertiesChanged(propertyOwner, object_path, mapReader);
            }
            void BlueZDeviceManager::onPropertiesChanged(const tring& propertyOwner, const string& objectPath, const GVariantMapReader& changesMap) {
                if (BlueZConstants::BLUEZ_MEDIATRANSPORT_INTERFACE == propertyOwner) onMediaStreamPropertyChanged(objectPath, changesMap);
                else if (BlueZConstants::BLUEZ_DEVICE_INTERFACE == propertyOwner) onDevicePropertyChanged(objectPath, changesMap);
                else if (BlueZConstants::BLUEZ_ADAPTER_INTERFACE == propertyOwner) onAdapterPropertyChanged(objectPath, changesMap);
            }
            void BlueZDeviceManager::onInterfaceAdded(const char* objectPath, ManagedGVariant& interfacesChangedMap) {
                ACSDK_DEBUG7(LX(__func__).d("path", objectPath));
                ACSDK_DEBUG9(LX(__func__).d("Details", g_variant_print(interfacesChangedMap.get(), true)));
                GVariantMapReader mapReader(interfacesChangedMap.get());
                ManagedGVariant deviceInterfaceObject = mapReader.getVariant(BlueZConstants::BLUEZ_DEVICE_INTERFACE);
                if (deviceInterfaceObject.get() != nullptr) {
                    shared_ptr<BlueZBluetoothDevice> device = addDeviceFromDBusObject(objectPath, deviceInterfaceObject.get());
                    notifyDeviceAdded(device);
                }
            }
            void BlueZDeviceManager::onInterfaceRemoved(const char* objectPath) {
                ACSDK_DEBUG7(LX(__func__));
                removeDevice(objectPath);
            }
            void BlueZDeviceManager::addDevice(const char* devicePath, std::shared_ptr<BlueZBluetoothDevice> device) {
                ACSDK_DEBUG7(LX(__func__));
                if (nullptr == devicePath) { ACSDK_ERROR(LX("addDeviceFailed").d("reason", "devicePath is null")); }
                if (nullptr == device) { ACSDK_ERROR(LX("addDeviceFailed").d("reason", "device is null")); }
                {
                    lock_guard<std::mutex> guard(m_devicesMutex);
                    m_devices[devicePath] = device;
                }
                ACSDK_DEBUG7(LX("Device added").d("path", devicePath).d("mac", device->getMac()).d("alias", device->getFriendlyName()));
            }
            void BlueZDeviceManager::notifyDeviceAdded(std::shared_ptr<BlueZBluetoothDevice> device) {
                ACSDK_DEBUG7(LX(__func__));
                DeviceDiscoveredEvent event(device);
                m_eventBus->sendEvent(event);
            }
            void BlueZDeviceManager::removeDevice(const char* devicePath) {
                if (nullptr == devicePath) { ACSDK_ERROR(LX("removeDeviceFailed").d("reason", "devicePath is null")); }
                ACSDK_DEBUG5(LX("Removing device").d("device path", devicePath));
                shared_ptr<BluetoothDeviceInterface> device;
                {
                    lock_guard<mutex> guard(m_devicesMutex);
                    auto it = m_devices.find(devicePath);
                    if (it != m_devices.end()) {
                        device = it->second;
                        m_devices.erase(it);
                    }
                }
                if (device) {
                    DeviceRemovedEvent event(device);
                    m_eventBus->sendEvent(event);
                }
            }
            void BlueZDeviceManager::doShutdown() {
                ACSDK_DEBUG5(LX(__func__));
                {
                    lock_guard<std::mutex> guard(m_devicesMutex);
                    for (auto iter : m_devices) {
                        shared_ptr<BlueZBluetoothDevice> device = iter.second;
                        device->disconnect().get();
                    }
                    m_devices.clear();
                }
                finalizeMedia();
                m_pairingAgent.reset();
                m_mediaPlayer.reset();
                m_connection->close();
                if (m_eventLoop) g_main_loop_quit(m_eventLoop);
                if (m_eventThread.joinable()) m_eventThread.join();
            }
            BlueZDeviceManager::~BlueZDeviceManager() {
                ACSDK_DEBUG5(LX(__func__));
            }
            BlueZDeviceManager::BlueZDeviceManager(const shared_ptr<BluetoothEventBus>& eventBus) : RequiresShutdown{"BlueZDeviceManager"},
                                                   m_eventBus{eventBus}, m_streamingState{MediaStreamingState::IDLE} {}
            bool BlueZDeviceManager::getStateFromBlueZ() {
                ManagedGError error;
                ManagedGVariant managedObjectsVar = m_objectManagerProxy->callMethod("GetManagedObjects", nullptr, error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("initializeKnownDevicesFailed").d("error", error.getMessage()));
                    return false;
                }
                GVariantTupleReader resultReader(managedObjectsVar);
                ManagedGVariant managedObjectsMap = resultReader.getVariant(0);
                GVariantMapReader mapReader(managedObjectsMap, true);
                mapReader.forEach([this](char* objectPath, GVariant* dbusObject) -> bool {
                    GVariantMapReader supportedInterfacesMap(dbusObject);
                    if (m_adapterPath.empty()) {
                        ManagedGVariant adapterInterfaceVar = supportedInterfacesMap.getVariant(BlueZConstants::BLUEZ_ADAPTER_INTERFACE);
                        if (adapterInterfaceVar.hasValue()) {
                            ACSDK_DEBUG3(LX("Found bluetooth adapter").d("Path", objectPath));
                            m_adapterPath = objectPath;
                        }
                    }
                    ManagedGVariant deviceInterfaceVar = supportedInterfacesMap.getVariant(BlueZConstants::BLUEZ_DEVICE_INTERFACE);
                    if (deviceInterfaceVar.hasValue()) auto device = addDeviceFromDBusObject(objectPath, deviceInterfaceVar.get());
                    return true;
                });
                return true;
            }
            shared_ptr<BlueZBluetoothDevice> BlueZDeviceManager::addDeviceFromDBusObject(const char* objectPath, GVariant* dbusObject) {
                if (nullptr == objectPath) { ACSDK_ERROR(LX("addDeviceFromDBusObjectFailed").d("reason", "objectPath is null")); }
                if (nullptr == dbusObject) { ACSDK_ERROR(LX("addDeviceFromDBusObjectFailed").d("reason", "dbusObject is null")); }
                GVariantMapReader deviceMapReader(dbusObject);
                char* macAddress = nullptr;
                if (!deviceMapReader.getCString(BlueZConstants::BLUEZ_DEVICE_INTERFACE_ADDRESS, &macAddress))  return nullptr;
                shared_ptr<BlueZBluetoothDevice> knownDevice = BlueZBluetoothDevice::create(macAddress, objectPath, shared_from_this());
                if (knownDevice) addDevice(objectPath, knownDevice);
                return knownDevice;
            }
            list<shared_ptr<BluetoothDeviceInterface>> BlueZDeviceManager::
                getDiscoveredDevices() {
                ACSDK_DEBUG5(LX(__func__));
                list<shared_ptr<BluetoothDeviceInterface>> newList;
                lock_guard<mutex> guard(m_devicesMutex);
                for (const auto& it : m_devices) newList.push_back(static_pointer_cast<BluetoothDeviceInterface>(it.second));
                return newList;
            }
            shared_ptr<BlueZHostController> BlueZDeviceManager::initializeHostController() {
                return BlueZHostController::create(m_adapterPath);
            }
            shared_ptr<MediaEndpoint> BlueZDeviceManager::getMediaEndpoint() {
                return m_mediaEndpoint;
            }
            shared_ptr<BluetoothHostControllerInterface> BlueZDeviceManager::getHostController() {
                return m_hostController;
            }
            shared_ptr<BluetoothEventBus> BlueZDeviceManager::getEventBus() {
                return m_eventBus;
            }
            bool BlueZDeviceManager::finalizeMedia() {
                ManagedGError error;
                m_mediaProxy->callMethod("UnregisterEndpoint", g_variant_new("(o)", DBUS_ENDPOINT_PATH_SINK), error.toOutputParameter());
                if (error.hasError()) {
                    ACSDK_ERROR(LX("finalizeMediaFailed").d("reason", "Failed to unregister MediaEndpoint"));
                    return false;
                }
                m_mediaEndpoint.reset();
                return true;
            }
            void BlueZDeviceManager::mainLoopThread() {
                g_main_context_push_thread_default(m_workerContext);
                ACSDK_DEBUG5(LX("Connecting signals..."));
                do {
                    int subscriptionId = m_connection->subscribeToSignal(BlueZConstants::BLUEZ_SERVICE_NAME, BlueZConstants::OBJECT_MANAGER_INTERFACE,
                                                                "InterfacesAdded",nullptr,BlueZDeviceManager::interfacesAddedCallback,
                                                                this);
                    if (0 == subscriptionId) {
                        ACSDK_ERROR(LX("initFailed").d("reason", "failed to subscribe to InterfacesAdded signal"));
                        m_mainLoopInitPromise.set_value(false);
                        break;
                    }
                    subscriptionId = m_connection->subscribeToSignal(BlueZConstants::BLUEZ_SERVICE_NAME, BlueZConstants::OBJECT_MANAGER_INTERFACE,
                                                             "InterfacesRemoved",nullptr, BlueZDeviceManager::interfacesRemovedCallback,
                                                                     this);
                    if (0 == subscriptionId) {
                        ACSDK_ERROR(LX("initFailed").d("reason", "failed to subscribe to InterfacesRemoved signal"));
                        m_mainLoopInitPromise.set_value(false);
                        break;
                    }
                    subscriptionId = m_connection->subscribeToSignal(BlueZConstants::BLUEZ_SERVICE_NAME, BlueZConstants::PROPERTIES_INTERFACE,
                                                             "PropertiesChanged",nullptr, BlueZDeviceManager::propertiesChangedCallback,
                                                            this);
                    if (0 == subscriptionId) {
                        ACSDK_ERROR(LX("initFailed").d("reason", "failed to subscribe to PropertiesChanged signal"));
                        m_mainLoopInitPromise.set_value(false);
                        break;
                    }
                    ACSDK_DEBUG5(LX("init").m("Initializing Bluetooth Media"));
                    if (!initializeMedia()) {
                        ACSDK_ERROR(LX("initFailed").d("reason", "initBluetoothMediaFailed"));
                        m_mainLoopInitPromise.set_value(false);
                        break;
                    }
                    ACSDK_DEBUG5(LX("init").m("Initializing Pairing Agent"));
                    m_pairingAgent = PairingAgent::create(m_connection);
                    if (!m_pairingAgent) {
                        ACSDK_ERROR(LX("initFailed").d("reason", "initPairingAgentFailed"));
                        m_mainLoopInitPromise.set_value(false);
                        break;
                    }
                    ACSDK_DEBUG5(LX("init").m("Initializing MRPIS Player"));
                    m_mediaPlayer = MPRISPlayer::create(m_connection, m_mediaProxy, m_eventBus);
                    if (!m_mediaPlayer) {
                        ACSDK_ERROR(LX("initFailed").d("reason", "initMediaPlayerFailed"));
                        m_mainLoopInitPromise.set_value(false);
                        break;
                    }
                    m_mainLoopInitPromise.set_value(true);
                    g_main_loop_run(m_eventLoop);
                } while(false);
                g_main_loop_unref(m_eventLoop);
                g_main_context_pop_thread_default(m_workerContext);
                g_main_context_unref(m_workerContext);
            }
        }
    }
}
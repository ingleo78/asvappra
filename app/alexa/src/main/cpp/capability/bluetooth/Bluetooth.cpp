#include <chrono>
#include <avs/ContentType.h>
#include <avs/NamespaceAndName.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSinkInterface.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPControllerInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include <sdkinterfaces/Bluetooth/Services/HFPInterface.h>
#include <sdkinterfaces/Bluetooth/Services/HIDInterface.h>
#include <sdkinterfaces/Bluetooth/Services/SPPInterface.h>
#include <util/bluetooth/SDPRecords.h>
#include <configuration/ConfigurationNode.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <util/MacAddressString.h>
#include <uuid_generation/UUIDGeneration.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "BasicDeviceConnectionRule.h"
#include "Bluetooth.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace chrono;
        using namespace logger;
        using namespace sdkInterfaces::bluetooth::services;
        using MediaPlayerState = MediaPlayerState;
        static const string TAG{"Bluetooth"};
        #define LX(event) LogEntry(TAG, event)
        static const string NAMESPACE = "Bluetooth";
        static const NamespaceAndName BLUETOOTH_STATE{NAMESPACE, "BluetoothState"};
        static const NamespaceAndName SCAN_DEVICES{NAMESPACE, "ScanDevices"};
        static const NamespaceAndName SCAN_DEVICES_REPORT{NAMESPACE, "ScanDevicesReport"};
        static const NamespaceAndName SCAN_DEVICES_FAILED{NAMESPACE, "ScanDevicesFailed"};
        static const NamespaceAndName ENTER_DISCOVERABLE_MODE{NAMESPACE, "EnterDiscoverableMode"};
        static const NamespaceAndName ENTER_DISCOVERABLE_MODE_SUCCEEDED{NAMESPACE, "EnterDiscoverableModeSucceeded"};
        static const NamespaceAndName ENTER_DISCOVERABLE_MODE_FAILED{NAMESPACE, "EnterDiscoverableModeFailed"};
        static const NamespaceAndName EXIT_DISCOVERABLE_MODE{NAMESPACE, "ExitDiscoverableMode"};
        static const NamespaceAndName PAIR_DEVICES_SUCCEEDED{NAMESPACE, "PairDevicesSucceeded"};
        static const NamespaceAndName PAIR_DEVICES_FAILED{NAMESPACE, "PairDevicesFailed"};
        static const NamespaceAndName PAIR_DEVICES{NAMESPACE, "PairDevices"};
        static const NamespaceAndName UNPAIR_DEVICES{NAMESPACE, "UnpairDevices"};
        static const NamespaceAndName UNPAIR_DEVICES_SUCCEEDED{NAMESPACE, "UnpairDevicesSucceeded"};
        static const NamespaceAndName UNPAIR_DEVICES_FAILED{NAMESPACE, "UnpairDevicesFailed"};
        static const NamespaceAndName SET_DEVICE_CATEGORIES{NAMESPACE, "SetDeviceCategories"};
        static const NamespaceAndName SET_DEVICE_CATEGORIES_SUCCEEDED{NAMESPACE, "SetDeviceCategoriesSucceeded"};
        static const NamespaceAndName SET_DEVICE_CATEGORIES_FAILED{NAMESPACE, "SetDeviceCategoriesFailed"};
        static const NamespaceAndName CONNECT_BY_DEVICE_IDS{NAMESPACE, "ConnectByDeviceIds"};
        static const NamespaceAndName CONNECT_BY_DEVICE_IDS_SUCCEEDED{NAMESPACE, "ConnectByDeviceIdsSucceeded"};
        static const NamespaceAndName CONNECT_BY_DEVICE_IDS_FAILED{NAMESPACE, "ConnectByDeviceIdsFailed"};
        static const NamespaceAndName CONNECT_BY_PROFILE{NAMESPACE, "ConnectByProfile"};
        static const NamespaceAndName CONNECT_BY_PROFILE_SUCCEEDED{NAMESPACE, "ConnectByProfileSucceeded"};
        static const NamespaceAndName CONNECT_BY_PROFILE_FAILED{NAMESPACE, "ConnectByProfileFailed"};
        static const NamespaceAndName DISCONNECT_DEVICES{NAMESPACE, "DisconnectDevices"};
        static const NamespaceAndName DISCONNECT_DEVICES_SUCCEEDED{NAMESPACE, "DisconnectDevicesSucceeded"};
        static const NamespaceAndName DISCONNECT_DEVICES_FAILED{NAMESPACE, "DisconnectDevicesFailed"};
        static const NamespaceAndName PLAY{NAMESPACE, "Play"};
        static const NamespaceAndName MEDIA_CONTROL_PLAY_SUCCEEDED{NAMESPACE, "MediaControlPlaySucceeded"};
        static const NamespaceAndName MEDIA_CONTROL_PLAY_FAILED{NAMESPACE, "MediaControlPlayFailed"};
        static const NamespaceAndName STOP{NAMESPACE, "Stop"};
        static const NamespaceAndName MEDIA_CONTROL_STOP_SUCCEEDED{NAMESPACE, "MediaControlStopSucceeded"};
        static const NamespaceAndName MEDIA_CONTROL_STOP_FAILED{NAMESPACE, "MediaControlStopFailed"};
        static const NamespaceAndName NEXT{NAMESPACE, "Next"};
        static const NamespaceAndName MEDIA_CONTROL_NEXT_SUCCEEDED{NAMESPACE, "MediaControlNextSucceeded"};
        static const NamespaceAndName MEDIA_CONTROL_NEXT_FAILED{NAMESPACE, "MediaControlNextFailed"};
        static const NamespaceAndName PREVIOUS{NAMESPACE, "Previous"};
        static const NamespaceAndName MEDIA_CONTROL_PREVIOUS_SUCCEEDED{NAMESPACE, "MediaControlPreviousSucceeded"};
        static const NamespaceAndName MEDIA_CONTROL_PREVIOUS_FAILED{NAMESPACE, "MediaControlPreviousFailed"};
        static const NamespaceAndName STREAMING_STARTED{NAMESPACE, "StreamingStarted"};
        static const NamespaceAndName STREAMING_ENDED{NAMESPACE, "StreamingEnded"};
        static const string CHANNEL_NAME = FocusManagerInterface::CONTENT_CHANNEL_NAME;
        static const string ACTIVITY_ID = "Bluetooth";
        static const unsigned int MAX_CONNECT_BY_PROFILE_COUNT = 2;
        static const milliseconds INITIALIZE_SOURCE_DELAY{1000};
        static const seconds DEFAULT_FUTURE_TIMEOUT{45};
        static const minutes TRANSIENT_FOCUS_DURATION{2};
        static const char ALEXA_DEVICE_NAME_KEY[] = "alexaDevice";
        static const char DEVICE_KEY[] = "device";
        static const char DEVICES_KEY[] = "devices";
        static const char DISCOVERED_DEVICES_KEY[] = "discoveredDevices";
        static const char HAS_MORE_KEY[] = "hasMore";
        static const string EMPTY_PAYLOAD = "{}";
        static const char FRIENDLY_NAME_KEY[] = "friendlyName";
        static const char NAME_KEY[] = "name";
        static const char VERSION_KEY[] = "version";
        static const char PAIRED_DEVICES_KEY[] = "pairedDevices";
        static const char PROFILE_KEY[] = "profile";
        static const char PROFILES_KEY[] = "profiles";
        static const char PROFILE_NAME_KEY[] = "profileName";
        static const char REQUESTER_KEY[] = "requester";
        static const char STREAMING_KEY[] = "streaming";
        static const char STATE_KEY[] = "state";
        static const char SUPPORTED_PROFILES_KEY[] = "supportedProfiles";
        static const char TRUNCATED_MAC_ADDRESS_KEY[] = "truncatedMacAddress";
        static const char UNIQUE_DEVICE_ID_KEY[] = "uniqueDeviceId";
        static const char METADATA_KEY[] = "metadata";
        static const char VENDOR_ID_KEY[] = "vendorId";
        static const char PRODUCT_ID_KEY[] = "productId";
        static const char CLASS_OF_DEVICE_KEY[] = "classOfDevice";
        static const char VENDOR_DEVICE_SIG_ID_KEY[] = "vendorDeviceSigId";
        static const char VENDOR_DEVICE_ID_KEY[] = "vendorDeviceId";
        static const char DEVICE_CATEGORY_KEY[] = "deviceCategory";
        static const char CONNECTION_STATE_KEY[] = "connectionState";
        static const string AVS_A2DP_SOURCE = "A2DP-SOURCE";
        static const string AVS_A2DP_SINK = "A2DP-SINK";
        static const string AVS_A2DP = "A2DP";
        static const string AVS_AVRCP = "AVRCP";
        static const string AVS_HFP = "HFP";
        static const string AVS_HID = "HID";
        static const string AVS_SPP = "SPP";
        static const map<string, string> AVS_PROFILE_MAP{{string(A2DPSourceInterface::UUID), AVS_A2DP_SOURCE}, {string(A2DPSinkInterface::UUID), AVS_A2DP_SINK},
                                                         {string(AVRCPTargetInterface::UUID), AVS_AVRCP}, {string(HFPInterface::UUID), AVS_HFP},
                                                         {string(HIDInterface::UUID), AVS_HID}, {string(SPPInterface::UUID), AVS_SPP}};
        static const map<DeviceCategory, unordered_set<string>> DEVICECATEGORY_PROFILES_MAP{
            {DeviceCategory::REMOTE_CONTROL, {string(SPPInterface::UUID), string(HIDInterface::UUID)}},
            {DeviceCategory::GADGET, {string(SPPInterface::UUID), string(HIDInterface::UUID)}}};
        static const string BLUETOOTH_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
        static const string BLUETOOTH_CAPABILITY_INTERFACE_NAME = "Bluetooth";
        static const string BLUETOOTH_CAPABILITY_INTERFACE_VERSION = "2.0";
        static const string BLUETOOTH_CAPABILITY_CONFIGURATION_VALUE = R"({"profiles": ["AVRCP","A2DP_SINK","A2DP_SOURCE"]})";
        static shared_ptr<CapabilityConfiguration> getBluetoothCapabilityConfiguration() {
            unordered_map<string, string> configMap;
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, BLUETOOTH_CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, BLUETOOTH_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, BLUETOOTH_CAPABILITY_INTERFACE_VERSION});
            configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, BLUETOOTH_CAPABILITY_CONFIGURATION_VALUE});
            return make_shared<CapabilityConfiguration>(configMap);
        }
        static string truncateWithDefault(const string& mac, const string& defaultString = "XX:XX:XX:XX:XX:XX") {
            unique_ptr<MacAddressString> macAddressString = MacAddressString::create(mac);
            if (macAddressString) return macAddressString->getTruncatedString();
            else return defaultString;
        }
        static string truncateFriendlyName(const string& friendlyName) {
            return truncateWithDefault(friendlyName, friendlyName);
        }
        static bool supportsAvsProfile(shared_ptr<BluetoothDeviceInterface> device, const string& avsProfileName) {
            ACSDK_DEBUG5(LX(__func__).d("avsProfileName", avsProfileName));
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return false;
            } else if (avsProfileName.empty()) {
                ACSDK_ERROR(LX(__func__).d("reason", "emptyProfile"));
                return false;
            }
            for (const auto& sdpRecord : device->getSupportedServices()) {
                auto avsProfileIt = AVS_PROFILE_MAP.find(sdpRecord->getUuid());
                if (avsProfileIt != AVS_PROFILE_MAP.end()) {
                    if (avsProfileIt->second == avsProfileName) return true;
                    else if (avsProfileIt->second == AVS_A2DP_SINK && avsProfileName == AVS_A2DP) return true;
                    else if (avsProfileIt->second == AVS_A2DP_SOURCE && avsProfileName == AVS_A2DP) return true;
                }
            }
            ACSDK_DEBUG5(LX(__func__).d("reason", "profileNotSupported").d("deviceMac", truncateWithDefault(device->getMac())).d("avsProfile", avsProfileName));
            return false;
        }
        static bool waitOnFuture(future<bool> future, string description = "", milliseconds timeout = DEFAULT_FUTURE_TIMEOUT) {
            if (future.valid()) {
                future_status status = future.wait_for(timeout);
                switch(status) {
                    case future_status::timeout: ACSDK_ERROR(LX(__func__).d("description", description).m("Timeout waiting on a future.")); break;
                    case future_status::deferred: ACSDK_WARN(LX(__func__).d("description", description).m("Blocking on a deferred future."));
                    case future_status::ready: return future.get();
                }
            } else { ACSDK_ERROR(LX(__func__).d("description", description).m("Cannot wait on invalid future.")); }
            return false;
        }
        static string convertToAVSStreamingState(const Bluetooth::StreamingState& state) {
            switch(state) {
                case Bluetooth::StreamingState::INACTIVE: return streamingStateToString(Bluetooth::StreamingState::INACTIVE);
                case Bluetooth::StreamingState::PAUSED: return streamingStateToString(Bluetooth::StreamingState::PAUSED);
                case Bluetooth::StreamingState::PENDING_ACTIVE: case Bluetooth::StreamingState::PENDING_PAUSED: case Bluetooth::StreamingState::ACTIVE:
                    return streamingStateToString(Bluetooth::StreamingState::ACTIVE);
            }
            return "UNKNOWN";
        }
        static bool validateDeviceConnectionRules(unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules) {
            set<DeviceCategory> enabledDeviceCategories;
            if (!enabledConnectionRules.empty()) {
                for (const auto& connectionRule : enabledConnectionRules) {
                    set<DeviceCategory> categories = connectionRule->getDeviceCategories();
                    set<string> dependentProfiles = connectionRule->getDependentProfiles();
                    for (const auto& category : categories) {
                        if (enabledDeviceCategories.find(category) != enabledDeviceCategories.end()) {
                            ACSDK_ERROR(LX(__func__).d("reason", "RedefinedDeviceCategory").d("category", category));
                            return false;
                        }
                        enabledDeviceCategories.insert(category);
                        if (DEVICECATEGORY_PROFILES_MAP.find(category) != DEVICECATEGORY_PROFILES_MAP.end()) {
                            unordered_set<string> requiredProfiles = DEVICECATEGORY_PROFILES_MAP.at(category);
                            for (const auto& requiredProfile : requiredProfiles) {
                                if (dependentProfiles.count(requiredProfile) == 0) {
                                    ACSDK_ERROR(LX(__func__).d("reason", "RequiredProfileNotAdded").d("uuid", requiredProfile));
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }
        static string convertToAVSConnectedState(const DeviceState& state) {
            switch(state) {
                case DeviceState::CONNECTED: return "CONNECTED";
                case DeviceState::FOUND: case DeviceState::IDLE: case DeviceState::PAIRED: case DeviceState::UNPAIRED: case DeviceState::DISCONNECTED:
                    return "DISCONNECTED";
            }
            return "UNKNOWN";
        }
        shared_ptr<Bluetooth> Bluetooth::create(shared_ptr<ContextManagerInterface> contextManager, shared_ptr<FocusManagerInterface> focusManager,
                                                shared_ptr<MessageSenderInterface> messageSender, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                shared_ptr<BluetoothStorageInterface> bluetoothStorage, unique_ptr<BluetoothDeviceManagerInterface> deviceManager,
                                                shared_ptr<BluetoothEventBus> eventBus, shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                shared_ptr<CustomerDataManager> customerDataManager, unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules,
                                                shared_ptr<ChannelVolumeInterface> bluetoothChannelVolumeInterface, shared_ptr<BluetoothMediaInputTransformer> mediaInputTransformer) {
            ACSDK_DEBUG5(LX(__func__));
            if (!contextManager) { ACSDK_ERROR(LX(__func__).d("reason", "nullContextManager")); }
            else if (!focusManager) { ACSDK_ERROR(LX(__func__).d("reason", "nullFocusManager")); }
            else if (!messageSender) { ACSDK_ERROR(LX(__func__).d("reason", "nullMessageSender")); }
            else if (!exceptionEncounteredSender) { ACSDK_ERROR(LX(__func__).d("reason", "nullExceptionEncounteredSender")); }
            else if (!bluetoothStorage) { ACSDK_ERROR(LX(__func__).d("reason", "nullBluetoothStorage")); }
            else if (!deviceManager) { ACSDK_ERROR(LX(__func__).d("reason", "nullDeviceManager")); }
            else if (!eventBus) { ACSDK_ERROR(LX(__func__).d("reason", "nullEventBus")); }
            else if (!mediaPlayer) { ACSDK_ERROR(LX(__func__).d("reason", "nullMediaPlayer")); }
            else if (!customerDataManager) { ACSDK_ERROR(LX(__func__).d("reason", "nullCustomerDataManager")); }
            else if (!validateDeviceConnectionRules(enabledConnectionRules)) { ACSDK_ERROR(LX(__func__).d("reason", "invalidBluetoothDeviceConnectionRules")); }
            else if (!bluetoothChannelVolumeInterface) { ACSDK_ERROR(LX(__func__).d("reason", "nullBluetoothChannelVolumeInterface")); }
            else {
                auto bluetooth = shared_ptr<Bluetooth>(new Bluetooth(contextManager, focusManager, messageSender, exceptionEncounteredSender,
                                                       bluetoothStorage, deviceManager, eventBus, mediaPlayer,
                                                       customerDataManager, enabledConnectionRules, bluetoothChannelVolumeInterface, mediaInputTransformer));
                if (bluetooth->init()) return bluetooth;
                else { ACSDK_ERROR(LX(__func__).d("reason", "initFailed")); }
            }
            return nullptr;
        }
        bool Bluetooth::init() {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_db->open()) {
                ACSDK_INFO(LX("init").m("Couldn't open database.  Creating."));
                if (!m_db->createDatabase()) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "createDatabaseFailed"));
                    return false;
                }
            }
            m_mediaPlayer->addObserver(shared_from_this());
            syncWithDeviceManager();
            executeUpdateContext();
            m_eventBus->addListener({BluetoothEventType::DEVICE_DISCOVERED, BluetoothEventType::DEVICE_STATE_CHANGED, BluetoothEventType::STREAMING_STATE_CHANGED,
                                    BluetoothEventType::SCANNING_STATE_CHANGED, BluetoothEventType::TOGGLE_A2DP_PROFILE_STATE_CHANGED},shared_from_this());
            return true;
        }
        void Bluetooth::syncWithDeviceManager() {
            for (const auto& device : m_deviceManager->getDiscoveredDevices()) {
                if (!device->isConnected()) {
                    ACSDK_DEBUG9(LX(__func__).d("reason", "deviceNotConnected").m("Excluding"));
                    continue;
                }
                executeOnDeviceConnect(device, false);
            }
            if (m_activeA2DPDevice && m_activeA2DPDevice->getService(A2DPSourceInterface::UUID)) {
                auto streamingState = m_activeA2DPDevice->getStreamingState();
                switch(streamingState) {
                    case MediaStreamingState::IDLE: m_streamingState = StreamingState::PAUSED; break;
                    case MediaStreamingState::PENDING: m_streamingState = StreamingState::INACTIVE; break;
                    case MediaStreamingState::ACTIVE:
                        m_streamingState = StreamingState::ACTIVE;
                        m_executor.submit([this] { executeAcquireFocus(__func__); });
                        break;
                }
            }
        }
        Bluetooth::Bluetooth(shared_ptr<ContextManagerInterface> contextManager, shared_ptr<FocusManagerInterface> focusManager,
                             shared_ptr<MessageSenderInterface> messageSender, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                             shared_ptr<BluetoothStorageInterface> bluetoothStorage, unique_ptr<BluetoothDeviceManagerInterface>& deviceManager,
                             shared_ptr<BluetoothEventBus> eventBus, shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<CustomerDataManager> customerDataManager,
                             unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules,
                             shared_ptr<ChannelVolumeInterface> bluetoothChannelVolumeInterface, shared_ptr<BluetoothMediaInputTransformer> mediaInputTransformer) :
                             CapabilityAgent{NAMESPACE, exceptionEncounteredSender}, RequiresShutdown{"Bluetooth"}, CustomerDataHandler{customerDataManager},
                             m_messageSender{messageSender}, m_contextManager{contextManager}, m_focusManager{focusManager}, m_streamingState{StreamingState::INACTIVE},
                             m_focusTransitionState{FocusTransitionState::INTERNAL}, m_scanningTransitionState{ScanningTransitionState::INACTIVE},
                             m_focusState{FocusState::NONE}, m_sourceId{MediaPlayerInterface::ERROR}, m_deviceManager{move(deviceManager)}, m_mediaPlayer{mediaPlayer},
                             m_db{bluetoothStorage}, m_eventBus{eventBus}, m_mediaInputTransformer{mediaInputTransformer}, m_mediaStream{nullptr},
                             m_bluetoothChannelVolumeInterface{bluetoothChannelVolumeInterface} {
            m_capabilityConfigurations.insert(getBluetoothCapabilityConfiguration());
            for (const auto& connectionRule : enabledConnectionRules) {
                for (const auto& category : connectionRule->getDeviceCategories()) m_enabledConnectionRules[category] = connectionRule;
            }
        }
        DirectiveHandlerConfiguration Bluetooth::getConfiguration() const {
            ACSDK_DEBUG5(LX(__func__));
            DirectiveHandlerConfiguration configuration;
            auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
            auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
            configuration[SCAN_DEVICES] = neitherNonBlockingPolicy;
            configuration[ENTER_DISCOVERABLE_MODE] = neitherNonBlockingPolicy;
            configuration[EXIT_DISCOVERABLE_MODE] = neitherNonBlockingPolicy;
            configuration[PAIR_DEVICES] = neitherNonBlockingPolicy;
            configuration[UNPAIR_DEVICES] = neitherNonBlockingPolicy;
            configuration[SET_DEVICE_CATEGORIES] = neitherNonBlockingPolicy;
            configuration[CONNECT_BY_DEVICE_IDS] = neitherNonBlockingPolicy;
            configuration[CONNECT_BY_PROFILE] = neitherNonBlockingPolicy;
            configuration[DISCONNECT_DEVICES] = neitherNonBlockingPolicy;
            configuration[PLAY] = audioNonBlockingPolicy;
            configuration[STOP] = audioNonBlockingPolicy;
            configuration[NEXT] = audioNonBlockingPolicy;
            configuration[PREVIOUS] = audioNonBlockingPolicy;
            return configuration;
        }
        void Bluetooth::doShutdown() {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.shutdown();
            m_db->close();
            m_db.reset();
            m_mediaStream.reset();
            m_mediaAttachment.reset();
            m_mediaAttachmentWriter.reset();
            m_mediaPlayer->removeObserver(shared_from_this());
            if (MediaPlayerInterface::ERROR != m_sourceId) m_mediaPlayer->stop(m_sourceId);
            cleanupMediaSource();
            m_mediaPlayer.reset();
            m_messageSender.reset();
            m_focusManager.reset();
            m_contextManager->setStateProvider(BLUETOOTH_STATE, nullptr);
            m_contextManager.reset();
            auto hostController = m_deviceManager->getHostController();
            if (hostController) {
                auto stopScanFuture = hostController->stopScan();
                waitOnFuture(move(stopScanFuture), "Stop bluetooth scanning");
                auto exitDiscoverableFuture = hostController->exitDiscoverableMode();
                waitOnFuture(move(exitDiscoverableFuture), "Exit discoverable mode");
            }
            m_disabledA2DPDevice.reset();
            m_activeA2DPDevice.reset();
            m_restrictedDevices.clear();
            m_deviceManager.reset();
            m_eventBus.reset();
            m_observers.clear();
            m_bluetoothEventStates.clear();
            m_connectedDevices.clear();
        }
        unordered_set<shared_ptr<CapabilityConfiguration>> Bluetooth::getCapabilityConfigurations() {
            return m_capabilityConfigurations;
        }
        void Bluetooth::clearData() {
            m_executor.submit([this] {
                ACSDK_DEBUG5(LX("clearData"));
                auto hostController = m_deviceManager->getHostController();
                if (hostController->isScanning()) {
                    auto stopScanFuture = hostController->stopScan();
                    waitOnFuture(std::move(stopScanFuture), "Stop bluetooth scanning");
                    ACSDK_DEBUG5(LX("clearData").d("action", "stoppedScanning"));
                }
                if (hostController->isDiscoverable()) {
                    auto exitDiscoverableFuture = hostController->exitDiscoverableMode();
                    waitOnFuture(std::move(exitDiscoverableFuture), "Exit discoverable mode");
                    ACSDK_DEBUG5(LX("clearData").d("action", "disabledDiscoverable"));
                }
                for (auto& device : m_deviceManager->getDiscoveredDevices()) {
                    if (device->isPaired()) {
                        auto unpairFuture = device->unpair();
                        waitOnFuture(std::move(unpairFuture), "Unpair device");
                        ACSDK_DEBUG5(LX("clearData").d("action", "unpairDevice").d("device", truncateFriendlyName(device->getFriendlyName())));
                    }
                }
                m_db->clear();
            });
        }
        void Bluetooth::executeInitializeMediaSource() {
            ACSDK_DEBUG5(LX(__func__));
            this_thread::sleep_for(INITIALIZE_SOURCE_DELAY);
            auto a2dpSource = getService<A2DPSourceInterface>(m_activeA2DPDevice);
            if (!a2dpSource) {
                ACSDK_CRITICAL(LX(__func__).d("reason", "a2dpSourceNotSupported"));
                return;
            }
            auto stream = a2dpSource->getSourceStream();
            if (!stream) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullA2DPStream"));
                return;
            }
            setCurrentStream(stream);
        }
        void Bluetooth::cleanupMediaSource() {
            ACSDK_DEBUG5(LX(__func__));
            setCurrentStream(nullptr);
            m_sourceId = MediaPlayerInterface::ERROR;
        }
        bool Bluetooth::retrieveUuid(const string& mac, string* uuid) {
            ACSDK_DEBUG5(LX(__func__));
            if (!uuid) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullUuid"));
                return false;
            }
            if (m_db->getUuid(mac, uuid)) return true;
            ACSDK_INFO(LX(__func__).d("reason", "noMatchingUUID").d("mac", mac));
            *uuid = uuidGeneration::generateUUID();
            if (!m_db->insertByMac(mac, *uuid, false)) {
                string truncatedMac = truncateWithDefault(mac);
                ACSDK_ERROR(LX(__func__).d("reason", "insertingToDBFailed").d("mac", truncatedMac).d("uuid", *uuid));
                return false;
            }
            return true;
        }
        bool Bluetooth::retrieveDeviceCategoryByUuid(const string& uuid, DeviceCategory* category) {
            ACSDK_DEBUG5(LX(__func__));
            if (!category) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDeviceCategory"));
                return false;
            }
            string categoryString;
            if (m_db->getCategory(uuid, &categoryString)) {
                *category = stringToDeviceCategory(categoryString);
                return true;
            }
            return false;
        }
        shared_ptr<BluetoothDeviceConnectionRuleInterface> Bluetooth::retrieveConnectionRuleByUuid(const string& uuid) {
            ACSDK_DEBUG5(LX(__func__));
            DeviceCategory category = DeviceCategory::UNKNOWN;
            if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveDeviceCategoryFailed"));
                return nullptr;
            }
            auto ruleIt = m_enabledConnectionRules.find(category);
            if (ruleIt != m_enabledConnectionRules.end()) return ruleIt->second;
            return nullptr;
        }
        void Bluetooth::clearUnusedUuids() {
            ACSDK_DEBUG5(LX(__func__));
            list<string> descendingMacs;
            if (!m_db->getOrderedMac(false, &descendingMacs)) {
                ACSDK_ERROR(LX(__func__).d("reason", "databaseQueryFailed"));
                return;
            }
            for (const auto& mac : descendingMacs) {
                shared_ptr<BluetoothDeviceInterface> device = retrieveDeviceByMac(mac);
                if (!device) m_db->remove(mac);
            }
        }
        shared_ptr<BluetoothDeviceInterface> Bluetooth::retrieveDeviceByUuid(const string& uuid) {
            ACSDK_DEBUG5(LX(__func__).d("uuid", uuid));
            string mac;
            if (!m_db->getMac(uuid, &mac)) {
                ACSDK_ERROR(LX("retrieveDeviceByUuidFailed").d("reason", "macNotFound").d("uuid", uuid));
                return nullptr;
            }
            shared_ptr<BluetoothDeviceInterface> device = retrieveDeviceByMac(mac);
            if (!device) {
                string truncatedMac = truncateWithDefault(mac);
                ACSDK_ERROR(LX("retrieveDeviceByUuidFailed").d("reason", "couldNotFindDevice").d("mac", truncatedMac));
                return nullptr;
            }
            return device;
        }
        shared_ptr<BluetoothDeviceInterface> Bluetooth::retrieveDeviceByMac(const string& mac) {
            string truncatedMac = truncateWithDefault(mac);
            ACSDK_DEBUG5(LX(__func__).d("mac", truncatedMac));
            auto devices = m_deviceManager->getDiscoveredDevices();
            for (const auto& device : devices) {
                if (device->getMac() == mac) return device;
            }
            return nullptr;
        }
        void Bluetooth::executeUpdateContext() {
            Document payload(kObjectType);
            Value alexaDevice(kObjectType);
            Value friendly_name_key{FRIENDLY_NAME_KEY, strlen(FRIENDLY_NAME_KEY)};
            string friendlyName = m_deviceManager->getHostController()->getFriendlyName();
            Value _friendlyName{friendlyName.data(), friendlyName.length()};
            alexaDevice.AddMember(friendly_name_key, _friendlyName);
            payload.AddMember(ALEXA_DEVICE_NAME_KEY, alexaDevice, payload.GetAllocator());
            Value pairedDevices(kArrayType);
            unordered_map<string, string> macToUuids;
            unordered_map<string, string> uuidToCategory;
            if (!m_db->getMacToUuid(&macToUuids)) {
                ACSDK_ERROR(LX(__func__).d("reason", "databaseQueryFailed"));
                macToUuids.clear();
            }
            if (!m_db->getUuidToCategory(&uuidToCategory)) {
                ACSDK_ERROR(LX(__func__).d("reason", "databaseQueryFailed"));
                uuidToCategory.clear();
            }
            for (const auto& device : m_deviceManager->getDiscoveredDevices()) {
                ACSDK_DEBUG9(LX(__func__).d("friendlyName", truncateFriendlyName(device->getFriendlyName())).d("mac", device->getMac())
                                 .d("paired", device->isPaired()));
                if (!device->isPaired()) {
                    ACSDK_DEBUG9(LX(__func__).d("reason", "deviceNotPaired").m("Excluding"));
                    continue;
                }
                string uuid;
                const auto uuidIt = macToUuids.find(device->getMac());
                if (macToUuids.end() != uuidIt) uuid = uuidIt->second;
                else {
                    ACSDK_DEBUG5(LX(__func__).d("reason", "uuidNotFound").d("mac", device->getMac()));
                    uuid.clear();
                    if (!retrieveUuid(device->getMac(), &uuid)) {
                        ACSDK_ERROR(LX("executeUpdateContextFailed").d("reason", "retrieveUuidFailed"));
                        continue;
                    }
                }
                string category;
                const auto categoryIt = uuidToCategory.find(uuid);
                if (uuidToCategory.end() != categoryIt) category = categoryIt->second;
                else category = deviceCategoryToString(DeviceCategory::UNKNOWN);
                Value deviceJson(rapidjson::kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value _uuid{uuid.data(), uuid.length()};
                Value device_category_key{DEVICE_CATEGORY_KEY, strlen(DEVICE_CATEGORY_KEY)};
                Value _category{category.data(), category.length()};
                Value deviceNameFriendly{device->getFriendlyName().data(), device->getFriendlyName().length()};
                deviceJson.AddMember(unique_device_id_key, _uuid);
                deviceJson.AddMember(device_category_key, _category);
                deviceJson.AddMember(friendly_name_key, deviceNameFriendly);
                rapidjson::Value supportedProfiles(rapidjson::kArrayType);
                if (!extractAvsProfiles(device, payload.GetAllocator(), &supportedProfiles)) {
                    supportedProfiles = rapidjson::Value(rapidjson::kArrayType);
                }
                Value supported_profiles_key{SUPPORTED_PROFILES_KEY, strlen(SUPPORTED_PROFILES_KEY)};
                Value supportedProfilesArray{supportedProfiles.GetArray()};
                Value connection_state_key{CONNECTION_STATE_KEY, strlen(CONNECTION_STATE_KEY)};
                string connectedState = convertToAVSConnectedState((device->getDeviceState()));
                Value _connectedState{connectedState.data(), connectedState.length()};
                deviceJson.AddMember(supported_profiles_key, supportedProfilesArray);
                deviceJson.AddMember(connection_state_key, _connectedState);
                pairedDevices.PushBack(deviceJson);
            }
            payload.AddMember(PAIRED_DEVICES_KEY, pairedDevices, payload.GetAllocator());
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            ACSDK_DEBUG9(LX(__func__).sensitive("buffer", buffer.GetString()));
            m_contextManager->setState(BLUETOOTH_STATE, buffer.GetString(), StateRefreshPolicy::NEVER);
        }
        void Bluetooth::executeSendEvent(const string& eventName, const string& eventPayload) {
            ACSDK_DEBUG5(LX(__func__).d("eventName", eventName));
            auto event = buildJsonEventString(eventName, "", eventPayload, "");
            ACSDK_DEBUG5(LX("onExecuteSendEventLambda").d("event", event.second));
            auto request = std::make_shared<MessageRequest>(event.second);
            m_messageSender->sendMessage(request);
        }
        bool Bluetooth::extractAvsProfiles(shared_ptr<BluetoothDeviceInterface> device, Document::AllocatorType& allocator, Value* supportedProfiles) {
            if (!device || !supportedProfiles || !supportedProfiles->IsArray()) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidInputParameters"));
                return false;
            }
            for (const auto& sdp : device->getSupportedServices()) {
                auto profileNameIt = AVS_PROFILE_MAP.find(sdp->getUuid());
                if (profileNameIt != AVS_PROFILE_MAP.end()) {
                    Value profile(kObjectType);
                    Value name_key{NAME_KEY, strlen(NAME_KEY)};
                    Value profileNameItSecond{profileNameIt->second.data(), profileNameIt->second.length()};
                    Value version_key{VERSION_KEY, strlen(VERSION_KEY)};
                    Value version{sdp->getVersion().data(), sdp->getVersion().length()};
                    profile.AddMember(name_key, profileNameItSecond);
                    profile.AddMember(version_key, version);
                    if (A2DPSinkInterface::UUID == profileNameIt->first || A2DPSourceInterface::UUID == profileNameIt->first) {
                        Value profileState(kObjectType);
                        Value streaming_key{STREAMING_KEY, strlen(STREAMING_KEY)};
                        if (m_activeA2DPDevice && device == m_activeA2DPDevice) {
                            string avsStreamingState = convertToAVSStreamingState(m_streamingState);
                            Value _avsStreamingState{avsStreamingState.data(), avsStreamingState.length()};
                            profileState.AddMember(streaming_key, _avsStreamingState);
                        } else {
                            string avsStreamingState = convertToAVSStreamingState(Bluetooth::StreamingState::INACTIVE);
                            Value _avsStreamingState{avsStreamingState.data(), avsStreamingState.length()};
                            profileState.AddMember(streaming_key, _avsStreamingState);
                        }
                        Value state_key{STATE_KEY, strlen(STATE_KEY)};
                        profile.AddMember(state_key, profileState);
                    }
                    supportedProfiles->PushBack(profile);
                }
            }
            return true;
        }
        void Bluetooth::executeQueueEventAndRequestContext(const string& eventName, const string& eventPayload) {
            ACSDK_DEBUG5(LX(__func__).d("eventName", eventName));
            m_eventQueue.push(make_pair(eventName, eventPayload));
            m_contextManager->getContext(shared_from_this());
        }
        void Bluetooth::executeAbortMediaPlayback() {
            if (MediaPlayerInterface::ERROR != m_sourceId && !m_mediaPlayer->stop(m_sourceId)) cleanupMediaSource();
            if (FocusState::FOREGROUND == m_focusState || FocusState::BACKGROUND == m_focusState) {
                ACSDK_DEBUG5(LX(__func__).d("reason", "releasingFocus").d("focusState", m_focusState));
                executeReleaseFocus(__func__);
            }
        }
        void Bluetooth::executeEnterForeground() {
            ACSDK_DEBUG5(LX(__func__).d("streamingState", streamingStateToString(m_streamingState)));
            if (!m_bluetoothChannelVolumeInterface->stopDucking()) { ACSDK_WARN(LX(__func__).m("Failed To Restore Audio Channel Volume")); }
            if (!m_activeA2DPDevice) {
                ACSDK_ERROR(LX(__func__).d("reason", "noActiveDevice"));
                executeAbortMediaPlayback();
                return;
            }
            auto avrcpTarget = getService<AVRCPTargetInterface>(m_activeA2DPDevice);
            if (!avrcpTarget) { ACSDK_INFO(LX(__func__).d("reason", "avrcpTargetNotSupported")); }
            switch(m_streamingState) {
                case StreamingState::ACTIVE: break;
                case StreamingState::PENDING_ACTIVE:
                    if (m_mediaStream == nullptr) executeInitializeMediaSource();
                    if (!m_mediaPlayer->play(m_sourceId)) { ACSDK_ERROR(LX(__func__).d("reason", "playFailed").d("sourceId", m_sourceId)); }
                    break;
                case StreamingState::PENDING_PAUSED: case StreamingState::PAUSED: case StreamingState::INACTIVE:
                    if (m_focusState == FocusState::BACKGROUND) {
                        if (avrcpTarget && !avrcpTarget->play()) { ACSDK_ERROR(LX(__func__).d("reason", "avrcpPlayFailed")); }
                        m_streamingState = StreamingState::PENDING_ACTIVE;
                    }
                    if (m_mediaStream == nullptr) executeInitializeMediaSource();
                    if (!m_mediaPlayer->play(m_sourceId)) { ACSDK_ERROR(LX(__func__).d("reason", "playFailed").d("sourceId", m_sourceId)); }
                    break;
            }
        }
        void Bluetooth::executeEnterBackground(MixingBehavior behavior) {
            ACSDK_DEBUG5(LX(__func__).d("streamingState", streamingStateToString(m_streamingState)));
            if (!m_activeA2DPDevice) {
                ACSDK_ERROR(LX(__func__).d("reason", "noActiveDevice"));
                executeAbortMediaPlayback();
                return;
            }
            if (MixingBehavior::MAY_DUCK == behavior) {
                if (!m_bluetoothChannelVolumeInterface->startDucking()) { ACSDK_WARN(LX(__func__).m("Failed to Attenuate Audio Channel Volume")); }
                else {
                    ACSDK_DEBUG4(LX(__func__).d("action", "ducking audio"));
                    return;
                }
            }
            auto avrcpTarget = getService<AVRCPTargetInterface>(m_activeA2DPDevice);
            if (!avrcpTarget) { ACSDK_INFO(LX(__func__).d("reason", "avrcpTargetNotSupported")); }
            switch(m_streamingState) {
                case StreamingState::ACTIVE:
                    if (avrcpTarget && !avrcpTarget->pause()) { ACSDK_ERROR(LX(__func__).d("reason", "avrcpPauseFailed")); }
                    m_streamingState = StreamingState::PENDING_PAUSED;
                    if (!m_mediaPlayer->stop(m_sourceId)) { ACSDK_ERROR(LX(__func__).d("reason", "stopFailed").d("sourceId", m_sourceId)); }
                    break;
                case StreamingState::PENDING_ACTIVE:
                    if (avrcpTarget && !avrcpTarget->pause()) { ACSDK_ERROR(LX(__func__).d("reason", "avrcpPauseFailed")); }
                    m_streamingState = StreamingState::PENDING_PAUSED;
                    break;
                case StreamingState::PAUSED: case StreamingState::PENDING_PAUSED: case StreamingState::INACTIVE: break;
            }
        }
        void Bluetooth::executeEnterNone() {
            ACSDK_DEBUG5(LX(__func__).d("streamingState", streamingStateToString(m_streamingState)));
            if (!m_bluetoothChannelVolumeInterface->stopDucking()) { ACSDK_WARN(LX(__func__).m("Failed To Restore Audio Channel Volume")); }
            if (!m_activeA2DPDevice) {
                ACSDK_DEBUG5(LX(__func__).d("reason", "noActiveDevice"));
                executeAbortMediaPlayback();
                return;
            }
            if (FocusTransitionState::EXTERNAL == m_focusTransitionState) {
                auto a2dpSource = getService<A2DPSourceInterface>(m_activeA2DPDevice);
                if (a2dpSource && m_focusState != FocusState::NONE) {
                    if (executeFunctionOnDevice(m_activeA2DPDevice, &BluetoothDeviceInterface::disconnect)) {
                        executeOnDeviceDisconnect(m_activeA2DPDevice, Requester::DEVICE);
                    } else {
                        unordered_set<std::string> uuids;
                        string uuid;
                        if (retrieveUuid(m_activeA2DPDevice->getMac(), &uuid)) uuids.insert(uuid);
                        else { ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed")); }
                        executeSendDisconnectDevicesFailed(uuids, Requester::DEVICE);
                    }
                }
            } else {
                auto avrcpTarget = getService<AVRCPTargetInterface>(m_activeA2DPDevice);
                if (!avrcpTarget) { ACSDK_INFO(LX(__func__).d("reason", "avrcpTargetNotSupported")); }
                switch(m_streamingState) {
                    case StreamingState::ACTIVE: case StreamingState::PENDING_ACTIVE:
                        if (avrcpTarget && !avrcpTarget->pause()) { ACSDK_ERROR(LX(__func__).d("reason", "avrcpPauseFailed")); }
                        m_streamingState = StreamingState::PENDING_PAUSED;
                        if (!m_mediaPlayer->stop(m_sourceId)) {
                            ACSDK_ERROR(LX(__func__).d("reason", "stopFailed").d("sourceId", m_sourceId));
                            cleanupMediaSource();
                        }
                        break;
                    case StreamingState::PENDING_PAUSED:
                        if (!m_mediaPlayer->stop(m_sourceId)) {
                            ACSDK_ERROR(LX(__func__).d("reason", "stopFailed").d("sourceId", m_sourceId));
                            cleanupMediaSource();
                        }
                        break;
                    case StreamingState::PAUSED: case StreamingState::INACTIVE: break;
                }
            }
        }
        void Bluetooth::onFocusChanged(FocusState newFocus, MixingBehavior behavior) {
            ACSDK_DEBUG5(LX(__func__).d("current", m_focusState).d("new", newFocus).d("MixingBehavior", behavior));
            m_executor.submit([this, newFocus, behavior] {
                switch(newFocus) {
                    case FocusState::FOREGROUND: executeEnterForeground(); break;
                    case FocusState::BACKGROUND: executeEnterBackground(behavior); break;
                    case FocusState::NONE: executeEnterNone(); break;
                }
                if (FocusState::NONE == newFocus && FocusTransitionState::PENDING_INTERNAL == m_focusTransitionState) {
                    m_focusTransitionState = FocusTransitionState::INTERNAL;
                } else if (FocusState::NONE != newFocus && FocusTransitionState::PENDING_INTERNAL != m_focusTransitionState) {
                    m_focusTransitionState = FocusTransitionState::EXTERNAL;
                }
                m_focusState = newFocus;
            });
        }
        void Bluetooth::onContextAvailable(const string& jsonContext) {
            m_executor.submit([this, jsonContext] {
                ACSDK_DEBUG9(LX("onContextAvailableLambda"));
                if (m_eventQueue.empty()) {
                    ACSDK_ERROR(LX("contextRequestedWithNoQueuedEvents"));
                    return;
                }
                pair<string, string> nameAndPayload = m_eventQueue.front();
                m_eventQueue.pop();
                auto event = buildJsonEventString(nameAndPayload.first, "", nameAndPayload.second, jsonContext);
                ACSDK_DEBUG5(LX("onContextAvailableLambda").d("event", event.second));
                auto request = make_shared<MessageRequest>(event.second);
                m_messageSender->sendMessage(request);
            });
        }
        void Bluetooth::onContextFailure(const ContextRequestError error) {
            pair<string, string> nameAndPayload = m_eventQueue.front();
            m_eventQueue.pop();
            ACSDK_ERROR(LX(__func__).d("error", error).d("eventName", nameAndPayload.first).sensitive("payload", nameAndPayload.second));
        }
        void Bluetooth::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
            ACSDK_DEBUG5(LX(__func__));
            handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
        }
        void Bluetooth::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
        static bool parseDirectivePayload(const string& payload, Document* document) {
            ACSDK_DEBUG5(LX(__func__));
            if (!document) {
                ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", "nullDocument"));
                return false;
            }
            ParseResult result = document->Parse(payload.data());
            if (!result) {
                ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", "parseFailed").d("error", GetParseError_En(result.Code())).d("offset", result.Offset()));
                return false;
            }
            return true;
        }
        unordered_set<string> Bluetooth::retrieveUuidsFromConnectionPayload(const Document& payload) {
            unordered_set<string> uuids;
            Value::ConstMemberIterator it;
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            if (jsonUtils::findNode(_payload, DEVICES_KEY, &it) && it->value.IsArray()) {
                for (auto deviceIt = it->value.Begin(); deviceIt != it->value.End(); deviceIt++) {
                    std::string uuid;
                    if (!json::jsonUtils::retrieveValue(*deviceIt, UNIQUE_DEVICE_ID_KEY, &uuid)) {
                        ACSDK_ERROR(LX("retrieveValueFailed").d("reason", "uuidNotFound"));
                        continue;
                    }
                    uuids.insert(uuid);
                }
            }
            return uuids;
        }
        void Bluetooth::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
            ACSDK_DEBUG5(LX(__func__));
            if (!info) {
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                return;
            }
            m_executor.submit([this, info] {
                const string directiveName = info->directive->getName();
                Document payload(kObjectType);
                if (!parseDirectivePayload(info->directive->getPayload(), &payload)) {
                    sendExceptionEncountered(info, "Payload Parsing Failed", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return;
                }
                if (directiveName == SCAN_DEVICES.name) {
                    clearUnusedUuids();
                    executeSetScanMode(true);
                } else if (directiveName == ENTER_DISCOVERABLE_MODE.name) {
                    if (executeSetDiscoverableMode(true)) executeSendEnterDiscoverableModeSucceeded();
                    else executeSendEnterDiscoverableModeFailed();
                } else if (directiveName == EXIT_DISCOVERABLE_MODE.name) {
                    executeSetScanMode(false);
                    executeSetDiscoverableMode(false);
                } else if (directiveName == PAIR_DEVICES.name) {
                    unordered_set<string> uuids = retrieveUuidsFromConnectionPayload(payload);
                    if (!uuids.empty()) {
                        executeSetScanMode(false, false);
                        executeSetDiscoverableMode(false);
                        if (!executePairDevices(uuids)) executeSetScanMode(true, false);
                    } else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == UNPAIR_DEVICES.name) {
                    unordered_set<string> uuids = retrieveUuidsFromConnectionPayload(payload);
                    if (!uuids.empty()) executeUnpairDevices(uuids);
                    else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == SET_DEVICE_CATEGORIES.name) {
                    Value::ConstMemberIterator it;
                    Value _payload{payload.GetString(), strlen(payload.GetString())};
                    if (jsonUtils::findNode(_payload, DEVICES_KEY, &it) && it->value.IsArray()) {
                        map<string, string> uuidCategoryMap;
                        string uuid;
                        string category;
                        for (auto deviceIt = it->value.Begin(); deviceIt != it->value.End(); deviceIt++) {
                            if (!jsonUtils::retrieveValue(*deviceIt, UNIQUE_DEVICE_ID_KEY, &uuid)) {
                                ACSDK_ERROR(LX("parsingSetDeviceCategoriesFailed").d("reason", "uuidNotFound"));
                                continue;
                            }
                            if (!jsonUtils::retrieveValue(*deviceIt, DEVICE_CATEGORY_KEY, &category)) {
                                ACSDK_ERROR(LX("parsingSetDeviceCategoriesFailed").d("reason", "categoryNotFound"));
                                continue;
                            }
                            uuidCategoryMap.insert({uuid, category});
                        }
                        executeSetDeviceCategories(uuidCategoryMap);
                    } else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == CONNECT_BY_DEVICE_IDS.name) {
                    unordered_set<string> uuids = retrieveUuidsFromConnectionPayload(payload);
                    if (!uuids.empty()) executeConnectByDeviceIds(uuids);
                    else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == CONNECT_BY_PROFILE.name) {
                    Value::ConstMemberIterator it;
                    string profileName;
                    string version;
                    Value _payload{payload.GetString(), strlen(payload.GetString())};
                    if (jsonUtils::findNode(_payload, PROFILE_KEY, &it) && jsonUtils::retrieveValue(it->value, NAME_KEY, &profileName)) {
                        if (!jsonUtils::retrieveValue(it->value, VERSION_KEY, &version)) {
                            ACSDK_INFO(LX("parsingConnectByProfileDirective").d("reason", "versionMissing")
                                           .d("action", "defaultToEmptyAndProceed"));
                            version.clear();
                        }
                        executeConnectByProfile(profileName, version);
                    } else {
                        sendExceptionEncountered(info, "profileName not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == DISCONNECT_DEVICES.name) {
                    unordered_set<string> uuids = retrieveUuidsFromConnectionPayload(payload);
                    if (!uuids.empty()) executeDisconnectDevices(uuids);
                    else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == PLAY.name) {
                    Value::ConstMemberIterator it;
                    string uuid;
                    Value _payload{payload.GetString(), strlen(payload.GetString())};
                    if (jsonUtils::findNode(_payload, DEVICE_KEY, &it) && jsonUtils::retrieveValue(it->value, UNIQUE_DEVICE_ID_KEY, &uuid)) {
                        auto device = retrieveDeviceByUuid(uuid);
                        if (!device) {
                            ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                            executeSendMediaControlPlayFailed(device);
                        }
                        executePlay(device);
                    } else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == STOP.name) {
                    Value::ConstMemberIterator it;
                    string uuid;
                    Value _payload{payload.GetString(), strlen(payload.GetString())};
                    if (jsonUtils::findNode(_payload, DEVICE_KEY, &it) && jsonUtils::retrieveValue(it->value, UNIQUE_DEVICE_ID_KEY, &uuid)) {
                        auto device = retrieveDeviceByUuid(uuid);
                        if (!device) {
                            ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                            executeSendMediaControlStopFailed(device);
                        }
                        executeStop(device);
                    } else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == NEXT.name) {
                    Value::ConstMemberIterator it;
                    string uuid;
                    Value _payload{payload.GetString(), strlen(payload.GetString())};
                    if (jsonUtils::findNode(_payload, DEVICE_KEY, &it) && jsonUtils::retrieveValue(it->value, UNIQUE_DEVICE_ID_KEY, &uuid)) {
                        auto device = retrieveDeviceByUuid(uuid);
                        if (!device) {
                            ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                            executeSendMediaControlNextFailed(device);
                        }
                        executeNext(device);
                    } else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else if (directiveName == PREVIOUS.name) {
                    Value::ConstMemberIterator it;
                    string uuid;
                    Value _payload{payload.GetString(), strlen(payload.GetString())};
                    if (jsonUtils::findNode(_payload, DEVICE_KEY, &it) && jsonUtils::retrieveValue(it->value, UNIQUE_DEVICE_ID_KEY, &uuid)) {
                        auto device = retrieveDeviceByUuid(uuid);
                        if (!device) {
                            ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                            executeSendMediaControlPreviousFailed(device);
                        }
                        executePrevious(device);
                    } else {
                        sendExceptionEncountered(info, "uniqueDeviceId not found.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                } else {
                    sendExceptionEncountered(info, "Unexpected Directive", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return;
                }
                executeSetHandlingCompleted(info);
            });
        }
        void Bluetooth::executePlay(shared_ptr<BluetoothDeviceInterface> device) {
            ACSDK_DEBUG5(LX(__func__));
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return;
            }
            auto avrcpTarget = getService<AVRCPTargetInterface>(device);
            if (!avrcpTarget) {
                ACSDK_ERROR(LX(__func__).d("reason", "notSupported"));
                executeSendMediaControlPlayFailed(device);
                return;
            }
            if (device == m_activeA2DPDevice) {
                if (StreamingState::ACTIVE == m_streamingState || StreamingState::PENDING_ACTIVE == m_streamingState) {
                    executeSendMediaControlPlaySucceeded(device);
                    return;
                }
                bool success = true;
                if (StreamingState::PAUSED == m_streamingState || StreamingState::INACTIVE == m_streamingState) success = avrcpTarget->play();
                if (success) {
                    m_streamingState = StreamingState::PENDING_ACTIVE;
                    executeSendMediaControlPlaySucceeded(device);
                    executeAcquireFocus(__func__);
                } else executeSendMediaControlPlayFailed(device);
            } else {
                if (avrcpTarget->play()) executeSendMediaControlPlaySucceeded(device);
                else executeSendMediaControlPlayFailed(device);
            }
        }
        void Bluetooth::executeStop(shared_ptr<BluetoothDeviceInterface> device) {
            ACSDK_DEBUG5(LX(__func__));
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return;
            }
            auto avrcpTarget = getService<AVRCPTargetInterface>(device);
            if (!avrcpTarget) {
                ACSDK_ERROR(LX(__func__).d("reason", "notSupported"));
                executeSendMediaControlStopFailed(device);
                return;
            }
            if (device == m_activeA2DPDevice) {
                if (StreamingState::PAUSED == m_streamingState || StreamingState::PENDING_PAUSED == m_streamingState) {
                    executeSendMediaControlStopSucceeded(device);
                    executeReleaseFocus(__func__);
                    return;
                }
                bool success = true;
                if (StreamingState::ACTIVE == m_streamingState) success = avrcpTarget->pause();
                if (success) {
                    m_streamingState = StreamingState::PENDING_PAUSED;
                    executeSendMediaControlStopSucceeded(device);
                } else executeSendMediaControlStopFailed(device);
                executeReleaseFocus(__func__);
            } else {
                if (avrcpTarget->pause()) executeSendMediaControlStopSucceeded(device);
                else executeSendMediaControlStopFailed(device);
            }
        }
        void Bluetooth::executeNext(shared_ptr<BluetoothDeviceInterface> device) {
            ACSDK_DEBUG5(LX(__func__));
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return;
            }
            auto avrcpTarget = getService<AVRCPTargetInterface>(device);
            if (!avrcpTarget) {
                ACSDK_ERROR(LX(__func__).d("reason", "notSupported"));
                executeSendMediaControlNextFailed(device);
                return;
            }
            if (avrcpTarget->next()) executeSendMediaControlNextSucceeded(device);
            else executeSendMediaControlNextFailed(device);
        }
        void Bluetooth::executePrevious(shared_ptr<BluetoothDeviceInterface> device) {
            ACSDK_DEBUG5(LX(__func__));
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return;
            }
            auto avrcpTarget = getService<AVRCPTargetInterface>(device);
            if (!avrcpTarget) {
                ACSDK_ERROR(LX(__func__).d("reason", "notSupported"));
                executeSendMediaControlPreviousFailed(device);
                return;
            }
            if (avrcpTarget->previous()) executeSendMediaControlPreviousSucceeded(device);
            else executeSendMediaControlPreviousFailed(device);
        }
        bool Bluetooth::executePairDevices(const unordered_set<string>& uuids) {
            ACSDK_DEBUG5(LX(__func__));
            bool pairingSuccess = true;
            for (const auto& uuid : uuids) {
                auto device = retrieveDeviceByUuid(uuid);
                if (!device) {
                    ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                    executeSendPairDevicesFailed({uuid});
                    pairingSuccess = false;
                    continue;
                }
                DeviceCategory category = DeviceCategory::UNKNOWN;
                if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "retrieveDeviceCategoryFailed"));
                    pairingSuccess = false;
                    continue;
                }
                if (executeFunctionOnDevice(device, &BluetoothDeviceInterface::pair)) {
                    executeInsertBluetoothEventState(device, DeviceState::PAIRED, Optional<Requester>(), Optional<std::string>());
                    auto connectionRule = retrieveConnectionRuleByUuid(uuid);
                    if (connectionRule && connectionRule->shouldExplicitlyConnect()) executeConnectByDeviceIds({uuid});
                } else {
                    executeSendPairDevicesFailed({uuid});
                    pairingSuccess = false;
                }
            }
            return pairingSuccess;
        }
        bool Bluetooth::executeUnpairDevices(const unordered_set<string>& uuids) {
            ACSDK_DEBUG5(LX(__func__));
            bool unpairingSuccess = true;
            for (const auto& uuid : uuids) {
                auto device = retrieveDeviceByUuid(uuid);
                if (!device) {
                    ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                    executeSendUnpairDevicesFailed({uuid});
                    unpairingSuccess = false;
                    continue;
                }
                DeviceCategory category = DeviceCategory::UNKNOWN;
                if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "retrieveDeviceCategoryFailed"));
                    unpairingSuccess = false;
                    continue;
                }
                auto connectionRule = retrieveConnectionRuleByUuid(uuid);
                if (device->isConnected()) {
                    if (connectionRule && connectionRule->shouldExplicitlyDisconnect()) executeDisconnectDevices({uuid});
                }
                if (executeFunctionOnDevice(device, &BluetoothDeviceInterface::unpair)) {
                    executeInsertBluetoothEventState(device, DeviceState::UNPAIRED, Optional<Requester>(), Optional<std::string>());
                } else {
                    executeSendUnpairDevicesFailed({uuid});
                    unpairingSuccess = false;
                }
            }
            return unpairingSuccess;
        }
        map<string, string> Bluetooth::executeSetDeviceCategories(const map<string, string>& uuidCategoryMap) {
            ACSDK_DEBUG5(LX(__func__));
            map<string, string> uuidCategorySucceededMap;
            map<string, string> uuidCategoryFailedMap;
            for (const auto& uuidCategory : uuidCategoryMap) {
                if (m_db->updateByCategory(uuidCategory.first, uuidCategory.second)) uuidCategorySucceededMap.insert({uuidCategory.first, uuidCategory.second});
                else uuidCategoryFailedMap.insert({uuidCategory.first, uuidCategory.second});
            }
            if (!uuidCategorySucceededMap.empty()) executeSetDeviceCategoriesSucceeded(uuidCategorySucceededMap);
            if (!uuidCategoryFailedMap.empty()) executeSetDeviceCategoriesFailed(uuidCategoryFailedMap);
            return uuidCategoryFailedMap;
        }
        void Bluetooth::executeConnectByDeviceIds(const unordered_set<string>& uuids) {
            ACSDK_DEBUG5(LX(__func__));
            for (const auto& uuid : uuids) {
                auto device = retrieveDeviceByUuid(uuid);
                unordered_set<shared_ptr<BluetoothDeviceInterface>> devices;
                if (!device) {
                    ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                    executeSendConnectByDeviceIdsFailed({uuid}, Requester::CLOUD);
                    continue;
                }
                devices.insert(device);
                if (device->isConnected()) {
                    executeSendConnectByDeviceIdsSucceeded(devices, Requester::CLOUD);
                    continue;
                }
                if (executeFunctionOnDevice(device, &BluetoothDeviceInterface::connect)) {
                    executeOnDeviceConnect(device, true);
                    executeInsertBluetoothEventState(device, DeviceState::CONNECTED, Optional<Requester>(Requester::CLOUD), Optional<std::string>());
                } else executeSendConnectByDeviceIdsFailed({uuid}, Requester::CLOUD);
            }
        }
        void Bluetooth::executeConnectByProfile(const string& profileName, const string& profileVersion) {
            ACSDK_DEBUG5(LX(__func__).d("profileName", profileName).d("profileVersion", profileVersion));
            list<string> descendingMacs;
            if (!m_db->getOrderedMac(false, &descendingMacs)) {
                ACSDK_ERROR(LX(__func__).d("reason", "databaseQueryFailed"));
                executeSendConnectByProfileFailed(profileName, Requester::CLOUD);
                return;
            }
            list<shared_ptr<BluetoothDeviceInterface>> matchedDevices;
            for (const auto& mac : descendingMacs) {
                string truncatedMac = truncateWithDefault(mac);
                auto device = retrieveDeviceByMac(mac);
                if (!device) {
                    ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("mac", truncatedMac));
                    continue;
                }
                if (!device->isPaired()) {
                    ACSDK_DEBUG0(LX(__func__).d("reason", "deviceUnpaired").d("mac", truncatedMac));
                    continue;
                }
                if (supportsAvsProfile(device, profileName)) matchedDevices.push_back(device);
                if (matchedDevices.size() == MAX_CONNECT_BY_PROFILE_COUNT) break;
            }
            bool deviceConnected = false;
            for (auto& matchedDevice : matchedDevices) {
                if (executeFunctionOnDevice(matchedDevice, &BluetoothDeviceInterface::connect)) {
                    deviceConnected = true;
                    executeOnDeviceConnect(matchedDevice);
                    executeInsertBluetoothEventState(matchedDevice,DeviceState::CONNECTED, Optional<Requester>(Requester::CLOUD),
                        Optional<std::string>(profileName));
                    break;
                }
            }
            if (!deviceConnected) executeSendConnectByProfileFailed(profileName, Requester::CLOUD);
        }
        void Bluetooth::executeOnDeviceConnect(shared_ptr<BluetoothDeviceInterface> device, bool shouldNotifyConnection) {
            ACSDK_DEBUG5(LX(__func__));
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return;
            }
            string uuid;
            if (!retrieveUuid(device->getMac(), &uuid)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveDeviceUuidFailed"));
                return;
            }
            DeviceCategory category = DeviceCategory::UNKNOWN;
            if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveDeviceCategoryFailed"));
                return;
            }
            ACSDK_DEBUG5(LX(__func__).d("uuid", uuid).d("deviceCategory", category));
            auto connectionRule = retrieveConnectionRuleByUuid(uuid);
            if (connectionRule) {
                set<shared_ptr<BluetoothDeviceInterface>> devicesToDisconnect = connectionRule->devicesToDisconnect(m_connectedDevices);
                for (const auto& deviceToDisconnect : devicesToDisconnect) {
                    if (deviceToDisconnect == device) continue;
                    auto disconnectionFuture = deviceToDisconnect->disconnect();
                    if (waitOnFuture(std::move(disconnectionFuture), "Disconnect the connected device")) {
                        executeOnDeviceDisconnect(deviceToDisconnect, Requester::DEVICE);
                    } else {
                        string truncatedMac = truncateWithDefault(deviceToDisconnect->getMac());
                        ACSDK_ERROR(LX(__func__).d("reason", "disconnectExistingActiveDeviceFailed").d("mac", truncatedMac));
                    }
                }
            }
            if (getService<A2DPSinkInterface>(device) || getService<A2DPSourceInterface>(device)) m_activeA2DPDevice = device;
            if (m_connectedDevices.find(category) != m_connectedDevices.end()) {
                auto& connectedDevices = m_connectedDevices[category];
                connectedDevices.insert(device);
            } else m_connectedDevices[category] = {device};
            if (shouldNotifyConnection) {
                for (const auto& observer : m_observers) {
                    observer->onActiveDeviceConnected(generateDeviceAttributes(device));
                }
            }
            m_db->insertByMac(device->getMac(), uuid, true);
        }
        void Bluetooth::executeOnDeviceDisconnect(shared_ptr<BluetoothDeviceInterface> device, Requester requester) {
            ACSDK_DEBUG5(LX(__func__));
            string uuid;
            if (!retrieveUuid(device->getMac(), &uuid)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveDeviceUuidFailed"));
                return;
            }
            DeviceCategory category = DeviceCategory::UNKNOWN;
            if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveDeviceCategoryFailed"));
                return;
            }
            ACSDK_DEBUG5(LX(__func__).d("uuid", uuid).d("deviceCategory", category));
            if (device == m_activeA2DPDevice) {
                if (StreamingState::INACTIVE != m_streamingState) {
                    ACSDK_DEBUG5(LX(__func__).d("currentState", streamingStateToString(m_streamingState)).d("newState", streamingStateToString(StreamingState::INACTIVE)));
                    if (StreamingState::ACTIVE == m_streamingState) {
                        executeSendStreamingEnded(m_activeA2DPDevice);
                        executeReleaseFocus(__func__);
                    }
                    m_streamingState = StreamingState::INACTIVE;
                }
                m_activeA2DPDevice.reset();
            }
            if (m_connectedDevices.find(category) != m_connectedDevices.end()) {
                auto& connectedDevices = m_connectedDevices[category];
                connectedDevices.erase(device);
                if (connectedDevices.empty()) m_connectedDevices.erase(category);
            }
            for (const auto& observer : m_observers) observer->onActiveDeviceDisconnected(generateDeviceAttributes(device));
            executeInsertBluetoothEventState(device, DeviceState::DISCONNECTED, Optional<Requester>(requester), Optional<std::string>());
        }
        void Bluetooth::executeDisconnectDevices(const std::unordered_set<std::string>& uuids) {
            ACSDK_DEBUG5(LX(__func__));
            for (const auto& uuid : uuids) {
                auto device = retrieveDeviceByUuid(uuid);
                if (!device) {
                    ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuid));
                    executeSendDisconnectDevicesFailed({uuid}, Requester::CLOUD);
                    continue;
                }
                if (executeFunctionOnDevice(device, &BluetoothDeviceInterface::disconnect)) executeOnDeviceDisconnect(device, Requester::CLOUD);
                else executeSendDisconnectDevicesFailed({uuid}, Requester::CLOUD);
            }
        }
        void Bluetooth::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
            removeDirective(info);
        }
        void Bluetooth::executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info) {
            if (info && info->result) info->result->setCompleted();
            removeDirective(info);
        }
        void Bluetooth::removeDirective(shared_ptr<DirectiveInfo> info) {
            if (info->directive && info->result) CapabilityAgent::removeDirective(info->directive->getMessageId());
        }
        void Bluetooth::sendExceptionEncountered(shared_ptr<CapabilityAgent::DirectiveInfo> info, const string& message, ExceptionErrorType type) {
            m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);
            if (info && info->result) info->result->setFailed(message);
            removeDirective(info);
        }
        bool Bluetooth::executeSetScanMode(bool scanning, bool shouldReport) {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_deviceManager->getHostController()) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullHostController"));
                return false;
            }
            auto future = scanning ? m_deviceManager->getHostController()->startScan() : m_deviceManager->getHostController()->stopScan();
            if (!future.valid()) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidFuture"));
                return false;
            }
            bool success = waitOnFuture(std::move(future));
            if (success) {
                if (shouldReport) {
                    executeSendScanDevicesReport(m_deviceManager->getDiscoveredDevices(), scanning);
                }
                m_scanningTransitionState =
                    scanning ? ScanningTransitionState::ACTIVE : ScanningTransitionState::PENDING_INACTIVE;
            } else {
                ACSDK_ERROR(LX("executeSetScanModeFailed").d("scanning", scanning));
                if (scanning) executeSendScanDevicesFailed();
            }
            return success;
        }
        bool Bluetooth::executeSetDiscoverableMode(bool discoverable) {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_deviceManager->getHostController()) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullHostController"));
                return false;
            }
            std::future<bool> future;
            if (discoverable) future = m_deviceManager->getHostController()->enterDiscoverableMode();
            else future = m_deviceManager->getHostController()->exitDiscoverableMode();
            if (!future.valid()) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidFuture"));
                return false;
            }
            return waitOnFuture(std::move(future));
        }
        bool Bluetooth::executeFunctionOnDevice(shared_ptr<BluetoothDeviceInterface>& device, function<future<bool>(shared_ptr<BluetoothDeviceInterface>&)> function) {
            if (!device) {
                ACSDK_ERROR(LX("executeFunctionOnDeviceFailed").d("reason", "nullDevice"));
                return false;
            }
            string truncatedMac = truncateWithDefault(device->getMac());
            ACSDK_DEBUG5(LX(__func__).d("mac", truncatedMac));
            stringstream description;
            description << "executeFunctionOnDevice mac=" << device->getMac();
            auto future = function(device);
            return waitOnFuture(move(future), description.str());
        }
        void Bluetooth::onFirstByteRead(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", id));
        }
        void Bluetooth::onPlaybackStarted(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", id));
            m_executor.submit([this] {
                if (m_streamingState == StreamingState::PENDING_PAUSED) {
                    executeSendStreamingStarted(m_activeA2DPDevice);
                    if (!m_mediaPlayer->stop(m_sourceId)) {
                        ACSDK_ERROR(LX("onPlaybackStartedLambdaFailed").d("reason", "stopFailed"));
                        cleanupMediaSource();
                    }
                    return;
                }
                m_streamingState = StreamingState::ACTIVE;
                if (!m_activeA2DPDevice) { ACSDK_ERROR(LX(__func__).d("reason", "noActiveDevice")); }
                else executeSendStreamingStarted(m_activeA2DPDevice);
            });
        }
        void Bluetooth::onPlaybackStopped(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", id));
            m_executor.submit([this] {
                cleanupMediaSource();
                if (m_activeA2DPDevice) {
                    m_streamingState = StreamingState::PAUSED;
                    executeSendStreamingEnded(m_activeA2DPDevice);
                } else m_streamingState = StreamingState::INACTIVE;
            });
        }
        void Bluetooth::onPlaybackFinished(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", id));
            m_executor.submit([this] {
                m_streamingState = StreamingState::INACTIVE;
                cleanupMediaSource();
                if (m_activeA2DPDevice) executeSendStreamingEnded(m_activeA2DPDevice);
            });
        }
        void Bluetooth::onPlaybackError(MediaPlayerObserverInterface::SourceId id, const ErrorType& type, string error, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("id", id).d("type", type).d("error", error));
            m_executor.submit([this, id] { if (id == m_sourceId) cleanupMediaSource(); });
        }
        void Bluetooth::executeSendScanDevicesReport(const list<shared_ptr<BluetoothDeviceInterface>>& devices, bool hasMore) {
            Document payload(kObjectType);
            Value devicesArray(kArrayType);
            ACSDK_DEBUG5(LX(__func__).d("count", devices.size()));
            for (const auto& device : devices) {
                string truncatedMac = truncateWithDefault(device->getMac());
                ACSDK_DEBUG(LX("foundDevice").d("deviceMac", truncatedMac));
                string uuid;
                if (!retrieveUuid(device->getMac(), &uuid)) {
                    ACSDK_ERROR(LX("executeSendScanDevicesReportFailed").d("reason", "retrieveUuidFailed"));
                    return;
                }
                if (device->isPaired()) {
                    ACSDK_DEBUG(LX(__func__).d("reason", "deviceAlreadyPaired").d("mac", truncatedMac).d("uuid", uuid).d("action", "ommitting"));
                    continue;
                }
                Value deviceJson(rapidjson::kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value _uuid{uuid.data(), uuid.length()};
                Value friendly_name_key{FRIENDLY_NAME_KEY, strlen(FRIENDLY_NAME_KEY)};
                Value deviceFriendlyName{device->getFriendlyName().data(), device->getFriendlyName().length()};
                Value truncated_mac_address_key{TRUNCATED_MAC_ADDRESS_KEY, strlen(TRUNCATED_MAC_ADDRESS_KEY)};
                Value _truncatedMac{truncatedMac.data(), truncatedMac.length()};
                deviceJson.AddMember(unique_device_id_key, _uuid);
                deviceJson.AddMember(friendly_name_key, deviceFriendlyName);
                deviceJson.AddMember(truncated_mac_address_key, _truncatedMac);
                rapidjson::Value metadataJson(rapidjson::kObjectType);
                auto metadata = device->getDeviceMetaData();
                if (metadata.vendorId.hasValue()) {
                    Value vendor_id_key{VENDOR_ID_KEY, strlen(VENDOR_ID_KEY)};
                    Value vendorId{metadata.vendorId.value()};
                    metadataJson.AddMember(vendor_id_key, vendorId);
                }
                if (metadata.productId.hasValue()) {
                    Value product_id_key{PRODUCT_ID_KEY, strlen(PRODUCT_ID_KEY)};
                    Value productId{metadata.productId.value()};
                    metadataJson.AddMember(product_id_key, productId);
                }
                Value class_of_device_key{CLASS_OF_DEVICE_KEY, strlen(CLASS_OF_DEVICE_KEY)};
                Value classOfDevice{metadata.classOfDevice};
                metadataJson.AddMember(class_of_device_key, classOfDevice);
                if (metadata.vendorDeviceSigId.hasValue()) {
                    Value vendor_device_sig_id_key{VENDOR_DEVICE_SIG_ID_KEY, strlen(VENDOR_DEVICE_SIG_ID_KEY)};
                    Value vendorDeviceSigId{metadata.vendorDeviceSigId.value()};
                    metadataJson.AddMember(vendor_device_sig_id_key, vendorDeviceSigId);
                }
                if (metadata.vendorDeviceId.hasValue()) {
                    Value vendor_device_id_key{VENDOR_DEVICE_ID_KEY, strlen(VENDOR_DEVICE_ID_KEY)};
                    Value vendorDeviceId{metadata.vendorDeviceId.value().data(), metadata.vendorDeviceId.value().length()};
                    metadataJson.AddMember(vendorDeviceId, vendorDeviceId);
                }
                Value metadata_key{METADATA_KEY, strlen(METADATA_KEY)};
                Value object{metadataJson.GetObject()};
                deviceJson.AddMember(metadata_key, object);
                devicesArray.PushBack(deviceJson);
            }
            Value discovered_devices_key{DISCOVERED_DEVICES_KEY, strlen(DISCOVERED_DEVICES_KEY)};
            Value array{devicesArray.GetArray()};
            Value has_more_key{HAS_MORE_KEY, strlen(HAS_MORE_KEY)};
            Value _hasMore{hasMore};
            payload.AddMember(discovered_devices_key, array);
            payload.AddMember(has_more_key, _hasMore);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(SCAN_DEVICES_REPORT.name, buffer.GetString());
        }
        void Bluetooth::executeSendScanDevicesFailed() {
            executeSendEvent(SCAN_DEVICES_FAILED.name, EMPTY_PAYLOAD);
        }
        void Bluetooth::executeSendEnterDiscoverableModeSucceeded() {
            executeSendEvent(ENTER_DISCOVERABLE_MODE_SUCCEEDED.name, EMPTY_PAYLOAD);
        }
        void Bluetooth::executeSendEnterDiscoverableModeFailed() {
            executeSendEvent(ENTER_DISCOVERABLE_MODE_FAILED.name, EMPTY_PAYLOAD);
        }
        void Bluetooth::executeSendPairDevicesSucceeded(
            const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices) {
            if (devices.empty()) {
                ACSDK_ERROR(LX(__func__).d("reason", "emptyDevices"));
                return;
            }
            Document payload(kObjectType);
            Value devicesJson(kArrayType);
            for (const auto& device : devices) {
                string uuid;
                if (!retrieveUuid(device->getMac(), &uuid)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed"));
                    continue;
                }
                Value deviceJson(kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value _uuid{uuid.data(), uuid.length()};
                Value friendly_name_key{FRIENDLY_NAME_KEY, strlen(FRIENDLY_NAME_KEY)};
                Value deviceFriendlyName{device->getFriendlyName().data(), device->getFriendlyName().length()};
                deviceJson.AddMember(unique_device_id_key, _uuid);
                deviceJson.AddMember(friendly_name_key, deviceFriendlyName);
                DeviceCategory category = DeviceCategory::UNKNOWN;
                if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                    ACSDK_WARN(LX(__func__).d("reason", "retrieveDeviceCategoryByUuidFailed"));
                    category = DeviceCategory::UNKNOWN;
                }
                Value device_category_key{DEVICE_CATEGORY_KEY, strlen(DEVICE_CATEGORY_KEY)};
                Value deviceCategory{deviceCategoryToString(category).data(), deviceCategoryToString(category).length()};
                deviceJson.AddMember(device_category_key, deviceCategory);
                rapidjson::Value supportedProfiles(rapidjson::kArrayType);
                if (!extractAvsProfiles(device, payload.GetAllocator(), &supportedProfiles)) supportedProfiles = Value(kArrayType);
                Value profiles_key{PROFILES_KEY, strlen(PROFILES_KEY)};
                deviceJson.AddMember(profiles_key, supportedProfiles);
                devicesJson.PushBack(deviceJson);
            }
            Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
            Value _devicesJson{devicesJson.GetArray()};
            payload.AddMember(devices_key, _devicesJson);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(PAIR_DEVICES_SUCCEEDED.name, buffer.GetString());
        }
        void Bluetooth::executeSendPairFailedEvent(const string& eventName, const unordered_set<string>& uuids) {
            Document payload(kObjectType);
            Value devicesJson(kArrayType);
            if (!uuids.empty()) {
                for (const auto& uuid : uuids) {
                    Value deviceJson(rapidjson::kObjectType);
                    Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                    Value _uuid{uuid.data(), uuid.length()};
                    deviceJson.AddMember(unique_device_id_key, _uuid);
                    devicesJson.PushBack(deviceJson);
                }
                Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
                Value _devicesJson{devicesJson.GetArray()};
                payload.AddMember(devices_key, _devicesJson);
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                if (!payload.Accept(writer)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                    return;
                }
                executeUpdateContext();
                executeSendEvent(eventName, buffer.GetString());
            }
        }
        void Bluetooth::executeSendPairDevicesFailed(const unordered_set<string>& uuids) {
            executeSendPairFailedEvent(PAIR_DEVICES_FAILED.name, uuids);
        }
        void Bluetooth::executeSendUnpairDevicesSucceeded(const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices) {
            if (devices.empty()) {
                ACSDK_ERROR(LX(__func__).d("reason", "emptyDevices"));
                return;
            }
            Document payload(kObjectType);
            Value devicesJson(kArrayType);
            for (const auto& device : devices) {
                string uuid;
                if (!retrieveUuid(device->getMac(), &uuid)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed"));
                    continue;
                }
                Value deviceJson(rapidjson::kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value _uuid{uuid.data(), uuid.length()};
                deviceJson.AddMember(unique_device_id_key, _uuid);
                devicesJson.PushBack(deviceJson);
            }
            Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
            Value array{devicesJson.GetArray()};
            payload.AddMember(devices_key, array);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(UNPAIR_DEVICES_SUCCEEDED.name, buffer.GetString());
        }
        void Bluetooth::executeSendUnpairDevicesFailed(const unordered_set<string>& uuids) {
            executeSendPairFailedEvent(UNPAIR_DEVICES_FAILED.name, uuids);
        }
        void Bluetooth::executeSetDeviceCategoriesSucceeded(const map<string, string>& uuidCategoryMap) {
            Document payload(kObjectType);
            Value categorizedDevices(kArrayType);
            for (const auto& uuidCategory : uuidCategoryMap) {
                Value deviceJson(kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value uuidCategoryFirst{uuidCategory.first.data(), uuidCategory.first.length()};
                Value device_category_key{DEVICE_CATEGORY_KEY, strlen(DEVICE_CATEGORY_KEY)};
                Value uuidCategorySecond{uuidCategory.second.data(), uuidCategory.second.length()};
                deviceJson.AddMember(unique_device_id_key, uuidCategoryFirst);
                deviceJson.AddMember(device_category_key, uuidCategorySecond);
                categorizedDevices.PushBack(deviceJson);
            }
            Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
            Value array{categorizedDevices.GetArray()};
            payload.AddMember(devices_key, array);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeQueueEventAndRequestContext(SET_DEVICE_CATEGORIES_SUCCEEDED.name, buffer.GetString());
        }
        void Bluetooth::executeSetDeviceCategoriesFailed(const map<string, string>& uuidCategoryMap) {
            Document payload(kObjectType);
            Value categorizedDevices(kArrayType);
            for (const auto& uuidCategory : uuidCategoryMap) {
                Value deviceJson(kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value uuidCategoryFirst{uuidCategory.first.data(), uuidCategory.first.length()};
                deviceJson.AddMember(unique_device_id_key, uuidCategoryFirst);
                auto device = retrieveDeviceByUuid(uuidCategory.first);
                if (device) {
                    Value metadataJson(kObjectType);
                    auto metadata = device->getDeviceMetaData();
                    if (metadata.vendorId.hasValue()) {
                        Value vendor_id_key{VENDOR_ID_KEY, strlen(VENDOR_ID_KEY)};
                        Value vendorIdValue{metadata.vendorId.value()};
                        metadataJson.AddMember(vendor_id_key, vendorIdValue);
                    }
                    if (metadata.productId.hasValue()) {
                        Value product_id_key{PRODUCT_ID_KEY, strlen(PRODUCT_ID_KEY)};
                        Value productIdValue{metadata.productId.value()};
                        metadataJson.AddMember(product_id_key, productIdValue);
                    }
                    Value class_of_device_key{CLASS_OF_DEVICE_KEY, strlen(CLASS_OF_DEVICE_KEY)};
                    Value classOfDevice{metadata.classOfDevice};
                    metadataJson.AddMember(class_of_device_key, classOfDevice);
                    if (metadata.vendorDeviceSigId.hasValue()) {
                        Value vendor_device_sig_id_key{VENDOR_DEVICE_SIG_ID_KEY, strlen(VENDOR_DEVICE_SIG_ID_KEY)};
                        Value deviceSigIdValue{metadata.vendorDeviceSigId.value()};
                        metadataJson.AddMember(vendor_device_sig_id_key, deviceSigIdValue);
                    }
                    if (metadata.vendorDeviceId.hasValue()) {
                        Value vendor_device_id_key{VENDOR_DEVICE_ID_KEY, strlen(VENDOR_DEVICE_ID_KEY)};
                        Value vendorDeviceId{metadata.vendorDeviceId.value().data(), metadata.vendorDeviceId.value().length()};
                        metadataJson.AddMember(vendor_device_id_key, vendorDeviceId);
                    }
                    Value metadata_key{METADATA_KEY, strlen(METADATA_KEY)};
                    Value dataJson{metadataJson.GetObject()};
                    deviceJson.AddMember(metadata_key, dataJson);
                } else { ACSDK_ERROR(LX(__func__).d("reason", "deviceNotFound").d("uuid", uuidCategory.first)); }
                categorizedDevices.PushBack(deviceJson);
            }
            Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
            Value categorizedDevicesObject{categorizedDevices.GetObject()};
            payload.AddMember(devices_key, categorizedDevicesObject);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeQueueEventAndRequestContext(SET_DEVICE_CATEGORIES_FAILED.name, buffer.GetString());
        }
        void Bluetooth::executeSendConnectByDeviceIdsSucceeded(const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices, Requester requester) {
            if (devices.empty()) {
                ACSDK_ERROR(LX(__func__).d("reason", "emptyDevices"));
                return;
            }
            Document payload(kObjectType);
            Value devicesJson(kArrayType);
            for (const auto& device : devices) {
                string uuid;
                if (!retrieveUuid(device->getMac(), &uuid)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed"));
                    continue;
                }
                Value deviceJson(kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value _uuid{uuid.data(), uuid.length()};
                Value friendly_name_key{FRIENDLY_NAME_KEY, strlen(FRIENDLY_NAME_KEY)};
                Value deviceFriendlyName{device->getFriendlyName().data(), device->getFriendlyName().length()};
                deviceJson.AddMember(unique_device_id_key, _uuid);
                deviceJson.AddMember(friendly_name_key, deviceFriendlyName);
                devicesJson.PushBack(deviceJson);
            }
            Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
            Value array{devicesJson.GetArray()};
            Value requester_key{REQUESTER_KEY, strlen(REQUESTER_KEY)};
            Value _requester{requesterToString(requester).data(), requesterToString(requester).length()};
            payload.AddMember(devices_key, array);
            payload.AddMember(requester_key, _requester);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(CONNECT_BY_DEVICE_IDS_SUCCEEDED.name, buffer.GetString());
        }
        void Bluetooth::executeSendConnectFailedEvent(const string& eventName, const unordered_set<string>& uuids, Requester requester) {
            if (uuids.empty()) {
                ACSDK_ERROR(LX(__func__).d("reason", "emptyUuids").d("eventName", eventName));
                return;
            }
            Document payload(kObjectType);
            Value devicesJson(kArrayType);
            for (const auto& uuid : uuids) {
                Value deviceJson(kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value _uuid{uuid.data(), uuid.length()};
                Value friendly_name_key{FRIENDLY_NAME_KEY, strlen(FRIENDLY_NAME_KEY)};
                Value value{"", strlen("")};
                deviceJson.AddMember(unique_device_id_key, _uuid);
                deviceJson.AddMember(friendly_name_key, value);
                devicesJson.PushBack(deviceJson);
            }
            Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
            Value array{devicesJson.GetArray()};
            Value requester_key{REQUESTER_KEY, strlen(REQUESTER_KEY)};
            Value _requester{requesterToString(requester).data(), requesterToString(requester).length()};
            payload.AddMember(devices_key, array);
            payload.AddMember(requester_key, _requester);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(eventName, buffer.GetString());
        }
        void Bluetooth::executeSendConnectByDeviceIdsFailed(const unordered_set<string>& uuids, Requester requester) {
            executeSendConnectFailedEvent(CONNECT_BY_DEVICE_IDS_FAILED.name, uuids, requester);
        }
        void Bluetooth::executeSendConnectByProfileSucceeded(shared_ptr<BluetoothDeviceInterface> device, const string& profileName, Requester requester) {
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return;
            }
            Document payload(kObjectType);
            string uuid;
            if (!retrieveUuid(device->getMac(), &uuid)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed"));
                return;
            }
            Value deviceJson(kObjectType);
            Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
            Value _uuid{uuid.data(), uuid.length()};
            Value device_key{DEVICE_KEY, strlen(DEVICE_KEY)};
            Value deviceJsonObject{deviceJson.GetObject()};
            Value profile_name_key{PROFILE_NAME_KEY, strlen(PROFILE_NAME_KEY)};
            Value _profileName{profileName.data(), profileName.length()};
            Value requester_key{REQUESTER_KEY, strlen(REQUESTER_KEY)};
            Value _requester{requesterToString(requester).data(), requesterToString(requester).length()};
            deviceJson.AddMember(unique_device_id_key, _uuid);
            payload.AddMember(device_key, deviceJsonObject);
            payload.AddMember(profile_name_key, _profileName);
            payload.AddMember(requester_key, _requester);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(CONNECT_BY_PROFILE_SUCCEEDED.name, buffer.GetString());
        }
        void Bluetooth::executeSendConnectByProfileFailed(const string& profileName, Requester requester) {
            Document payload(kObjectType);
            Value requester_key{REQUESTER_KEY, strlen(REQUESTER_KEY)};
            Value _requester{requesterToString(requester).data(), requesterToString(requester).length()};
            Value profile_name_key{PROFILE_NAME_KEY, strlen(PROFILE_NAME_KEY)};
            Value _profileName{profileName.data(), profileName.length()};
            payload.AddMember(requester_key, _requester);
            payload.AddMember(profile_name_key, _profileName);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(CONNECT_BY_PROFILE_FAILED.name, buffer.GetString());
        }
        void Bluetooth::executeSendDisconnectDevicesSucceeded(const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices, Requester requester) {
            if (devices.empty()) {
                ACSDK_ERROR(LX(__func__).d("reason", "emptyDevices"));
                return;
            }
            Document payload(kObjectType);
            Value devicesJson(kArrayType);
            for (const auto& device : devices) {
                std::string uuid;
                if (!retrieveUuid(device->getMac(), &uuid)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed"));
                    continue;
                }
                Value deviceJson(kObjectType);
                Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
                Value _uuid{uuid.data(), uuid.length()};
                Value friendly_name_key{FRIENDLY_NAME_KEY, strlen(FRIENDLY_NAME_KEY)};
                Value deviceFriendlyName{device->getFriendlyName().data(), device->getFriendlyName().length()};
                deviceJson.AddMember(unique_device_id_key, _uuid);
                deviceJson.AddMember(friendly_name_key, deviceFriendlyName);
                devicesJson.PushBack(deviceJson);
            }
            Value devices_key{DEVICES_KEY, strlen(DEVICES_KEY)};
            Value array{devicesJson.GetArray()};
            Value requester_key{REQUESTER_KEY, strlen(REQUESTER_KEY)};
            Value _requester{requesterToString(requester).data(), requesterToString(requester).length()};
            payload.AddMember(devices_key, array);
            payload.AddMember(requester_key, _requester);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(DISCONNECT_DEVICES_SUCCEEDED.name, buffer.GetString());
        }
        void Bluetooth::executeSendDisconnectDevicesFailed(const unordered_set<string>& uuids, Requester requester) {
            executeSendConnectFailedEvent(DISCONNECT_DEVICES_FAILED.name, uuids, requester);
        }
        void Bluetooth::executeSendMediaControlEvent(const string& eventName, shared_ptr<BluetoothDeviceInterface> device) {
            Document payload(kObjectType);
            string uuid;
            if (!retrieveUuid(device->getMac(), &uuid)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed"));
                return;
            }
            Value deviceJson(kObjectType);
            Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
            Value _uuid{uuid.data(), uuid.length()};
            Value device_key{DEVICE_KEY, strlen(DEVICE_KEY)};
            Value deviceJsonObject{deviceJson.GetObject()};
            deviceJson.AddMember(unique_device_id_key, _uuid);
            payload.AddMember(device_key, deviceJsonObject);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeSendEvent(eventName, buffer.GetString());
        }
        void Bluetooth::executeSendMediaControlPlaySucceeded(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_PLAY_SUCCEEDED.name, device);
        }
        void Bluetooth::executeSendMediaControlPlayFailed(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_PLAY_FAILED.name, device);
        }
        void Bluetooth::executeSendMediaControlStopSucceeded(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_STOP_SUCCEEDED.name, device);
        }
        void Bluetooth::executeSendMediaControlStopFailed(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_STOP_FAILED.name, device);
        }
        void Bluetooth::executeSendMediaControlNextSucceeded(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_NEXT_SUCCEEDED.name, device);
        }
        void Bluetooth::executeSendMediaControlNextFailed(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_NEXT_FAILED.name, device);
        }
        void Bluetooth::executeSendMediaControlPreviousSucceeded(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_PREVIOUS_SUCCEEDED.name, device);
        }
        void Bluetooth::executeSendMediaControlPreviousFailed(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendMediaControlEvent(MEDIA_CONTROL_PREVIOUS_FAILED.name, device);
        }
        void Bluetooth::executeSendStreamingEvent(const string& eventName, shared_ptr<BluetoothDeviceInterface> device) {
            Document payload(kObjectType);
            string uuid;
            if (!device) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                return;
            }
            if (!retrieveUuid(device->getMac(), &uuid)) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveUuidFailed"));
                return;
            }
            Value deviceJson(kObjectType);
            Value unique_device_id_key{UNIQUE_DEVICE_ID_KEY, strlen(UNIQUE_DEVICE_ID_KEY)};
            Value _uuid{uuid.data(), uuid.length()};
            deviceJson.AddMember(unique_device_id_key, _uuid);
            if (supportsAvsProfile(device, AVS_A2DP_SINK)) {
                Value profile_name_key{PROFILE_NAME_KEY, strlen(PROFILE_NAME_KEY)};
                Value avsA2DPSink{AVS_A2DP_SINK.data(), AVS_A2DP_SINK.length()};
                deviceJson.AddMember(profile_name_key, avsA2DPSink);
            } else if (supportsAvsProfile(device, AVS_A2DP_SOURCE)) {
                Value profile_name_key{PROFILE_NAME_KEY, strlen(PROFILE_NAME_KEY)};
                Value avsA2DPSource{AVS_A2DP_SOURCE.data(), AVS_A2DP_SOURCE.length()};
                deviceJson.AddMember(profile_name_key, avsA2DPSource);
            }
            Value device_key{DEVICE_KEY, strlen(DEVICE_KEY)};
            Value deviceJsonObject{deviceJson.GetObject()};
            payload.AddMember(device_key, deviceJsonObject);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
                return;
            }
            executeUpdateContext();
            executeQueueEventAndRequestContext(eventName, buffer.GetString());
        }
        void Bluetooth::executeSendStreamingStarted(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendStreamingEvent(STREAMING_STARTED.name, device);
        }
        void Bluetooth::executeSendStreamingEnded(shared_ptr<BluetoothDeviceInterface> device) {
            executeSendStreamingEvent(STREAMING_ENDED.name, device);
        }
        void Bluetooth::executeAcquireFocus(const string& callingMethodName) {
            if (FocusState::FOREGROUND == m_focusState || FocusState::BACKGROUND == m_focusState) {
                ACSDK_DEBUG9(LX(__func__).d("focus", m_focusState).d("callingMethodName", callingMethodName).m("Already acquired channel"));
                return;
            }
            auto activity = FocusManagerInterface::Activity::create(ACTIVITY_ID,shared_from_this(),TRANSIENT_FOCUS_DURATION,ContentType::MIXABLE);
            if (!activity) { ACSDK_ERROR(LX(__func__).d("reason", "activityCreateFailed").d("callingMethodName", callingMethodName)); }
            else if (!m_focusManager->acquireChannel(CHANNEL_NAME, activity)) {
                ACSDK_ERROR(LX(__func__).d("reason", "acquireChannelFailed").d("callingMethodName", callingMethodName));
            } else { ACSDK_DEBUG1(LX(__func__).d("callingMethodName", callingMethodName).m("Acquiring channel")); }
        }
        void Bluetooth::executeReleaseFocus(const string& callingMethodName) {
            m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
            m_focusTransitionState = FocusTransitionState::PENDING_INTERNAL;
            ACSDK_DEBUG1(LX(__func__).d("callingMethodName", callingMethodName).m("Releasing channel"));
        }
        void Bluetooth::onFormattedAudioStreamAdapterData(AudioFormat audioFormat, const unsigned char* buffer, size_t size) {
            AttachmentWriter::WriteStatus writeStatus;
            if (m_mediaAttachment) m_mediaAttachmentWriter->write(buffer, size, &writeStatus);
        }
        template <typename ServiceType> shared_ptr<ServiceType> Bluetooth::getService(shared_ptr<BluetoothDeviceInterface> device) {
            ACSDK_DEBUG5(LX(__func__).d("uuid", ServiceType::UUID));
            shared_ptr<ServiceType> service = nullptr;
            {
                if (!device) { ACSDK_DEBUG5(LX(__func__).d("reason", "nullDevice")); }
                else { service = std::static_pointer_cast<ServiceType>(device->getService(ServiceType::UUID)); }
            }
            return service;
        }
        void Bluetooth::setCurrentStream(shared_ptr<FormattedAudioStreamAdapter> stream) {
            ACSDK_DEBUG5(LX(__func__));
            if (stream == m_mediaStream) {
                ACSDK_DEBUG5(LX(__func__).d("reason", "sameStream"));
                return;
            }
            if (m_mediaAttachmentReader) {
                m_mediaAttachmentReader->close();
                m_mediaAttachmentReader.reset();
            }
            if (m_mediaAttachmentWriter) {
                m_mediaAttachmentWriter->close();
                m_mediaAttachmentWriter.reset();
            }
            if (m_mediaStream) m_mediaStream->setListener(nullptr);
            m_mediaStream = stream;
            if (m_mediaStream) {
                m_mediaAttachment = make_shared<InProcessAttachment>("Bluetooth");
                m_mediaAttachmentReader = m_mediaAttachment->createReader(ReaderPolicy::NONBLOCKING);
                m_mediaAttachmentWriter = m_mediaAttachment->createWriter(WriterPolicy::ALL_OR_NOTHING);
                m_mediaStream->setListener(shared_from_this());
                auto audioFormat = m_mediaStream->getAudioFormat();
                m_sourceId = m_mediaPlayer->setSource(m_mediaAttachmentReader, &audioFormat);
                if (MediaPlayerInterface::ERROR == m_sourceId) {
                    ACSDK_ERROR(LX(__func__).d("reason", "setSourceFailed"));
                    m_mediaAttachment.reset();
                    m_mediaAttachmentWriter.reset();
                    m_mediaStream.reset();
                }
            }
        }
        void Bluetooth::executeRestrictA2DPDevices() {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_restrictedDevices.empty()) {
                ACSDK_ERROR(LX("failedToRestrictDevices").d("reason", "restrictedListNotEmpty"));
                return;
            }
            m_disabledA2DPDevice = m_activeA2DPDevice;
            for (const auto& device : m_deviceManager->getDiscoveredDevices()) {
                if (device->isPaired()) {
                    auto supportA2DP = false;
                    if (device->getService(A2DPSinkInterface::UUID) != nullptr) {
                        device->toggleServiceConnection(false, device->getService(A2DPSinkInterface::UUID));
                        supportA2DP = true;
                    }
                    if (device->getService(A2DPSourceInterface::UUID) != nullptr) {
                        device->toggleServiceConnection(false, device->getService(A2DPSourceInterface::UUID));
                        supportA2DP = true;
                    }
                    if (supportA2DP) m_restrictedDevices.push_back(device);
                }
            }
        }
        void Bluetooth::executeUnrestrictA2DPDevices() {
            ACSDK_INFO(LX(__func__));
            if (!m_restrictedDevices.empty()) {
                for (const auto& device : m_restrictedDevices) {
                    if (device->getService(A2DPSinkInterface::UUID) != nullptr) {
                        device->toggleServiceConnection(true, device->getService(A2DPSinkInterface::UUID));
                    }
                    if (device->getService(A2DPSourceInterface::UUID) != nullptr) {
                        device->toggleServiceConnection(true, device->getService(A2DPSourceInterface::UUID));
                    }
                }
                m_restrictedDevices.clear();
            }
            if (m_disabledA2DPDevice) {
                bool supportsA2DP = supportsAvsProfile(m_disabledA2DPDevice, AVS_A2DP);
                if (!supportsA2DP) { ACSDK_DEBUG0(LX(__func__).d("reason", "noSupportedA2DPRoles").m("Connect Request Rejected")); }
                else {
                    if (!executeFunctionOnDevice(m_disabledA2DPDevice, &BluetoothDeviceInterface::connect)) {
                        std::string uuid;
                        if (!retrieveUuid(m_disabledA2DPDevice->getMac(), &uuid)) {
                            ACSDK_ERROR(LX("executeUnrestrictA2DPDevicesFailed").d("reason", "retrieveUuidFailed"));
                        } else executeSendConnectByDeviceIdsFailed({uuid}, Requester::DEVICE);
                    }
                }
                m_disabledA2DPDevice.reset();
            }
        }
        void Bluetooth::addObserver(shared_ptr<ObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                return;
            }
            m_executor.submit([this, observer]() { m_observers.insert(observer); });
        }
        void Bluetooth::removeObserver(shared_ptr<ObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                return;
            }
            m_executor.submit([this, observer]() { m_observers.erase(observer); });
        }
        Bluetooth::ObserverInterface::DeviceAttributes Bluetooth::generateDeviceAttributes(
            shared_ptr<BluetoothDeviceInterface> device) {
            ObserverInterface::BluetoothDeviceObserverInterface::DeviceAttributes deviceAttributes;
            deviceAttributes.name = device->getFriendlyName();
            for (const auto& supportedServices : device->getSupportedServices()) {
                deviceAttributes.supportedServices.insert(supportedServices->getName());
            }
            return deviceAttributes;
        }
        void Bluetooth::executeInsertBluetoothEventState(shared_ptr<BluetoothDeviceInterface> device, DeviceState state, Optional<Requester> requester,
                                                         Optional<string> profileName) {
            if (!device) {
                ACSDK_ERROR(LX("insertBluetoothEventStateFailed").d("reason", "nullDevice"));
                return;
            }
            unordered_set<shared_ptr<BluetoothEventState>> bluetoothEventStates;
            const string mac = device->getMac();
            auto it = m_bluetoothEventStates.find(mac);
            if (it != m_bluetoothEventStates.end()) {
                bluetoothEventStates.insert(it->second.begin(), it->second.end());
                m_bluetoothEventStates.erase(mac);
            }
            shared_ptr<BluetoothEventState> eventState = make_shared<BluetoothEventState>(state, requester, profileName);
            bluetoothEventStates.insert(eventState);
            m_bluetoothEventStates.insert({mac, bluetoothEventStates});
        }
        shared_ptr<BluetoothEventState> Bluetooth::executeRemoveBluetoothEventState(
            shared_ptr<BluetoothDeviceInterface> device,
            DeviceState state) {
            const string mac = device->getMac();
            auto it = m_bluetoothEventStates.find(mac);
            if (it != m_bluetoothEventStates.end()) {
                unordered_set<std::shared_ptr<BluetoothEventState>> bluetoothEventStates = it->second;
                for (const auto& bluetoothEventState : bluetoothEventStates) {
                    if (bluetoothEventState->getDeviceState() == state) {
                        auto event = bluetoothEventState;
                        bluetoothEventStates.erase(event);
                        if (!bluetoothEventStates.empty()) m_bluetoothEventStates[mac] = bluetoothEventStates;
                        else m_bluetoothEventStates.erase(mac);
                        return event;
                    }
                }
                ACSDK_DEBUG5(LX(__func__).d("reason", "noDeviceStateFound"));
                return nullptr;
            }
            ACSDK_DEBUG5(LX(__func__).d("reason", "noDeviceFound"));
            return nullptr;
        }
        void Bluetooth::onEventFired(const BluetoothEvent& event) {
            ACSDK_DEBUG5(LX(__func__));
            switch(event.getType()) {
                case BluetoothEventType::DEVICE_DISCOVERED: {
                    auto device = event.getDevice();
                    if (!device) {
                        ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                        return;
                    }
                    ACSDK_INFO(LX(__func__).d("reason", "DEVICE_DISCOVERED").d("deviceName", truncateFriendlyName(device->getFriendlyName()))
                        .d("mac", truncateWithDefault(device->getMac())));
                    m_executor.submit([this] {
                        if (ScanningTransitionState::ACTIVE == m_scanningTransitionState) {
                            executeSendScanDevicesReport(m_deviceManager->getDiscoveredDevices(), true);
                        }
                    });
                    break;
                }
                case BluetoothEventType::SCANNING_STATE_CHANGED:
                    ACSDK_INFO(LX(__func__).d("reason", "SCANNING_STATE_CHANGED").d("isScanning", event.isScanning()));
                    m_executor.submit([this, event] {
                        bool isScanning = event.isScanning();
                        if (!isScanning) {
                            if (ScanningTransitionState::PENDING_INACTIVE == m_scanningTransitionState) {
                                ACSDK_DEBUG5(LX(__func__).d("reason", "PENDING_INACTIVE resolved").d("m_scanningTransitionState", m_scanningTransitionState)
                                    .d("isScanning", event.isScanning()));
                                m_scanningTransitionState = ScanningTransitionState::INACTIVE;
                            } else executeSendScanDevicesReport(m_deviceManager->getDiscoveredDevices(), isScanning);
                        }
                    });
                    break;
                case BluetoothEventType::DEVICE_STATE_CHANGED: {
                        auto device = event.getDevice();
                        if (!device) {
                            ACSDK_ERROR(LX(__func__).d("reason", "nullDevice"));
                            return;
                        }
                        ACSDK_INFO(LX(__func__).d("event", "DEVICE_STATE_CHANGED").d("deviceName", truncateFriendlyName(device->getFriendlyName()))
                                       .d("mac", truncateWithDefault(device->getMac())).d("state", event.getDeviceState()));
                        switch(event.getDeviceState()) {
                            case DeviceState::FOUND: case DeviceState::IDLE: break;
                            case DeviceState::PAIRED:
                                m_executor.submit([this, device] {
                                    executeSendScanDevicesReport(m_deviceManager->getDiscoveredDevices(), true);
                                    executeRemoveBluetoothEventState(device, DeviceState::PAIRED);
                                    unordered_set<std::shared_ptr<BluetoothDeviceInterface>> devices({device});
                                    executeSendPairDevicesSucceeded(devices);
                                });
                                break;
                            case DeviceState::DISCONNECTED:
                                m_executor.submit([this, device] {
                                    shared_ptr<BluetoothEventState> disconnectEvent = executeRemoveBluetoothEventState(device, DeviceState::DISCONNECTED);
                                    if (disconnectEvent) {
                                        Optional<Requester> requester = disconnectEvent->getRequester();
                                        if (requester.hasValue()) {
                                            unordered_set<shared_ptr<BluetoothDeviceInterface>> devices({device});
                                            executeSendDisconnectDevicesSucceeded(devices, requester.value());
                                        } else {
                                            ACSDK_ERROR(LX(__func__).d("reason", "sendDisconnectDeviceSucceededEventFailed").d("error", "retrieveDisconnectRequesterFailed"));
                                        }
                                    } else {
                                        DeviceCategory category;
                                        string uuid;
                                        if (!retrieveUuid(device->getMac(), &uuid)) {
                                            ACSDK_ERROR(LX(__func__).d("reason", "disconnectDeviceFailed").d("error", "retrieveDisconnectedDeviceUuidFailed"));
                                            return;
                                        }
                                        if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                                            ACSDK_ERROR(LX(__func__).d("reason", "disconnectDeviceFailed").d("error", "retrieveDisconnectedDeviceCategoryFailed"));
                                            return;
                                        }
                                        ACSDK_DEBUG9(LX(__func__).d("uuid", uuid).d("deviceCategory", category));
                                        auto it = m_connectedDevices.find(category);
                                        if (it != m_connectedDevices.end() && it->second.find(device) != it->second.end()) {
                                            executeOnDeviceDisconnect(device, Requester::DEVICE);
                                            executeRemoveBluetoothEventState(device, DeviceState::DISCONNECTED);
                                            unordered_set<shared_ptr<BluetoothDeviceInterface>> devices({device});
                                            executeSendDisconnectDevicesSucceeded(devices, Requester::DEVICE);
                                        } else {
                                            ACSDK_ERROR(LX(__func__).d("reason", "disconnectDeviceFailed").d("error", "deviceNotConnectedBefore")
                                                .d("deviceName", truncateFriendlyName(device->getFriendlyName())).d("mac", truncateWithDefault(device->getMac())));
                                        }
                                    }
                                });
                                break;
                            case DeviceState::UNPAIRED:
                                m_executor.submit([this, device] {
                                    executeRemoveBluetoothEventState(device, DeviceState::UNPAIRED);
                                    unordered_set<shared_ptr<BluetoothDeviceInterface>> devices({device});
                                    executeSendUnpairDevicesSucceeded(devices);
                                });
                                break;
                            case DeviceState::CONNECTED: {
                                m_executor.submit([this, device] {
                                    shared_ptr<BluetoothEventState> connectEvent = executeRemoveBluetoothEventState(device, DeviceState::CONNECTED);
                                    if (connectEvent) {
                                        Optional<Requester> requester = connectEvent->getRequester();
                                        Optional<string> profileName = connectEvent->getProfileName();
                                        if (!requester.hasValue()) {
                                            ACSDK_ERROR(LX(__func__).d("reason", profileName.hasValue() ? "sendConnectByProfileFailed" : "sendConnectByDeviceIdsFailed")
                                                .d("error", "retrieveConnectRequesterFailed"));
                                            return;
                                        }
                                        if (profileName.hasValue()) executeSendConnectByProfileSucceeded(device, profileName.value(), requester.value());
                                        else {
                                            unordered_set<shared_ptr<BluetoothDeviceInterface>> devices({device});
                                            executeSendConnectByDeviceIdsSucceeded(devices, requester.value());
                                        }
                                    } else {
                                        DeviceCategory category;
                                        string uuid;
                                        if (!retrieveUuid(device->getMac(), &uuid)) {
                                            ACSDK_ERROR(LX(__func__).d("reason", "connectDeviceFailed").d("error", "retrieveConnectedDeviceCategoryFailed"));
                                            return;
                                        }
                                        if (!retrieveDeviceCategoryByUuid(uuid, &category)) {
                                            ACSDK_ERROR(LX(__func__).d("reason", "connectDeviceFailed").d("error", "retrieveConnectedDeviceCategoryFailed"));
                                            return;
                                        }
                                        auto it = m_connectedDevices.find(category);
                                        if (it == m_connectedDevices.end() || (it != m_connectedDevices.end() && it->second.find(device) == it->second.end())) {
                                            executeOnDeviceConnect(device, true);
                                            unordered_set<shared_ptr<BluetoothDeviceInterface>> devices({device});
                                            executeSendConnectByDeviceIdsSucceeded(devices, Requester::DEVICE);
                                        } else {
                                            ACSDK_ERROR(LX(__func__).d("reason", "connectDeviceFailed").d("error", "deviceAlreadyConnectedBefore")
                                                .d("deviceName", truncateFriendlyName(device->getFriendlyName())).d("mac", truncateWithDefault(device->getMac())));
                                        }
                                    }
                                });
                                break;
                            }
                        }
                        break;
                    }
                case BluetoothEventType::STREAMING_STATE_CHANGED: {
                    if (event.getDevice() != m_activeA2DPDevice) {
                        ACSDK_ERROR(LX(__func__).d("reason", "mismatchedDevices").d("eventDevice", event.getDevice() ? event.getDevice()->getMac() : "null")
                            .d("activeDevice", m_activeA2DPDevice ? m_activeA2DPDevice->getMac() : "null"));
                        break;
                    }
                    if (!event.getA2DPRole()) {
                        ACSDK_ERROR(LX(__func__).d("reason", "nullRole"));
                        break;
                    }
                    if (A2DPRole::SINK == *event.getA2DPRole()) {
                        if (MediaStreamingState::ACTIVE == event.getMediaStreamingState()) {
                            ACSDK_DEBUG5(LX(__func__).m("Streaming is active"));
                            m_executor.submit([this] {
                                if (m_streamingState == StreamingState::INACTIVE || m_streamingState == StreamingState::PAUSED ||
                                    m_streamingState == StreamingState::PENDING_PAUSED) {
                                    executeAcquireFocus(__func__);
                                }
                            });
                        } else if (MediaStreamingState::IDLE == event.getMediaStreamingState()) {
                            m_executor.submit([this] {
                                if (StreamingState::ACTIVE == m_streamingState) {
                                    if (FocusState::FOREGROUND == m_focusState || FocusState::BACKGROUND == m_focusState) {
                                        executeReleaseFocus(__func__);
                                    }
                                }
                            });
                        }
                    } else if (A2DPRole::SOURCE == *event.getA2DPRole()) {
                        if (MediaStreamingState::ACTIVE == event.getMediaStreamingState()) {
                            m_executor.submit([this] {
                                if (StreamingState::ACTIVE != m_streamingState) {
                                    m_streamingState = StreamingState::ACTIVE;
                                    executeSendStreamingStarted(m_activeA2DPDevice);
                                }
                            });
                        } else if (MediaStreamingState::IDLE == event.getMediaStreamingState()) {
                            m_executor.submit([this] {
                                if (StreamingState::ACTIVE == m_streamingState) {
                                    m_streamingState = StreamingState::PAUSED;
                                    executeSendStreamingEnded(m_activeA2DPDevice);
                                }
                            });
                        }
                    }
                    break;
                }
                case BluetoothEventType::TOGGLE_A2DP_PROFILE_STATE_CHANGED: {
                    ACSDK_DEBUG5(LX(__func__).d("event", "TOGGLE_A2DP_PROFILE_STATE_CHANGED").d("a2dpEnable", event.isA2DPEnabled()));
                    if (event.isA2DPEnabled()) m_executor.submit([this] { executeUnrestrictA2DPDevices(); });
                    else m_executor.submit([this] { executeRestrictA2DPDevices(); });
                    break;
                }
                default: ACSDK_ERROR(LX("onEventFired").d("reason", "unexpectedEventType"));
            }
        }
    }
}
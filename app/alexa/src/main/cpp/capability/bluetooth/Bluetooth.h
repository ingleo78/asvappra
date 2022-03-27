#ifndef ACSDKBLUETOOTH_BLUETOOTH_H_
#define ACSDKBLUETOOTH_BLUETOOTH_H_

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <queue>
#include <unordered_set>
#include <avs/attachment/InProcessAttachment.h>
#include <avs/AVSDirective.h>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/ExceptionErrorType.h>
#include <avs/FocusState.h>
#include <avs/Requester.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceConnectionRuleInterface.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ChannelVolumeInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ContextRequesterInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include <util/bluetooth/BluetoothEvents.h>
#include <util/bluetooth/DeviceCategory.h>
#include <media_player/MediaPlayerInterface.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <util/Optional.h>
#include <util/RequiresShutdown.h>
#include <util/bluetooth/FormattedAudioStreamAdapter.h>
#include <threading/Executor.h>
#include <registration_manager/CustomerDataHandler.h>
#include <registration_manager/CustomerDataManager.h>
#include "BluetoothEventState.h"
#include "BluetoothMediaInputTransformer.h"
#include "BluetoothStorageInterface.h"
#include "BluetoothDeviceObserverInterface.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace attachment;
        using namespace utils;
        using namespace json;
        using namespace sds;
        using namespace threading;
        using namespace sdkInterfaces;
        using namespace acsdkBluetoothInterfaces;
        using namespace rapidjson;
        using namespace sdkInterfaces::bluetooth;
        using namespace utils::bluetooth;
        using namespace utils::mediaPlayer;
        using namespace registrationManager;
        class Bluetooth : public enable_shared_from_this<Bluetooth>, public CapabilityAgent, public BluetoothEventListenerInterface,
                          public FormattedAudioStreamAdapterListener, public CapabilityConfigurationInterface, public MediaPlayerObserverInterface,
                          public RequiresShutdown, public CustomerDataHandler {
        public:
            using ObserverInterface = BluetoothDeviceObserverInterface;
            enum class StreamingState {
                INACTIVE,
                PAUSED,
                PENDING_PAUSED,
                PENDING_ACTIVE,
                ACTIVE
            };
            enum class FocusTransitionState {
                INTERNAL,
                PENDING_INTERNAL,
                EXTERNAL
            };
            enum class ScanningTransitionState {
                ACTIVE,
                PENDING_INACTIVE,
                INACTIVE
            };
            static shared_ptr<Bluetooth> create(shared_ptr<ContextManagerInterface> contextManager, shared_ptr<FocusManagerInterface> focusManager,
                                                shared_ptr<MessageSenderInterface> messageSender, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                shared_ptr<BluetoothStorageInterface> bluetoothStorage, unique_ptr<BluetoothDeviceManagerInterface> deviceManager,
                                                shared_ptr<BluetoothEventBus> eventBus, shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                shared_ptr<CustomerDataManager> customerDataManager, unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules,
                                                shared_ptr<ChannelVolumeInterface> bluetoothChannelVolumeInterface, shared_ptr<BluetoothMediaInputTransformer> mediaInputTransformer = nullptr);
            DirectiveHandlerConfiguration getConfiguration() const override;
            void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
            void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
            void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
            void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
            void onFocusChanged(FocusState newFocus, avsCommon::avs::MixingBehavior behavior) override;
            unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
            void onContextAvailable(const string& jsonContext) override;
            void onContextFailure(const ContextRequestError error) override;
            void doShutdown() override;
            void onFirstByteRead(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState& state) override;
            void onPlaybackStarted(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState& state) override;
            void onPlaybackStopped(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState& state) override;
            void onPlaybackFinished(MediaPlayerObserverInterface::SourceId id, const MediaPlayerState& state) override;
            void onPlaybackError(MediaPlayerObserverInterface::SourceId id, const ErrorType& type, std::string error, const MediaPlayerState& state) override;
            void clearData() override;
            void addObserver(shared_ptr<ObserverInterface> observer);
            void removeObserver(shared_ptr<ObserverInterface> observer);
        protected:
            void onEventFired(const BluetoothEvent& event) override;
        private:
            Bluetooth(shared_ptr<ContextManagerInterface> contextManager, shared_ptr<FocusManagerInterface> focusManager,
                      shared_ptr<MessageSenderInterface> messageSender, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                      shared_ptr<BluetoothStorageInterface> bluetoothStorage, unique_ptr<BluetoothDeviceManagerInterface>& deviceManager,
                      shared_ptr<BluetoothEventBus> eventBus, shared_ptr<MediaPlayerInterface> mediaPlayer,
                      shared_ptr<CustomerDataManager> customerDataManager, unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules,
                      shared_ptr<ChannelVolumeInterface> bluetoothChannelVolumeInterface, shared_ptr<BluetoothMediaInputTransformer> mediaInputTransformer);
            bool init();
            void syncWithDeviceManager();
            void executeUpdateContext();
            bool extractAvsProfiles(shared_ptr<BluetoothDeviceInterface> device, Document::AllocatorType& allocator, Value* supportedProfiles);
            void executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info);
            void removeDirective(shared_ptr<DirectiveInfo> info);
            void sendExceptionEncountered(shared_ptr<CapabilityAgent::DirectiveInfo> info, const string& message, ExceptionErrorType type);
            void executeEnterForeground();
            void executeEnterBackground(MixingBehavior behavior);
            void executeEnterNone();
            bool executeSetDiscoverableMode(bool discoverable);
            bool executeSetScanMode(bool scanning, bool shouldReport = true);
            bool executePairDevices(const unordered_set<string>& uuids);
            bool executeUnpairDevices(const unordered_set<string>& uuids);
            map<string, string> executeSetDeviceCategories( const map<string, string>& uuidCategoryMap);
            void executeConnectByDeviceIds(const unordered_set<string>& uuids);
            void executeConnectByProfile(const string& profileName, const string& profileVersion);
            void executeDisconnectDevices(const unordered_set<string>& uuids);
            void executeOnDeviceDisconnect(shared_ptr<BluetoothDeviceInterface> device, Requester requester);
            void executeOnDeviceConnect(shared_ptr<BluetoothDeviceInterface> device, bool shouldNotifyConnection = true);
            bool executeFunctionOnDevice(shared_ptr<BluetoothDeviceInterface>& device, function<future<bool>(shared_ptr<BluetoothDeviceInterface>&)> function);
            void executePlay(shared_ptr<BluetoothDeviceInterface> device);
            void executeStop(shared_ptr<BluetoothDeviceInterface> device);
            void executeNext(shared_ptr<BluetoothDeviceInterface> device);
            void executePrevious(shared_ptr<BluetoothDeviceInterface> device);
            void executeInitializeMediaSource();
            void cleanupMediaSource();
            void executeAbortMediaPlayback();
            void setCurrentStream(shared_ptr<FormattedAudioStreamAdapter> stream);
            void onFormattedAudioStreamAdapterData(AudioFormat audioFormat, const unsigned char* buffer, size_t size) override;
            shared_ptr<BluetoothDeviceInterface> retrieveDeviceByMac(const string& mac);
            shared_ptr<BluetoothDeviceInterface> retrieveDeviceByUuid(const string& uuid);
            bool retrieveDeviceCategoryByUuid(const string& uuid, DeviceCategory* category);
            shared_ptr<BluetoothDeviceConnectionRuleInterface> retrieveConnectionRuleByUuid(const string& uuid);
            bool retrieveUuid(const string& mac, string* uuid);
            unordered_set<string> retrieveUuidsFromConnectionPayload(const Document& payload);
            void clearUnusedUuids();
            void executeSendEvent(const string& eventName, const string& eventPayload);
            void executeQueueEventAndRequestContext(const string& eventName, const string& eventPayload);
            void executeSendScanDevicesReport(const list<shared_ptr<BluetoothDeviceInterface>>& devices, bool hasMore);
            void executeSendScanDevicesFailed();
            void executeSendEnterDiscoverableModeSucceeded();
            void executeSendEnterDiscoverableModeFailed();
            void executeSendPairDevicesSucceeded(const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices);
            void executeSendPairFailedEvent(const string& eventName, const unordered_set<string>& uuids);
            void executeSendPairDevicesFailed(const unordered_set<string>& uuids);
            void executeSendUnpairDevicesSucceeded(const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices);
            void executeSendUnpairDevicesFailed(const unordered_set<string>& uuids);
            void executeSetDeviceCategoriesSucceeded(const map<string, string>& uuidCategoryMap);
            void executeSetDeviceCategoriesFailed(const map<string, string>& uuidCategoryMap);
            void executeSendConnectByDeviceIdsSucceeded(const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices, Requester requester);
            void executeSendConnectByDeviceIdsFailed(const unordered_set<string>& uuids, Requester requester);
            void executeSendConnectByProfileSucceeded(shared_ptr<BluetoothDeviceInterface> device, const string& profileName, Requester requester);
            void executeSendConnectByProfileFailed(const string& profileName, Requester requester);
            void executeSendDisconnectDevicesSucceeded(const unordered_set<shared_ptr<BluetoothDeviceInterface>>& devices, Requester requester);
            void executeSendConnectFailedEvent(const string& eventName, const unordered_set<string>& uuids, Requester requester);
            void executeSendDisconnectDevicesFailed(const unordered_set<string>& uuids, Requester requester);
            void executeSendMediaControlEvent(const string& eventName, shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlPlaySucceeded(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlPlayFailed(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlStopSucceeded(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlStopFailed(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlNextSucceeded(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlNextFailed(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlPreviousSucceeded(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendMediaControlPreviousFailed(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendStreamingEvent(const string& eventName, shared_ptr<BluetoothDeviceInterface> device);
            void executeSendStreamingStarted(shared_ptr<BluetoothDeviceInterface> device);
            void executeSendStreamingEnded(shared_ptr<BluetoothDeviceInterface> device);
            ObserverInterface::DeviceAttributes generateDeviceAttributes(shared_ptr<BluetoothDeviceInterface> device);
            template <typename ServiceType> shared_ptr<ServiceType> getService(shared_ptr<BluetoothDeviceInterface> device);
            void executeInsertBluetoothEventState(shared_ptr<BluetoothDeviceInterface> device, DeviceState state, Optional<Requester> requester,
                                                  Optional<string> profileName);
            shared_ptr<BluetoothEventState> executeRemoveBluetoothEventState(shared_ptr<BluetoothDeviceInterface> device, DeviceState state);
            void executeRestrictA2DPDevices();
            void executeUnrestrictA2DPDevices();
            void executeAcquireFocus(const string& callingMethodName = "");
            void executeReleaseFocus(const string& callingMethodName = "");
            unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
            shared_ptr<MessageSenderInterface> m_messageSender;
            shared_ptr<ContextManagerInterface> m_contextManager;
            shared_ptr<FocusManagerInterface> m_focusManager;
            StreamingState m_streamingState;
            FocusTransitionState m_focusTransitionState;
            ScanningTransitionState m_scanningTransitionState;
            FocusState m_focusState;
            MediaPlayerObserverInterface::SourceId m_sourceId;
            shared_ptr<BluetoothDeviceManagerInterface> m_deviceManager;
            queue<pair<string, string>> m_eventQueue;
            shared_ptr<BluetoothDeviceInterface> m_activeA2DPDevice;
            shared_ptr<BluetoothDeviceInterface> m_disabledA2DPDevice;
            vector<shared_ptr<BluetoothDeviceInterface>> m_restrictedDevices;
            shared_ptr<MediaPlayerInterface> m_mediaPlayer;
            shared_ptr<BluetoothStorageInterface> m_db;
            shared_ptr<BluetoothEventBus> m_eventBus;
            shared_ptr<BluetoothMediaInputTransformer> m_mediaInputTransformer;
            shared_ptr<FormattedAudioStreamAdapter> m_mediaStream;
            shared_ptr<InProcessAttachment> m_mediaAttachment;
            shared_ptr<AttachmentWriter> m_mediaAttachmentWriter;
            shared_ptr<AttachmentReader> m_mediaAttachmentReader;
            unordered_set<shared_ptr<ObserverInterface>> m_observers;
            map<DeviceCategory, shared_ptr<BluetoothDeviceConnectionRuleInterface>> m_enabledConnectionRules;
            map<DeviceCategory, set<shared_ptr<BluetoothDeviceInterface>>> m_connectedDevices;
            map<string, unordered_set<shared_ptr<BluetoothEventState>>> m_bluetoothEventStates;
            shared_ptr<ChannelVolumeInterface> m_bluetoothChannelVolumeInterface;
            Executor m_executor;
        };
        inline string streamingStateToString(Bluetooth::StreamingState state) {
            switch(state) {
                case Bluetooth::StreamingState::INACTIVE: return "INACTIVE";
                case Bluetooth::StreamingState::PAUSED: return "PAUSED";
                case Bluetooth::StreamingState::PENDING_PAUSED: return "PENDING_PAUSED";
                case Bluetooth::StreamingState::PENDING_ACTIVE: return "PENDING_ACTIVE";
                case Bluetooth::StreamingState::ACTIVE: return "ACTIVE";
            }
            return "UNKNOWN";
        }
        inline ostream& operator<<(ostream& stream, const Bluetooth::StreamingState state) {
            return stream << streamingStateToString(state);
        }
        inline string focusTransitionStateToString(Bluetooth::FocusTransitionState state) {
            switch(state) {
                case Bluetooth::FocusTransitionState::INTERNAL: return "INTERNAL";
                case Bluetooth::FocusTransitionState::PENDING_INTERNAL: return "PENDING_INTERNAL";
                case Bluetooth::FocusTransitionState::EXTERNAL: return "EXTERNAL";
            }
            return "UNKNOWN";
        }
        inline ostream& operator<<(ostream& stream, const Bluetooth::FocusTransitionState state) {
            return stream << focusTransitionStateToString(state);
        }
        inline string scanningStateToString(Bluetooth::ScanningTransitionState state) {
            switch(state) {
                case Bluetooth::ScanningTransitionState::ACTIVE: return "ACTIVE";
                case Bluetooth::ScanningTransitionState::PENDING_INACTIVE: return "PENDING_ACTIVE";
                case Bluetooth::ScanningTransitionState::INACTIVE: return "INACTIVE";
            }
            return "UNKNWON";
        }
        inline ostream& operator<<(ostream& stream, const Bluetooth::ScanningTransitionState state) {
            return stream << scanningStateToString(state);
        }
    }
}
#endif
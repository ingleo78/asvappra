#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_DEFAULTCLIENT_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_DEFAULTCLIENT_H_

#include <acl/AVSConnectionManager.h>
#include <acl/Transport/MessageRouter.h>
#include <acl/Transport/MessageRouterFactory.h>
#include <adsl/DirectiveSequencer.h>
#include <afml/AudioActivityTracker.h>
#include <afml/FocusManager.h>
#include <afml/VisualActivityTracker.h>
#include <capability_agents/AIP/AudioInputProcessor.h>
#include <capability_agents/AIP/AudioProvider.h>
#include <acsdk_alerts/AlertsCapabilityAgent.h>
#include <acsdk_alerts/Renderer/Renderer.h>
#include <acsdk_alerts/Storage/AlertStorageInterface.h>
#include <capability_agents/Alexa/AlexaInterfaceCapabilityAgent.h>
#include <capability_agents/Alexa/AlexaInterfaceMessageSender.h>
#include <capability_agents/ApiGateway/ApiGatewayCapabilityAgent.h>
#include <capability/audio_player/AudioPlayer.h>
#include <capability/audio_player/AudioPlayerObserverInterface.h>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <avs/DialogUXStateAggregator.h>
#include <avs/ExceptionEncounteredSender.h>
#include <sdkinterfaces/AVSGatewayManagerInterface.h>
#include <sdkinterfaces/Audio/AudioFactoryInterface.h>
#include <sdkinterfaces/Audio/EqualizerConfigurationInterface.h>
#include <sdkinterfaces/Audio/EqualizerModeControllerInterface.h>
#include <sdkinterfaces/Audio/EqualizerStorageInterface.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceConnectionRuleInterface.h>
#include <sdkinterfaces/CallManagerInterface.h>
#include <sdkinterfaces/CapabilitiesDelegateInterface.h>
#include <sdkinterfaces/CapabilitiesObserverInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/Diagnostics/DiagnosticsInterface.h>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <sdkinterfaces/Endpoints/EndpointBuilderInterface.h>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <sdkinterfaces/Endpoints/EndpointInterface.h>
#include <sdkinterfaces/InternetConnectionMonitorInterface.h>
#include <sdkinterfaces/LocaleAssetsManagerInterface.h>
#include <sdkinterfaces/PowerResourceManagerInterface.h>
#include <sdkinterfaces/SingleSettingObserverInterface.h>
#include <sdkinterfaces/SpeechInteractionHandlerInterface.h>
#include <sdkinterfaces/Storage/MiscStorageInterface.h>
#include <sdkinterfaces/SystemTimeZoneInterface.h>
#include <sdkinterfaces/TemplateRuntimeObserverInterface.h>
#include <util/DeviceInfo.h>
#include <lib_curl_utils/HTTPContentFetcherFactory.h>
#include <media_player/MediaPlayerFactoryInterface.h>
#include <media_player/MediaPlayerInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/Optional.h>
#include <timing/SystemClockMonitor.h>
#include <capability/bluetooth/Bluetooth.h>
#include <capability/bluetooth/BluetoothStorageInterface.h>
#include <capability/notifications/NotificationRenderer.h>
#include <capability/notifications/NotificationsCapabilityAgent.h>
#include <captions/CaptionPresenterInterface.h>
#include <certified_sender/CertifiedSender.h>
#include <certified_sender/SQLiteMessageStorage.h>
#include <capability/do_not_disturb/DoNotDisturbCapabilityAgent.h>
#include <endpoints/EndpointBuilder.h>
#include <endpoints/EndpointRegistrationManager.h>
#include <capability_agents/Equalizer/EqualizerCapabilityAgent.h>
#include <equalizer_implementations/EqualizerController.h>
#include <capability_agents/ExternalMediaPlayer/ExternalMediaPlayer.h>
#include <capability_agents/InteractionModel/InteractionModelCapabilityAgent.h>
#include <captions/CaptionManager.h>
#ifdef ENABLE_PCC
#include <sdkinterfaces/Phone/PhoneCallerInterface.h>
#include "PhoneCallController/PhoneCallController.h"
#endif
#ifdef ENABLE_MCC
#include <sdkinterfaces/Calendar/CalendarClientInterface.h>
#include <sdkinterfaces/Meeting/MeetingClientInterface.h>
#include "MeetingClientController/MeetingClientController.h>
#endif
#include <sdkinterfaces/SystemSoundPlayerInterface.h>
#include <sdkinterfaces/Endpoints/EndpointInterface.h>
#include <endpoints/Endpoint.h>
#include <capability_agents/PlaybackController/PlaybackController.h>
#include <capability_agents/PlaybackController/PlaybackRouter.h>
#include <registration_manager/RegistrationManager.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Storage/DeviceSettingStorageInterface.h>
#include <capability_agents/SoftwareComponentReporter/SoftwareComponentReporterCapabilityAgent.h>
#include <capability_agents/SpeakerManager/DefaultChannelVolumeFactory.h>
#include <capability_agents/SpeakerManager/SpeakerManager.h>
#include <capability_agents/SpeechSynthesizer/SpeechSynthesizer.h>
#include <capability_agents/System/SoftwareInfoSender.h>
#include <capability_agents/System/UserInactivityMonitor.h>
#include <capability_agents/TemplateRuntime/TemplateRuntime.h>
#ifdef ENABLE_REVOKE_AUTH
#include <System/RevokeAuthorizationHandler.h>
#endif
#include "ConnectionRetryTrigger.h"
#include "EqualizerRuntimeSetup.h"
#include "ExternalCapabilitiesBuilderInterface.h"

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace chrono;
        using namespace acl;
        using namespace afml;
        using namespace avsCommon;
        using namespace alexaClientSDK::endpoints;
        using namespace equalizer;
        using namespace sdkInterfaces;
        using namespace avs;
        using namespace captions;
        using namespace certifiedSender;
        using namespace acsdkAlerts;
        using namespace acsdkAlertsInterfaces;
        using namespace acsdkAudioPlayer;
        using namespace acsdkAudioPlayerInterfaces;
        using namespace acsdkBluetooth;
        using namespace acsdkBluetoothInterfaces;
        using namespace acsdkNotifications;
        using namespace acsdkNotificationsInterfaces;
        using namespace acsdkAlerts::storage;
        using namespace sdkInterfaces::audio;
        using namespace sdkInterfaces::bluetooth;
        using namespace sdkInterfaces::diagnostics;
        using namespace sdkInterfaces::endpoints;
        using namespace sdkInterfaces::softwareInfo;
        using namespace sdkInterfaces::storage;
        using namespace settings;
        using namespace settings::storage;
        using namespace acsdkAlerts::storage;
        using namespace audio;
        using namespace externalMediaPlayer;
        using namespace capabilityAgents;
        using namespace capabilityAgents::alexa;
        using namespace capabilityAgents::apiGateway;
        using namespace capabilityAgents::doNotDisturb;
        using namespace capabilityAgents::equalizer;
        using namespace capabilityAgents::externalMediaPlayer;
        using namespace capabilityAgents::interactionModel;
        using namespace capabilityAgents::playbackController;
        using namespace capabilityAgents::softwareComponentReporter;
        using namespace capabilityAgents::system;
        using namespace capabilityAgents::templateRuntime;
        using namespace aip;
        using namespace speakerManager;
        using namespace speakerConstants;
        using namespace speechencoder;
        using namespace speechSynthesizer;
        using namespace utils;
        using namespace metrics;
        using namespace mediaPlayer;
        using namespace registrationManager;
        class DefaultClient : public SpeechInteractionHandlerInterface {
        public:
            static unique_ptr<DefaultClient> create(shared_ptr<DeviceInfo> deviceInfo, shared_ptr<CustomerDataManager> customerDataManager,
                                                    const unordered_map<string, shared_ptr<MediaPlayerInterface>>& externalMusicProviderMediaPlayers,
                                                    const unordered_map<string, shared_ptr<SpeakerInterface>>& externalMusicProviderSpeakers,
                                                    const ExternalMediaPlayer::AdapterCreationMap& adapterCreationMap,
                                                    shared_ptr<MediaPlayerInterface> speakMediaPlayer, unique_ptr<MediaPlayerFactoryInterface> audioMediaPlayerFactory,
                                                    shared_ptr<MediaPlayerInterface> alertsMediaPlayer, shared_ptr<MediaPlayerInterface> notificationsMediaPlayer,
                                                    shared_ptr<MediaPlayerInterface> bluetoothMediaPlayer, shared_ptr<MediaPlayerInterface> ringtoneMediaPlayer,
                                                    shared_ptr<MediaPlayerInterface> systemSoundMediaPlayer, shared_ptr<SpeakerInterface> speakSpeaker,
                                                    vector<shared_ptr<SpeakerInterface>> audioSpeakers, shared_ptr<SpeakerInterface> alertsSpeaker,
                                                    shared_ptr<SpeakerInterface> notificationsSpeaker, shared_ptr<SpeakerInterface> bluetoothSpeaker,
                                                    shared_ptr<SpeakerInterface> ringtoneSpeaker, shared_ptr<SpeakerInterface> systemSoundSpeaker,
                                                    const multimap<ChannelVolumeInterface::Type, shared_ptr<SpeakerInterface>> additionalSpeakers,
                                                #ifdef ENABLE_PCC
                                                    shared_ptr<SpeakerInterface> phoneSpeaker, shared_ptr<phone::PhoneCallerInterface> phoneCaller,
                                                #endif
                                                #ifdef ENABLE_MCC
                                                    shared_ptr<SpeakerInterface> meetingSpeaker, shared_ptr<meeting::MeetingClientInterface> meetingClient,
                                                    shared_ptr<calendar::CalendarClientInterface> calendarClient,
                                                #endif
                                                #ifdef ENABLE_COMMS_AUDIO_PROXY
                                                    shared_ptr<MediaPlayerInterface> commsMediaPlayer, shared_ptr<SpeakerInterface> commsSpeaker,
                                                    shared_ptr<AudioInputStream> sharedDataStream,
                                                #endif
                                                    shared_ptr<EqualizerRuntimeSetup> equalizerRuntimeSetup, shared_ptr<AudioFactoryInterface> audioFactory,
                                                    shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AlertStorageInterface> alertStorage,
                                                    shared_ptr<MessageStorageInterface> messageStorage, shared_ptr<NotificationsStorageInterface> notificationsStorage,
                                                    unique_ptr<DeviceSettingStorageInterface> deviceSettingStorage, shared_ptr<BluetoothStorageInterface> bluetoothStorage,
                                                    shared_ptr<MiscStorageInterface> miscStorage, unordered_set<shared_ptr<DialogUXStateObserverInterface>> alexaDialogStateObservers,
                                                    unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionObservers, shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor,
                                                    bool isGuiSupported, shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate, shared_ptr<ContextManagerInterface> contextManager,
                                                    shared_ptr<TransportFactoryInterface> transportFactory, shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                                                    shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager,
                                                    unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules = unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>>(),
                                                    shared_ptr<SystemTimeZoneInterface> systemTimezone = nullptr, FirmwareVersion firmwareVersion = INVALID_FIRMWARE_VERSION,
                                                    bool sendSoftwareInfoOnConnected = false, shared_ptr<SoftwareInfoSenderObserverInterface> softwareInfoSenderObserver = nullptr,
                                                    unique_ptr<BluetoothDeviceManagerInterface> bluetoothDeviceManager = nullptr, shared_ptr<MetricRecorderInterface> metricRecorder = nullptr,
                                                    shared_ptr<PowerResourceManagerInterface> powerResourceManager = nullptr, shared_ptr<DiagnosticsInterface> diagnostics = nullptr,
                                                    const shared_ptr<ExternalCapabilitiesBuilderInterface>& externalCapabilitiesBuilder = nullptr,
                                                    shared_ptr<ChannelVolumeFactoryInterface> channelVolumeFactory = make_shared<DefaultChannelVolumeFactory>(),
                                                    bool startAlertSchedulingOnInitialization = true,
                                                    shared_ptr<MessageRouterFactoryInterface> messageRouterFactory = make_shared<MessageRouterFactory>());
            void connect(bool performReset = true);
            void disconnect();
            string getAVSGateway();
            void stopForegroundActivity();
            void stopAllActivities();
            void localStopActiveAlert();
            void addAlexaDialogStateObserver(shared_ptr<DialogUXStateObserverInterface> observer);
            void removeAlexaDialogStateObserver(shared_ptr<DialogUXStateObserverInterface> observer);
            void addMessageObserver(shared_ptr<MessageObserverInterface> observer);
            void removeMessageObserver(shared_ptr<MessageObserverInterface> observer);
            void addConnectionObserver(shared_ptr<ConnectionStatusObserverInterface> observer);
            void removeConnectionObserver(shared_ptr<ConnectionStatusObserverInterface> observer);
            void addInternetConnectionObserver(shared_ptr<InternetConnectionObserverInterface> observer);
            void removeInternetConnectionObserver(shared_ptr<InternetConnectionObserverInterface> observer);
            void addAlertsObserver(shared_ptr<AlertObserverInterface> observer);
            void removeAlertsObserver(shared_ptr<AlertObserverInterface> observer);
            void addAudioPlayerObserver(shared_ptr<AudioPlayerObserverInterface> observer);
            void removeAudioPlayerObserver(shared_ptr<AudioPlayerObserverInterface> observer);
            void addTemplateRuntimeObserver(shared_ptr<TemplateRuntimeObserverInterface> observer);
            void removeTemplateRuntimeObserver(shared_ptr<TemplateRuntimeObserverInterface> observer);
            void TemplateRuntimeDisplayCardCleared();
            void addNotificationsObserver(shared_ptr<NotificationsObserverInterface> observer);
            void removeNotificationsObserver(shared_ptr<NotificationsObserverInterface> observer);
            void addExternalMediaPlayerObserver(shared_ptr<ExternalMediaPlayerObserverInterface> observer);
            void removeExternalMediaPlayerObserver(shared_ptr<ExternalMediaPlayerObserverInterface> observer);
            void addBluetoothDeviceObserver(shared_ptr<BluetoothDeviceObserverInterface> observer);
            void removeBluetoothDeviceObserver(shared_ptr<BluetoothDeviceObserverInterface> observer);
            void addCaptionPresenter(shared_ptr<CaptionPresenterInterface> presenter);
            void setCaptionMediaPlayers(const vector<shared_ptr<MediaPlayerInterface>>& mediaPlayers);
            shared_ptr<PlaybackRouterInterface> getPlaybackRouter() const;
            void addSpeakerManagerObserver(shared_ptr<SpeakerManagerObserverInterface> observer);
            void removeSpeakerManagerObserver(shared_ptr<SpeakerManagerObserverInterface> observer);
            shared_ptr<SpeakerManagerInterface> getSpeakerManager();
            void addSpeechSynthesizerObserver(shared_ptr<SpeechSynthesizerObserverInterface> observer);
            void removeSpeechSynthesizerObserver(shared_ptr<SpeechSynthesizerObserverInterface> observer);
            shared_ptr<RegistrationManager> getRegistrationManager();
        #ifdef ENABLE_REVOKE_AUTH
            void addRevokeAuthorizationObserver(shared_ptr<RevokeAuthorizationObserverInterface> observer);
            void removeRevokeAuthorizationObserver(shared_ptr<RevokeAuthorizationObserverInterface> observer);
        #endif
            std::shared_ptr<EqualizerController> getEqualizerController();
            void addEqualizerControllerListener(shared_ptr<EqualizerControllerListenerInterface> listener);
            void removeEqualizerControllerListener(shared_ptr<EqualizerControllerListenerInterface> listener);
            void addContextManagerObserver(shared_ptr<ContextManagerObserverInterface> observer);
            void removeContextManagerObserver(shared_ptr<ContextManagerObserverInterface> observer);
            bool setFirmwareVersion(FirmwareVersion firmwareVersion);
            future<bool> notifyOfWakeWord(AudioProvider wakeWordAudioProvider, AudioInputStream::Index beginIndex, AudioInputStream::Index endIndex,
                                          string keyword, steady_clock::time_point startOfSpeechTimestamp, shared_ptr<const vector<char>> KWDMetadata = nullptr) override;
            future<bool> notifyOfTapToTalk(AudioProvider tapToTalkAudioProvider, AudioInputStream::Index beginIndex = AudioInputProcessor::INVALID_INDEX,
                                           steady_clock::time_point startOfSpeechTimestamp = steady_clock::now()) override;
            future<bool> notifyOfHoldToTalkStart(AudioProvider holdToTalkAudioProvider, steady_clock::time_point startOfSpeechTimestamp = steady_clock::now()) override;
            future<bool> notifyOfHoldToTalkEnd() override;
            future<bool> notifyOfTapToTalkEnd() override;
            shared_ptr<settings::DeviceSettingsManager> getSettingsManager();
            unique_ptr<avsCommon::sdkInterfaces::endpoints::EndpointBuilderInterface> createEndpointBuilder();
            future<EndpointRegistrationManager::RegistrationResult> registerEndpoint(shared_ptr<EndpointInterface> endpoint);
            future<EndpointRegistrationManager::DeregistrationResult> deregisterEndpoint(EndpointIdentifier endpointId);
            shared_ptr<EndpointBuilderInterface> getDefaultEndpointBuilder();
            void addCallStateObserver(shared_ptr<CallStateObserverInterface> observer);
            void removeCallStateObserver(shared_ptr<CallStateObserverInterface> observer);
            bool isCommsEnabled();
            void acceptCommsCall();
            void sendDtmf(CallManagerInterface::DTMFTone dtmfTone);
            void stopCommsCall();
            void audioPlayerLocalStop();
            bool isCommsCallMuted();
            void muteCommsCall();
            void unmuteCommsCall();
            void onSystemClockSynchronized();
            ~DefaultClient();
        private:
            DefaultClient(const avsCommon::utils::DeviceInfo& deviceInfo);
            bool initialize(shared_ptr<CustomerDataManager> customerDataManager, const unordered_map<string, shared_ptr<MediaPlayerInterface>>& externalMusicProviderMediaPlayers,
                            const unordered_map<string, shared_ptr<SpeakerInterface>>& externalMusicProviderSpeakers, const ExternalMediaPlayer::AdapterCreationMap& adapterCreationMap,
                            shared_ptr<MediaPlayerInterface> speakMediaPlayer, unique_ptr<MediaPlayerFactoryInterface> audioMediaPlayerFactory,
                            shared_ptr<MediaPlayerInterface> alertsMediaPlayer, shared_ptr<MediaPlayerInterface> notificationsMediaPlayer,
                            shared_ptr<MediaPlayerInterface> bluetoothMediaPlayer, shared_ptr<MediaPlayerInterface> ringtoneMediaPlayer,
                            shared_ptr<MediaPlayerInterface> systemSoundMediaPlayer, shared_ptr<SpeakerInterface> speakSpeaker, vector<shared_ptr<SpeakerInterface>> audioSpeakers,
                            shared_ptr<SpeakerInterface> alertsSpeaker, shared_ptr<SpeakerInterface> notificationsSpeaker, shared_ptr<SpeakerInterface> bluetoothSpeaker,
                            shared_ptr<SpeakerInterface> ringtoneSpeaker, shared_ptr<SpeakerInterface> systemSoundSpeaker,
                            const multimap<ChannelVolumeInterface::Type, shared_ptr<SpeakerInterface>> additionalSpeakers,
                        #ifdef ENABLE_PCC
                            shared_ptr<SpeakerInterface> phoneSpeaker, shared_ptr<PhoneCallerInterface> phoneCaller,
                        #endif
                        #ifdef ENABLE_MCC
                            shared_ptr<SpeakerInterface> meetingSpeaker, shared_ptr<MeetingClientInterface> meetingClient, shared_ptr<CalendarClientInterface> calendarClient,
                        #endif
                        #ifdef ENABLE_COMMS_AUDIO_PROXY
                            shared_ptrMediaPlayerInterface> commsMediaPlayer, shared_ptr<SpeakerInterface> commsSpeaker, shared_ptr<AudioInputStream> sharedDataStream,
                        #endif
                            shared_ptr<EqualizerRuntimeSetup> equalizerRuntimeSetup, shared_ptr<AudioFactoryInterface> audioFactory,
                            shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AlertStorageInterface> alertStorage,
                            shared_ptr<MessageStorageInterface> messageStorage, shared_ptr<NotificationsStorageInterface> notificationsStorage,
                            shared_ptr<DeviceSettingStorageInterface> deviceSettingStorage, shared_ptr<BluetoothStorageInterface> bluetoothStorage,
                            shared_ptr<MiscStorageInterface> miscStorage, unordered_set<shared_ptr<DialogUXStateObserverInterface>> alexaDialogStateObservers,
                            unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionObservers, shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor,
                            bool isGuiSupported, shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate, shared_ptr<ContextManagerInterface> contextManager,
                            shared_ptr<TransportFactoryInterface> transportFactory, shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                            shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager, unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules,
                            shared_ptr<SystemTimeZoneInterface> systemTimezone, FirmwareVersion firmwareVersion, bool sendSoftwareInfoOnConnected,
                            shared_ptr<SoftwareInfoSenderObserverInterface> softwareInfoSenderObserver, unique_ptr<BluetoothDeviceManagerInterface> bluetoothDeviceManager,
                            shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<PowerResourceManagerInterface> powerResourceManager,
                            shared_ptr<DiagnosticsInterface> diagnostics, const shared_ptr<ExternalCapabilitiesBuilderInterface>& externalCapabilitiesBuilder,
                            shared_ptr<ChannelVolumeFactoryInterface> channelVolumeFactory, bool startAlertSchedulingOnInitialization,
                            shared_ptr<MessageRouterFactoryInterface> messageRouterFactory);
            shared_ptr<DirectiveSequencerInterface> m_directiveSequencer;
            shared_ptr<FocusManager> m_audioFocusManager;
            shared_ptr<FocusManager> m_visualFocusManager;
            shared_ptr<AudioActivityTracker> m_audioActivityTracker;
            shared_ptr<VisualActivityTracker> m_visualActivityTracker;
            shared_ptr<MessageRouterInterface> m_messageRouter;
            shared_ptr<AVSConnectionManager> m_connectionManager;
            shared_ptr<InternetConnectionMonitorInterface> m_internetConnectionMonitor;
        #ifdef ENABLE_CAPTIONS
            shared_ptr<CaptionManager> m_captionManager;
        #endif
            shared_ptr<ExceptionEncounteredSender> m_exceptionSender;
            shared_ptr<CertifiedSender> m_certifiedSender;
            shared_ptr<AudioInputProcessor> m_audioInputProcessor;
            shared_ptr<SpeechSynthesizer> m_speechSynthesizer;
            shared_ptr<AudioPlayer> m_audioPlayer;
            shared_ptr<ExternalMediaPlayer> m_externalMediaPlayer;
            shared_ptr<AlexaInterfaceMessageSender> m_alexaMessageSender;
            shared_ptr<AlexaInterfaceCapabilityAgent> m_alexaCapabilityAgent;
            shared_ptr<ApiGatewayCapabilityAgent> m_apiGatewayCapabilityAgent;
            shared_ptr<AlertsCapabilityAgent> m_alertsCapabilityAgent;
            shared_ptr<Bluetooth> m_bluetooth;
            shared_ptr<InteractionModelCapabilityAgent> m_interactionCapabilityAgent;
            shared_ptr<NotificationRenderer> m_notificationsRenderer;
            shared_ptr<NotificationsCapabilityAgent> m_notificationsCapabilityAgent;
            shared_ptr<UserInactivityMonitor> m_userInactivityMonitor;
        #ifdef ENABLE_PCC
            std::shared_ptr<PhoneCallController> m_phoneCallControllerCapabilityAgent;
        #endif
        #ifdef ENABLE_MCC
            std::shared_ptr<MeetingClientController> m_meetingClientControllerCapabilityAgent;
        #endif
            shared_ptr<CallManagerInterface> m_callManager;
            shared_ptr<DialogUXStateAggregator> m_dialogUXStateAggregator;
            shared_ptr<PlaybackRouter> m_playbackRouter;
            shared_ptr<PlaybackController> m_playbackController;
            shared_ptr<SpeakerManager> m_speakerManager;
            shared_ptr<TemplateRuntime> m_templateRuntime;
            shared_ptr<DoNotDisturbCapabilityAgent> m_dndCapabilityAgent;
            shared_ptr<EqualizerCapabilityAgent> m_equalizerCapabilityAgent;
            shared_ptr<EqualizerController> m_equalizerController;
            shared_ptr<EqualizerRuntimeSetup> m_equalizerRuntimeSetup;
            mutex m_softwareInfoSenderMutex;
            shared_ptr<SoftwareInfoSender> m_softwareInfoSender;
        #ifdef ENABLE_REVOKE_AUTH
            shared_ptr<RevokeAuthorizationHandler> m_revokeAuthorizationHandler;
        #endif
            shared_ptr<RegistrationManager> m_registrationManager;
            shared_ptr<SystemSoundPlayerInterface> m_systemSoundPlayer;
            shared_ptr<DeviceSettingsManager> m_deviceSettingsManager;
            shared_ptr<DeviceSettingStorageInterface> m_deviceSettingStorage;
            DeviceInfo m_deviceInfo;
            shared_ptr<ContextManagerInterface> m_contextManager;
            shared_ptr<EndpointRegistrationManager> m_endpointRegistrationManager;
            shared_ptr<EndpointBuilder> m_defaultEndpointBuilder;
            shared_ptr<AVSGatewayManagerInterface> m_avsGatewayManager;
            shared_ptr<CapabilitiesDelegateInterface> m_capabilitiesDelegate;
            shared_ptr<DiagnosticsInterface> m_diagnostics;
            shared_ptr<SystemClockMonitor> m_systemClockMonitor;
            list<shared_ptr<RequiresShutdown>> m_shutdownObjects;
            shared_ptr<ConnectionRetryTrigger> m_connectionRetryTrigger;
            unordered_set<shared_ptr<SoftwareInfoSenderObserverInterface>> m_softwareInfoSenderObservers;
            shared_ptr<SoftwareComponentReporterCapabilityAgent> m_softwareReporterCapabilityAgent;
        };
    }
}
#endif
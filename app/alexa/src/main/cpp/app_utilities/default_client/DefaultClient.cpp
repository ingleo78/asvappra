#include <adsl/MessageInterpreter.h>
#include <avs/attachment/AttachmentManager.h>
#include <avs/ExceptionEncounteredSender.h>
#include <sdkinterfaces/InternetConnectionMonitorInterface.h>
#include <sdkinterfaces/SystemClockMonitorObserverInterface.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/network/InternetConnectionMonitor.h>
#include <app_utilities/resource/audio/SystemSoundAudioFactory.h>
#include <app_utilities/system_sound_player/SystemSoundPlayer.h>
#include <captions/LibwebvttParserAdapter.h>
#include <captions/TimingAdapterFactory.h>
#include <speech_enconder/OpusEncoderContext.h>
#ifdef ENABLE_PCC
#include <sdkinterfaces/Phone/PhoneCallerInterface.h>
#include <PhoneCallController/PhoneCallController.h>
#endif
#ifdef ENABLE_MCC
#include <sdkinterfaces/Calendar/CalendarClientInterface.h>
#include <sdkinterfaces/Meeting/MeetingClientInterface.h>
#include <MeetingClientController/MeetingClientController.h>
#endif
#include <avs/SpeakerConstants/SpeakerConstants.h>
#include <interrupt_model/InterruptModel.h>
#include <capability_agents/System/LocaleHandler.h>
#include <capability_agents/System/ReportStateHandler.h>
#include <capability_agents/System/SystemCapabilityProvider.h>
#include <capability_agents/System/TimeZoneHandler.h>
#include <capability_agents/System/UserInactivityMonitor.h>
#include <bluetooth_implementations/BlueZBluetoothDeviceManager.h>
#include <capability/bluetooth/BluetoothMediaInputTransformer.h>
#include <app_utilities/sdkcomponent/SDKComponent.h>
#include "DefaultClient.h"
#include "DeviceSettingsManagerBuilder.h"

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace chrono;
        using namespace acl;
        using namespace adsl;
        using namespace afml;
        using namespace acsdkAudioPlayerInterfaces;
        using namespace avsCommon;
        using namespace avs;
        using namespace acsdkAlerts;
        using namespace acsdkBluetooth;
        using namespace acsdkNotificationsInterfaces;
        using namespace applicationUtilities;
        using namespace capabilityAgents;
        using namespace certifiedSender;
        using namespace registrationManager;
        using namespace sdkInterfaces;
        using namespace settings;
        using namespace speechencoder;
        using namespace speechSynthesizer;
        using namespace utils;
        using namespace utils::configuration;
        using namespace utils::logger;
        using namespace utils::mediaPlayer;
        using namespace utils::metrics;
        using namespace utils::timing;
        using namespace acsdkAlerts::renderer;
        using namespace acsdkAlerts::storage;
        using namespace afml::interruptModel;
        using namespace applicationUtilities::systemSoundPlayer;
        using namespace avs::attachment;
        using namespace capabilityAgents::aip;
        using namespace capabilityAgents::alexa;
        using namespace capabilityAgents::apiGateway;
        using namespace capabilityAgents::doNotDisturb;
        using namespace capabilityAgents::equalizer;
        using namespace capabilityAgents::externalMediaPlayer;
        using namespace capabilityAgents::playbackController;
        using namespace capabilityAgents::softwareComponentReporter;
        using namespace capabilityAgents::system;
        using namespace capabilityAgents::templateRuntime;
        using namespace sdkInterfaces::audio;
        using namespace sdkInterfaces::bluetooth;
        using namespace sdkInterfaces::diagnostics;
        using namespace sdkInterfaces::endpoints;
        using namespace sdkInterfaces::softwareInfo;
        using namespace sdkInterfaces::storage;
        using namespace settings::storage;
        using namespace alexaClientSDK::endpoints;
        static const string AUDIO_CHANNEL_CONFIG_KEY = "audioChannels";
        static const string VISUAL_CHANNEL_CONFIG_KEY = "visualChannels";
        static const string INTERRUPT_MODEL_CONFIG_KEY = "interruptModel";
        static const string TAG("DefaultClient");
        #define LX(event) LogEntry(TAG, event)
        unique_ptr<DefaultClient> DefaultClient::create(shared_ptr<DeviceInfo> deviceInfo, shared_ptr<CustomerDataManager> customerDataManager,
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
                                                        shared_ptr<SpeakerInterface> phoneSpeaker, shared_ptr<PhoneCallerInterface> phoneCaller,
                                                    #endif
                                                    #ifdef ENABLE_MCC
                                                        shared_ptr<SpeakerInterface> meetingSpeaker, shared_ptr<MeetingClientInterface> meetingClient,
                                                        shared_ptr<CalendarClientInterface> calendarClient,
                                                    #endif
                                                    #ifdef ENABLE_COMMS_AUDIO_PROXY
                                                        shared_ptr<MediaPlayerInterface> commsMediaPlayer, shared_ptr<SpeakerInterface> commsSpeaker,
                                                        shared_ptr<AudioInputStream> sharedDataStream,
                                                    #endif
                                                        shared_ptr<EqualizerRuntimeSetup> equalizerRuntimeSetup, shared_ptr<AudioFactoryInterface> audioFactory,
                                                        shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AlertStorageInterface> alertStorage,
                                                        shared_ptr<MessageStorageInterface> messageStorage,
                                                        shared_ptr<NotificationsStorageInterface> notificationsStorage,
                                                        unique_ptr<DeviceSettingStorageInterface> deviceSettingStorage,
                                                        shared_ptr<BluetoothStorageInterface> bluetoothStorage, shared_ptr<MiscStorageInterface> miscStorage,
                                                        unordered_set<shared_ptr<DialogUXStateObserverInterface>> alexaDialogStateObservers,
                                                        unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionObservers,
                                                        shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor, bool isGuiSupported,
                                                        shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                                        shared_ptr<ContextManagerInterface> contextManager,
                                                        shared_ptr<TransportFactoryInterface> transportFactory,
                                                        shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                                                        shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager,
                                                        unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules,
                                                        shared_ptr<SystemTimeZoneInterface> systemTimezone, FirmwareVersion firmwareVersion,
                                                        bool sendSoftwareInfoOnConnected, shared_ptr<SoftwareInfoSenderObserverInterface> softwareInfoSenderObserver,
                                                        unique_ptr<BluetoothDeviceManagerInterface> bluetoothDeviceManager,
                                                        shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<PowerResourceManagerInterface> powerResourceManager,
                                                        shared_ptr<DiagnosticsInterface> diagnostics, const shared_ptr<ExternalCapabilitiesBuilderInterface>& externalCapabilitiesBuilder,
                                                        shared_ptr<ChannelVolumeFactoryInterface> channelVolumeFactory, bool startAlertSchedulingOnInitialization,
                                                        shared_ptr<MessageRouterFactoryInterface> messageRouterFactory) {
            if (!deviceInfo) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullDeviceInfo"));
                return nullptr;
            }
            unique_ptr<DefaultClient> defaultClient(new DefaultClient(*deviceInfo));
            if (!defaultClient->initialize(customerDataManager, externalMusicProviderMediaPlayers, externalMusicProviderSpeakers, adapterCreationMap,
                speakMediaPlayer, move(audioMediaPlayerFactory), alertsMediaPlayer, notificationsMediaPlayer, bluetoothMediaPlayer, ringtoneMediaPlayer,
                systemSoundMediaPlayer, speakSpeaker, audioSpeakers, alertsSpeaker, notificationsSpeaker, bluetoothSpeaker, ringtoneSpeaker, systemSoundSpeaker,
                additionalSpeakers,
            #ifdef ENABLE_PCC
                phoneSpeaker, phoneCaller,
            #endif
            #ifdef ENABLE_MCC
                meetingSpeaker, meetingClient, calendarClient,
            #endif
            #ifdef ENABLE_COMMS_AUDIO_PROXY
                commsMediaPlayer, commsSpeaker, sharedDataStream,
            #endif
                equalizerRuntimeSetup, audioFactory, authDelegate, alertStorage, messageStorage, notificationsStorage,move(deviceSettingStorage),
                bluetoothStorage, miscStorage, alexaDialogStateObservers, connectionObservers, internetConnectionMonitor, isGuiSupported, capabilitiesDelegate,
                contextManager, transportFactory, avsGatewayManager, localeAssetsManager, enabledConnectionRules, systemTimezone, firmwareVersion,
                sendSoftwareInfoOnConnected, softwareInfoSenderObserver, move(bluetoothDeviceManager), move(metricRecorder), powerResourceManager, diagnostics,
                externalCapabilitiesBuilder, channelVolumeFactory, startAlertSchedulingOnInitialization, messageRouterFactory)) {
                return nullptr;
            }
            return defaultClient;
        }
        DefaultClient::DefaultClient(const DeviceInfo& deviceInfo) : m_deviceInfo{deviceInfo} {}
        bool DefaultClient::initialize(shared_ptr<CustomerDataManager> customerDataManager, const unordered_map<string, shared_ptr<MediaPlayerInterface>>& externalMusicProviderMediaPlayers,
                                       const unordered_map<string, shared_ptr<SpeakerInterface>>& externalMusicProviderSpeakers,
                                       const capabilityAgents::externalMediaPlayer::ExternalMediaPlayer::AdapterCreationMap& adapterCreationMap,
                                       shared_ptr<MediaPlayerInterface> speakMediaPlayer, unique_ptr<MediaPlayerFactoryInterface> audioMediaPlayerFactory,
                                       shared_ptr<MediaPlayerInterface> alertsMediaPlayer, shared_ptr<MediaPlayerInterface> notificationsMediaPlayer,
                                       shared_ptr<MediaPlayerInterface> bluetoothMediaPlayer, shared_ptr<MediaPlayerInterface> ringtoneMediaPlayer,
                                       shared_ptr<MediaPlayerInterface> systemSoundMediaPlayer, shared_ptr<SpeakerInterface> speakSpeaker,
                                       vector<shared_ptr<SpeakerInterface>> audioSpeakers, shared_ptr<SpeakerInterface> alertsSpeaker,
                                       shared_ptr<SpeakerInterface> notificationsSpeaker, shared_ptr<SpeakerInterface> bluetoothSpeaker,
                                       shared_ptr<SpeakerInterface> ringtoneSpeaker, shared_ptr<SpeakerInterface> systemSoundSpeaker,
                                       const multimap<ChannelVolumeInterface::Type, shared_ptr<SpeakerInterface>> additionalSpeakers,
                                   #ifdef ENABLE_PCC
                                       shared_ptr<SpeakerInterface> phoneSpeaker, shared_ptr<PhoneCallerInterface> phoneCaller,
                                   #endif
                                   #ifdef ENABLE_MCC
                                       shared_ptr<SpeakerInterface> meetingSpeaker, shared_ptr<MeetingClientInterface> meetingClient,
                                       shared_ptr<CalendarClientInterface> calendarClient,
                                   #endif
                                   #ifdef ENABLE_COMMS_AUDIO_PROXY
                                       shared_ptr<MediaPlayerInterface> commsMediaPlayer, shared_ptr<SpeakerInterface> commsSpeaker,
                                       shared_ptr<AudioInputStream> sharedDataStream,
                                   #endif
                                       shared_ptr<EqualizerRuntimeSetup> equalizerRuntimeSetup, shared_ptr<AudioFactoryInterface> audioFactory,
                                       shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AlertStorageInterface> alertStorage,
                                       shared_ptr<MessageStorageInterface> messageStorage, shared_ptr<NotificationsStorageInterface> notificationsStorage,
                                       shared_ptr<DeviceSettingStorageInterface> deviceSettingStorage, shared_ptr<BluetoothStorageInterface> bluetoothStorage,
                                       shared_ptr<MiscStorageInterface> miscStorage, unordered_set<shared_ptr<DialogUXStateObserverInterface>> alexaDialogStateObservers,
                                       unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionObservers,
                                       shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor,
                                       bool isGuiSupported, shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                       shared_ptr<ContextManagerInterface> contextManager, shared_ptr<TransportFactoryInterface> transportFactory,
                                       shared_ptr<AVSGatewayManagerInterface> avsGatewayManager, shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager,
                                       unordered_set<shared_ptr<BluetoothDeviceConnectionRuleInterface>> enabledConnectionRules,
                                       shared_ptr<SystemTimeZoneInterface> systemTimezone, FirmwareVersion firmwareVersion, bool sendSoftwareInfoOnConnected,
                                       shared_ptr<SoftwareInfoSenderObserverInterface> softwareInfoSenderObserver,
                                       unique_ptr<BluetoothDeviceManagerInterface> bluetoothDeviceManager, shared_ptr<MetricRecorderInterface> metricRecorder,
                                       shared_ptr<PowerResourceManagerInterface> powerResourceManager, shared_ptr<DiagnosticsInterface> diagnostics,
                                       const shared_ptr<ExternalCapabilitiesBuilderInterface>& externalCapabilitiesBuilder,
                                       shared_ptr<ChannelVolumeFactoryInterface> channelVolumeFactory, bool startAlertSchedulingOnInitialization,
                                       shared_ptr<MessageRouterFactoryInterface> messageRouterFactory) {
            if (!audioFactory) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAudioFactory"));
                return false;
            }
            if (!speakMediaPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullSpeakMediaPlayer"));
                return false;
            }
            if (!audioMediaPlayerFactory) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAudioMediaPlayerFactory"));
                return false;
            }
            if (!alertsMediaPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAlertsMediaPlayer"));
                return false;
            }
            if (!notificationsMediaPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullNotificationsMediaPlayer"));
                return false;
            }
            if (!bluetoothMediaPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullBluetoothMediaPlayer"));
                return false;
            }
            if (!ringtoneMediaPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullRingtoneMediaPlayer"));
                return false;
            }
            if (!systemSoundMediaPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullSystemSoundMediaPlayer"));
                return false;
            }
            if (!authDelegate) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAuthDelegate"));
                return false;
            }
            if (!capabilitiesDelegate) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullCapabilitiesDelegate"));
                return false;
            }
            if (!deviceSettingStorage) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullDeviceSettingStorage"));
                return false;
            }
            if (!contextManager) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullContextManager"));
                return false;
            }
            if (!transportFactory) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullTransportFactory"));
                return false;
            }
            if (!avsGatewayManager) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAVSGatewayManager"));
                return false;
            }
            if (!channelVolumeFactory) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullChannelVolumeFactory"));
                return false;
            }
            if (!messageRouterFactory) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullMessageRouterFactory"));
            }
            m_softwareReporterCapabilityAgent = SoftwareComponentReporterCapabilityAgent::create();
            if (!m_softwareReporterCapabilityAgent) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "nullSoftwareReporterCapabilityAgent"));
                return false;
            }
            if (!SDKComponent::SDKComponent::registerComponent(m_softwareReporterCapabilityAgent)) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToRegisterSDKComponent"));
                return false;
            }
            m_avsGatewayManager = avsGatewayManager;
            m_dialogUXStateAggregator = make_shared<DialogUXStateAggregator>(metricRecorder);
            for (auto observer : alexaDialogStateObservers) m_dialogUXStateAggregator->addObserver(observer);
            auto attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
            m_messageRouter = messageRouterFactory->createMessageRouter(authDelegate, attachmentManager, transportFactory);
            if (!internetConnectionMonitor) {
                ACSDK_CRITICAL(LX("initializeFailed").d("reason", "internetConnectionMonitor was nullptr"));
                return false;
            }
            m_internetConnectionMonitor = internetConnectionMonitor;
            m_connectionManager = AVSConnectionManager::create(m_messageRouter, false, connectionObservers, {m_dialogUXStateAggregator}, internetConnectionMonitor);
            if (!m_connectionManager) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateConnectionManager"));
                return false;
            }
            m_certifiedSender = CertifiedSender::create(m_connectionManager, m_connectionManager, messageStorage, customerDataManager);
            if (!m_certifiedSender) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateCertifiedSender"));
                return false;
            }
            m_exceptionSender = ExceptionEncounteredSender::create(m_connectionManager);
            if (!m_exceptionSender) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateExceptionSender"));
                return false;
            }
            m_directiveSequencer = DirectiveSequencer::create(m_exceptionSender, metricRecorder);
            if (!m_directiveSequencer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateDirectiveSequencer"));
                return false;
            }
            auto messageInterpreter = make_shared<MessageInterpreter>(m_exceptionSender, m_directiveSequencer, attachmentManager, metricRecorder);
            m_connectionManager->addMessageObserver(messageInterpreter);
            m_registrationManager = make_shared<RegistrationManager>(m_directiveSequencer, m_connectionManager, customerDataManager);
            m_contextManager = contextManager;
            m_capabilitiesDelegate = move(capabilitiesDelegate);
            m_capabilitiesDelegate->setMessageSender(m_connectionManager);
            m_avsGatewayManager->addObserver(m_capabilitiesDelegate);
            addConnectionObserver(m_capabilitiesDelegate);
            m_endpointRegistrationManager = EndpointRegistrationManager::create(m_directiveSequencer, m_capabilitiesDelegate, m_deviceInfo.getDefaultEndpointId());
            if (!m_endpointRegistrationManager) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "endpointRegistrationManagerCreateFailed"));
                return false;
            }
            m_deviceSettingStorage = deviceSettingStorage;
            if (!m_deviceSettingStorage->open()) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "deviceSettingStorageOpenFailed"));
                return false;
            }
            m_dndCapabilityAgent = DoNotDisturbCapabilityAgent::create(m_exceptionSender, m_connectionManager, m_deviceSettingStorage);
            if (!m_dndCapabilityAgent) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateDNDCapabilityAgent"));
                return false;
            }
            addConnectionObserver(m_dndCapabilityAgent);
            DeviceSettingsManagerBuilder settingsManagerBuilder{m_deviceSettingStorage, m_connectionManager, m_connectionManager, customerDataManager};
            settingsManagerBuilder.withDoNotDisturbSetting(m_dndCapabilityAgent).withAlarmVolumeRampSetting().withWakeWordConfirmationSetting()
                .withSpeechConfirmationSetting().withTimeZoneSetting(systemTimezone).withNetworkInfoSetting();
            if (localeAssetsManager->getDefaultSupportedWakeWords().empty()) settingsManagerBuilder.withLocaleSetting(localeAssetsManager);
            else settingsManagerBuilder.withLocaleAndWakeWordsSettings(localeAssetsManager);
            m_deviceSettingsManager = settingsManagerBuilder.build();
            if (!m_deviceSettingsManager) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "createDeviceSettingsManagerFailed"));
                return false;
            }
            m_audioActivityTracker = AudioActivityTracker::create(contextManager);
            auto interruptModel = InterruptModel::create(ConfigurationNode::getRoot()[INTERRUPT_MODEL_CONFIG_KEY]);
            vector<FocusManager::ChannelConfiguration> audioVirtualChannelConfiguration;
            if (FocusManager::ChannelConfiguration::readChannelConfiguration(AUDIO_CHANNEL_CONFIG_KEY, &audioVirtualChannelConfiguration)) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToReadAudioChannelConfiguration"));
                return false;
            }
            m_audioFocusManager = make_shared<FocusManager>(FocusManager::getDefaultAudioChannels(), m_audioActivityTracker, audioVirtualChannelConfiguration,
                                                            interruptModel);
        #ifdef ENABLE_CAPTIONS
            auto webvttParser = LibwebvttParserAdapter::getInstance();
            m_captionManager = CaptionManager::create(webvttParser);
        #endif
            m_userInactivityMonitor = UserInactivityMonitor::create(m_connectionManager, m_exceptionSender);
            if (!m_userInactivityMonitor) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateUserInactivityMonitor"));
                return false;
            }
            m_systemSoundPlayer = SystemSoundPlayer::create(systemSoundMediaPlayer, audioFactory->systemSounds());
            auto wakeWordConfirmationSetting = settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::WAKEWORD_CONFIRMATION>();
            auto speechConfirmationSetting = settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::SPEECH_CONFIRMATION>();
            auto wakeWordsSetting = settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::WAKE_WORDS>();
        #ifdef ENABLE_OPUS
            m_audioInputProcessor = AudioInputProcessor::create(m_directiveSequencer, m_connectionManager, contextManager, m_audioFocusManager, m_dialogUXStateAggregator,
                                                                m_exceptionSender, m_userInactivityMonitor, m_systemSoundPlayer, localeAssetsManager,
                                                                wakeWordConfirmationSetting, speechConfirmationSetting, wakeWordsSetting,
                                                                make_shared<SpeechEncoder>(std::make_shared<speechencoder::OpusEncoderContext>()),
                                                                AudioProvider::null(), powerResourceManager, metricRecorder);
        #else
            m_audioInputProcessor = AudioInputProcessor::create(m_directiveSequencer,m_connectionManager, contextManager,
                                                    m_audioFocusManager,m_dialogUXStateAggregator,m_exceptionSender,
                                               m_userInactivityMonitor,m_systemSoundPlayer, localeAssetsManager,
                                                                wakeWordConfirmationSetting, speechConfirmationSetting, wakeWordsSetting,nullptr,
                                                                AudioProvider::null(), powerResourceManager, metricRecorder);
        #endif
            if (!m_audioInputProcessor) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAudioInputProcessor"));
                return false;
            }
            addInternetConnectionObserver(m_audioInputProcessor);
            m_audioInputProcessor->addObserver(m_dialogUXStateAggregator);
            m_connectionRetryTrigger = ConnectionRetryTrigger::create(m_connectionManager, m_audioInputProcessor);
            if (!m_connectionRetryTrigger) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateConnectionRetryTrigger"));
                return false;
            }
            m_speechSynthesizer = SpeechSynthesizer::create(speakMediaPlayer,m_connectionManager,m_audioFocusManager,
                                                            contextManager,m_exceptionSender, metricRecorder,
                                                            m_dialogUXStateAggregator,
                                                        #ifdef ENABLE_CAPTIONS
                                                            m_captionManager,
                                                        #else
                                                            nullptr,
                                                        #endif
                                                            powerResourceManager);

            if (!m_speechSynthesizer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSpeechSynthesizer"));
                return false;
            }
            m_speechSynthesizer->addObserver(m_dialogUXStateAggregator);
            m_playbackController = PlaybackController::create(contextManager, m_connectionManager);
            if (!m_playbackController) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreatePlaybackController"));
                return false;
            }
            m_playbackRouter = PlaybackRouter::create(m_playbackController);
            if (!m_playbackRouter) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreatePlaybackRouter"));
                return false;
            }
            vector<shared_ptr<SpeakerInterface>> allAvsSpeakers{speakSpeaker, systemSoundSpeaker};
            vector<shared_ptr<SpeakerInterface>> allAlertSpeakers{alertsSpeaker, notificationsSpeaker};
            for (const auto& it : additionalSpeakers) {
                if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == it.first) allAvsSpeakers.push_back(it.second);
                else if (ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME == it.first) allAlertSpeakers.push_back(it.second);
            }
        #ifdef ENABLE_PCC
            allAvsSpeakers.push_back(phoneSpeaker);
        #endif
        #ifdef ENABLE_MCC
            allAvsSpeakers.push_back(meetingSpeaker);
        #endif
        #ifdef ENABLE_COMMS_AUDIO_PROXY
            allAvsSpeakers.push_back(commsSpeaker);
        #endif
            vector<shared_ptr<ChannelVolumeInterface>> allAvsChannelVolumeInterfaces, allAlertChannelVolumeInterfaces, allChannelVolumeInterfaces;

            for (auto& it : allAvsSpeakers) {
                SpeakerInterface::SpeakerSettings speakerSettings;
                it->getSpeakerSettings(&speakerSettings);
                function<int8_t(int8_t)> volumeCurve;
                allAvsChannelVolumeInterfaces.push_back(channelVolumeFactory->createChannelVolumeInterface(it, ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME,
                                                        volumeCurve));
            }
            vector<shared_ptr<ChannelVolumeInterface>> audioChannelVolumeInterfaces;
            for (auto& it : audioSpeakers) {
                SpeakerInterface::SpeakerSettings speakerSettings;
                it->getSpeakerSettings(&speakerSettings);
                function<int8_t(int8_t)> volumeCurve;
                audioChannelVolumeInterfaces.push_back(channelVolumeFactory->createChannelVolumeInterface(it, ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME,
                                                       volumeCurve));
            }
            allAvsChannelVolumeInterfaces.insert(allAvsChannelVolumeInterfaces.end(), audioChannelVolumeInterfaces.begin(), audioChannelVolumeInterfaces.end());
            function<int8_t(int8_t)> volumeCurveBluetooth;
            shared_ptr<ChannelVolumeInterface> bluetoothChannelVolumeInterface = channelVolumeFactory->createChannelVolumeInterface(bluetoothSpeaker, ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME, volumeCurveBluetooth);
            allAvsChannelVolumeInterfaces.push_back(bluetoothChannelVolumeInterface);
            function<int8_t(int8_t)> volumeCurveRingTone;
            shared_ptr<ChannelVolumeInterface> ringtoneChannelVolumeInterface = channelVolumeFactory->createChannelVolumeInterface(ringtoneSpeaker, ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, volumeCurveRingTone);
            allAvsChannelVolumeInterfaces.push_back(ringtoneChannelVolumeInterface);
            for (auto& it : allAlertSpeakers) {
                function<int8_t(int8_t)> volumeCurve;
                allAlertChannelVolumeInterfaces.push_back(channelVolumeFactory->createChannelVolumeInterface(it, ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME,
                                                          volumeCurve));
            }
            capabilityAgents::externalMediaPlayer::ExternalMediaPlayer::AdapterSpeakerMap externalMusicProviderVolumeInterfaces;
            for (auto& it : externalMusicProviderSpeakers) {
                function<int8_t(int8_t)> volumeCurve;
                auto empChannelVolume = channelVolumeFactory->createChannelVolumeInterface(it.second, ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME, volumeCurve);
                externalMusicProviderVolumeInterfaces[it.first] = empChannelVolume;
                allAvsChannelVolumeInterfaces.push_back(empChannelVolume);
            }
            allChannelVolumeInterfaces.insert(allChannelVolumeInterfaces.end(), allAvsChannelVolumeInterfaces.begin(), allAvsChannelVolumeInterfaces.end());
            allChannelVolumeInterfaces.insert(allChannelVolumeInterfaces.end(), allAlertChannelVolumeInterfaces.begin(), allAlertChannelVolumeInterfaces.end());
            m_speakerManager = capabilityAgents::speakerManager::SpeakerManager::create(allChannelVolumeInterfaces, contextManager, m_connectionManager, m_exceptionSender, metricRecorder);
            if (!m_speakerManager) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSpeakerManager"));
                return false;
            }
            m_audioPlayer = acsdkAudioPlayer::AudioPlayer::create(move(audioMediaPlayerFactory),m_connectionManager,m_audioFocusManager,
                                                                  contextManager, m_exceptionSender, m_playbackRouter,
                                                                  audioChannelVolumeInterfaces,
                                                              #ifdef ENABLE_CAPTIONS
                                                                  m_captionManager,
                                                              #else
                                                    nullptr,
                                                              #endif
                                                                  metricRecorder);
            if (!m_audioPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAudioPlayer"));
                return false;
            }
            auto alertRenderer = Renderer::create(alertsMediaPlayer, metricRecorder);
            if (!alertRenderer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlarmRenderer"));
                return false;
            }
            m_alertsCapabilityAgent = AlertsCapabilityAgent::create(m_connectionManager,m_connectionManager,m_certifiedSender,
                                                                    m_audioFocusManager,m_speakerManager, contextManager,
                                                                    m_exceptionSender, alertStorage, audioFactory->alerts(),alertRenderer,
                                                                    customerDataManager, settingsManagerBuilder.getSetting<settings::ALARM_VOLUME_RAMP>(),
                                                                    m_deviceSettingsManager, metricRecorder, startAlertSchedulingOnInitialization);

            if (!m_alertsCapabilityAgent) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlertsCapabilityAgent"));
                return false;
            }
            m_systemClockMonitor = shared_ptr<SystemClockMonitor>(new SystemClockMonitor());
            m_systemClockMonitor->addSystemClockMonitorObserver(m_alertsCapabilityAgent);
            addConnectionObserver(m_dialogUXStateAggregator);
            m_notificationsRenderer = NotificationRenderer::create(notificationsMediaPlayer, m_audioFocusManager);
            if (!m_notificationsRenderer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateNotificationsRenderer"));
                return false;
            }
            m_notificationsCapabilityAgent = NotificationsCapabilityAgent::create(notificationsStorage,m_notificationsRenderer, contextManager,
                                                                    m_exceptionSender, audioFactory->notifications(), customerDataManager,
                                                                                  metricRecorder);
            if (!m_notificationsCapabilityAgent) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateNotificationsCapabilityAgent"));
                return false;
            }
            m_interactionCapabilityAgent = InteractionModelCapabilityAgent::create(m_directiveSequencer, m_exceptionSender);
            if (!m_interactionCapabilityAgent) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateInteractionModelCapabilityAgent"));
                return false;
            }
            m_interactionCapabilityAgent->addObserver(m_dialogUXStateAggregator);
        #ifdef ENABLE_PCC
            m_phoneCallControllerCapabilityAgent = PhoneCallController::create(contextManager, m_connectionManager, phoneCaller, phoneSpeaker, m_audioFocusManager,
                                                                               m_exceptionSender);
            if (!m_phoneCallControllerCapabilityAgent) ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreatePhoneCallControllerCapabilityAgent"));
        #endif
        #ifdef ENABLE_MCC
            m_meetingClientControllerCapabilityAgent = MeetingClientController::create(contextManager, m_connectionManager, meetingClient, calendarClient,
                                                                                       m_speakerManager, m_audioFocusManager, m_exceptionSender);
            if (!m_meetingClientControllerCapabilityAgent) ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateMeetingClientControllerCapabilityAgent"));
        #endif
            m_externalMediaPlayer = ExternalMediaPlayer::create(externalMusicProviderMediaPlayers, externalMusicProviderVolumeInterfaces, adapterCreationMap,
                                                  m_speakerManager,m_connectionManager, m_certifiedSender,
                                                    m_audioFocusManager, contextManager,m_exceptionSender,
                                                   m_playbackRouter, metricRecorder);
            if (!m_externalMediaPlayer) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateExternalMediaPlayer"));
                return false;
            }
            if (isGuiSupported) {
                m_visualActivityTracker = VisualActivityTracker::create(contextManager);
                vector<FocusManager::ChannelConfiguration> visualVirtualChannelConfiguration;
                if (!FocusManager::ChannelConfiguration::readChannelConfiguration(VISUAL_CHANNEL_CONFIG_KEY, &visualVirtualChannelConfiguration)) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToReadVisualChannels"));
                    return false;
                }
                m_visualFocusManager = make_shared<FocusManager>(FocusManager::getDefaultVisualChannels(), m_visualActivityTracker,
                                                                 visualVirtualChannelConfiguration, interruptModel);
                unordered_set<shared_ptr<RenderPlayerInfoCardsProviderInterface>> renderPlayerInfoCardsProviders = {m_audioPlayer, m_externalMediaPlayer};
                m_templateRuntime = TemplateRuntime::create(renderPlayerInfoCardsProviders, m_visualFocusManager, m_exceptionSender);
                if (!m_templateRuntime) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateTemplateRuntimeCapabilityAgent"));
                    return false;
                }
                m_dialogUXStateAggregator->addObserver(m_templateRuntime);
                if (externalCapabilitiesBuilder) {
                    externalCapabilitiesBuilder->withTemplateRunTime(m_templateRuntime);
                }
            }
            m_equalizerRuntimeSetup = equalizerRuntimeSetup;
            if (nullptr != m_equalizerRuntimeSetup) {
                auto equalizerController = EqualizerController::create(equalizerRuntimeSetup->getModeController(), equalizerRuntimeSetup->getConfiguration(),
                                                                       equalizerRuntimeSetup->getStorage());
                if (!equalizerController) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateEqualizerController"));
                    return false;
                }
                m_equalizerCapabilityAgent = EqualizerCapabilityAgent::create(
                    equalizerController,
                    m_capabilitiesDelegate,
                    equalizerRuntimeSetup->getStorage(),
                    customerDataManager,
                    m_exceptionSender,
                    contextManager,
                    m_connectionManager);
                if (!m_equalizerCapabilityAgent) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateEqualizerCapabilityAgent"));
                    return false;
                }
                m_equalizerController = equalizerController;
                for (auto& equalizer : m_equalizerRuntimeSetup->getAllEqualizers()) equalizerController->registerEqualizer(equalizer);
                for (auto& listener : m_equalizerRuntimeSetup->getAllEqualizerControllerListeners()) equalizerController->addListener(listener);
            } else ACSDK_DEBUG3(LX(__func__).m("Equalizer is disabled"));
            auto timezoneHandler = TimeZoneHandler::create(settingsManagerBuilder.getSetting<settings::TIMEZONE>(), m_exceptionSender);
            if (!timezoneHandler) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateTimeZoneHandler"));
                return false;
            }
            auto localeHandler = LocaleHandler::create(m_exceptionSender, settingsManagerBuilder.getSetting<settings::DeviceSettingsIndex::LOCALE>());
            if (!localeHandler) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateLocaleHandler"));
                return false;
            }
            auto reportGenerator = StateReportGenerator::create(m_deviceSettingsManager, settingsManagerBuilder.getConfigurations());
            if (!reportGenerator.hasValue()) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateStateReportGenerator"));
                return false;
            }
            vector<StateReportGenerator> reportGenerators{{reportGenerator.value()}};
            shared_ptr<ReportStateHandler> reportStateHandler = ReportStateHandler::create(customerDataManager,m_exceptionSender,
                                                                           m_connectionManager, m_connectionManager,
                                                                                           miscStorage, reportGenerators);
            if (!reportStateHandler) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateReportStateHandler"));
                return false;
            }
            auto systemCapabilityProvider = capabilityAgents::system::SystemCapabilityProvider::create(localeAssetsManager);
            if (!systemCapabilityProvider) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSystemCapabilityProvider"));
                return false;
            }
        #ifdef ENABLE_REVOKE_AUTH
            m_revokeAuthorizationHandler = capabilityAgents::system::RevokeAuthorizationHandler::create(m_exceptionSender);
            if (!m_revokeAuthorizationHandler) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateRevokeAuthorizationHandler"));
                return false;
            }
        #endif
            if (bluetoothDeviceManager) {
                ACSDK_DEBUG5(LX(__func__).m("Creating Bluetooth CA"));
                auto eventBus = bluetoothDeviceManager->getEventBus();
                auto bluetoothMediaInputTransformer = BluetoothMediaInputTransformer::create(eventBus, m_playbackRouter);
                m_bluetooth = Bluetooth::create(contextManager,m_audioFocusManager,m_connectionManager,m_exceptionSender,
                                                move(bluetoothStorage), move(bluetoothDeviceManager), move(eventBus), bluetoothMediaPlayer, customerDataManager,
                                                enabledConnectionRules, bluetoothChannelVolumeInterface, bluetoothMediaInputTransformer);
            } else ACSDK_DEBUG5(LX("bluetoothCapabilityAgentDisabled").d("reason", "nullBluetoothDeviceManager"));
            m_apiGatewayCapabilityAgent = ApiGatewayCapabilityAgent::create(m_avsGatewayManager, m_exceptionSender);
            if (!m_apiGatewayCapabilityAgent) ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateApiGatewayCapabilityAgent"));
            m_diagnostics = diagnostics;
            if (m_diagnostics) {
                m_diagnostics->setDiagnosticDependencies(m_directiveSequencer, attachmentManager);
                auto deviceProperties = m_diagnostics->getDevicePropertyAggregator();
                if (deviceProperties) {
                    deviceProperties->setContextManager(contextManager);
                    deviceProperties->initializeVolume(m_speakerManager);
                    addSpeakerManagerObserver(deviceProperties);
                    addAlertsObserver(deviceProperties);
                    addConnectionObserver(deviceProperties);
                    addNotificationsObserver(deviceProperties);
                    addAudioPlayerObserver(deviceProperties);
                    addAlexaDialogStateObserver(deviceProperties);
                }
                auto protocolTrace = m_diagnostics->getProtocolTracer();
                if (protocolTrace) addMessageObserver(protocolTrace);
            } else ACSDK_DEBUG0(LX(__func__).m("Diagnostics Not Enabled"));
            m_alexaMessageSender = AlexaInterfaceMessageSender::create(m_contextManager, m_connectionManager);
            if (!m_alexaMessageSender) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlexaMessageSender"));
                return false;
            }
            m_alexaCapabilityAgent = AlexaInterfaceCapabilityAgent::create(m_deviceInfo, m_deviceInfo.getDefaultEndpointId(), m_exceptionSender, m_alexaMessageSender);
            if (!m_alexaCapabilityAgent) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlexaCapabilityAgent"));
                return false;
            }
            m_alexaCapabilityAgent->addEventProcessedObserver(m_capabilitiesDelegate);
            m_defaultEndpointBuilder = EndpointBuilder::create(m_deviceInfo, m_contextManager, m_exceptionSender, m_alexaMessageSender);
            m_defaultEndpointBuilder->withCapabilityConfiguration(m_softwareReporterCapabilityAgent);
            m_defaultEndpointBuilder->withCapability(m_speechSynthesizer, m_speechSynthesizer);
            m_defaultEndpointBuilder->withCapability(m_audioPlayer, m_audioPlayer);
            m_defaultEndpointBuilder->withCapability(m_externalMediaPlayer, m_externalMediaPlayer);
            m_defaultEndpointBuilder->withCapability(m_audioInputProcessor, m_audioInputProcessor);
            m_defaultEndpointBuilder->withCapability(m_alertsCapabilityAgent, m_alertsCapabilityAgent);
            m_defaultEndpointBuilder->withCapability(m_apiGatewayCapabilityAgent, m_apiGatewayCapabilityAgent);
            m_defaultEndpointBuilder->withCapability(m_alexaCapabilityAgent->getCapabilityConfiguration(), m_alexaCapabilityAgent);
            m_defaultEndpointBuilder->withCapabilityConfiguration(m_audioActivityTracker);
            m_defaultEndpointBuilder->withCapabilityConfiguration(m_playbackController);
        #ifdef ENABLE_PCC
            if (m_phoneCallControllerCapabilityAgent) {
                m_defaultEndpointBuilder->withCapability(m_phoneCallControllerCapabilityAgent, m_phoneCallControllerCapabilityAgent);
            }
        #endif
        #ifdef ENABLE_MCC
            if (m_meetingClientControllerCapabilityAgent) {
                m_defaultEndpointBuilder->withCapability(m_meetingClientControllerCapabilityAgent, m_meetingClientControllerCapabilityAgent);
            }
        #endif
            m_defaultEndpointBuilder->withCapability(m_speakerManager, m_speakerManager);
            if (isGuiSupported) {
                m_defaultEndpointBuilder->withCapability(m_templateRuntime, m_templateRuntime);
                m_defaultEndpointBuilder->withCapabilityConfiguration(m_visualActivityTracker);
            }
            m_defaultEndpointBuilder->withCapability(m_notificationsCapabilityAgent, m_notificationsCapabilityAgent);
            m_defaultEndpointBuilder->withCapability(m_interactionCapabilityAgent, m_interactionCapabilityAgent);
            if (m_bluetooth) m_defaultEndpointBuilder->withCapability(m_bluetooth, m_bluetooth);
            if (m_equalizerCapabilityAgent) m_defaultEndpointBuilder->withCapability(m_equalizerCapabilityAgent, m_equalizerCapabilityAgent);
            m_defaultEndpointBuilder->withCapability(m_dndCapabilityAgent, m_dndCapabilityAgent);
            m_defaultEndpointBuilder->withCapabilityConfiguration(systemCapabilityProvider);
            if (!m_directiveSequencer->addDirectiveHandler(std::move(localeHandler)) ||
                !m_directiveSequencer->addDirectiveHandler(std::move(timezoneHandler)) ||
                !m_directiveSequencer->addDirectiveHandler(reportStateHandler) ||
        #ifdef ENABLE_REVOKE_AUTH
                !m_directiveSequencer->addDirectiveHandler(m_revokeAuthorizationHandler) ||
        #endif
                !m_directiveSequencer->addDirectiveHandler(m_userInactivityMonitor)) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToRegisterSystemDirectiveHandler"));
                return false;
            }
            if (externalCapabilitiesBuilder) {
                externalCapabilitiesBuilder->withSettingsStorage(m_deviceSettingStorage);
                externalCapabilitiesBuilder->withInternetConnectionMonitor(m_internetConnectionMonitor);
                externalCapabilitiesBuilder->withDialogUXStateAggregator(m_dialogUXStateAggregator);
                if (isGuiSupported) externalCapabilitiesBuilder->withVisualFocusManager(m_visualFocusManager);
                auto externalCapabilities = externalCapabilitiesBuilder->buildCapabilities(m_externalMediaPlayer,m_connectionManager,
                                                                              m_connectionManager,m_exceptionSender,
                                                                              m_certifiedSender, m_audioFocusManager,
                                                                                           customerDataManager, reportStateHandler, m_audioInputProcessor,
                                                                             m_speakerManager, m_directiveSequencer,
                                                                          m_userInactivityMonitor, m_contextManager,
                                                                          m_avsGatewayManager, ringtoneMediaPlayer, audioFactory,
                                                                                           ringtoneChannelVolumeInterface,
                                                                                       #ifdef ENABLE_COMMS_AUDIO_PROXY
                                                                                           commsMediaPlayer, commsSpeaker, sharedDataStream,
                                                                                       #endif
                                                                                           powerResourceManager);
                for (auto& capability : externalCapabilities.first) {
                    if (capability.configuration.hasValue()) m_defaultEndpointBuilder->withCapability(capability.configuration.value(), capability.directiveHandler);
                    else m_directiveSequencer->addDirectiveHandler(capability.directiveHandler);
                }
                m_shutdownObjects.insert(m_shutdownObjects.end(), externalCapabilities.second.begin(), externalCapabilities.second.end());
                m_callManager = externalCapabilitiesBuilder->getCallManager();
            }
            if (softwareInfoSenderObserver) m_softwareInfoSenderObservers.insert(softwareInfoSenderObserver);
            if (m_callManager) m_softwareInfoSenderObservers.insert(m_callManager);
            if (isValidFirmwareVersion(firmwareVersion)) {
                auto tempSender = SoftwareInfoSender::create(firmwareVersion, sendSoftwareInfoOnConnected,m_softwareInfoSenderObservers,
                                                   m_connectionManager,m_connectionManager,m_exceptionSender);
                if (tempSender) {
                    lock_guard<mutex> lock(m_softwareInfoSenderMutex);
                    m_softwareInfoSender = tempSender;
                } else {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSoftwareInfoSender"));
                    return false;
                }
            }
            if (!m_defaultEndpointBuilder->finishDefaultEndpointConfiguration()) {
                ACSDK_ERROR(LX("initializedFailed").d("reason", "defaultEndpointConfigurationFailed"));
                return false;
            }
            return true;
        }
        void DefaultClient::connect(bool performReset) {
            if (performReset) {
                if (m_defaultEndpointBuilder) {
                    auto defaultEndpoint = m_defaultEndpointBuilder->buildDefaultEndpoint();
                    if (!defaultEndpoint) {
                        ACSDK_CRITICAL(LX("connectFailed").d("reason", "couldNotBuildDefaultEndpoint"));
                        return;
                    }
                    auto resultFuture = m_endpointRegistrationManager->registerEndpoint(move(defaultEndpoint));
                    if ((resultFuture.wait_for(std::chrono::milliseconds(0)) == future_status::ready)) {
                        auto result = resultFuture.get();
                        if (result != alexaClientSDK::endpoints::EndpointRegistrationManager::RegistrationResult::SUCCEEDED) {
                            ACSDK_CRITICAL(LX("connectFailed").d("reason", "registrationFailed").d("result", result));
                            return;
                        }
                    }
                    m_defaultEndpointBuilder.reset();
                }
                m_endpointRegistrationManager->waitForPendingRegistrationsToEnqueue();
                m_avsGatewayManager->setAVSGatewayAssigner(m_connectionManager);
            }
            m_connectionManager->enable();
        }
        void DefaultClient::disconnect() {
            m_connectionManager->disable();
        }
        string DefaultClient::getAVSGateway() {
            return m_connectionManager->getAVSGateway();
        }
        void DefaultClient::stopForegroundActivity() {
            m_audioFocusManager->stopForegroundActivity();
        }
        void DefaultClient::stopAllActivities() {
            m_audioFocusManager->stopAllActivities();
        }
        void DefaultClient::localStopActiveAlert() {
            m_alertsCapabilityAgent->onLocalStop();
        }
        void DefaultClient::addAlexaDialogStateObserver(
            shared_ptr<DialogUXStateObserverInterface> observer) {
            m_dialogUXStateAggregator->addObserver(observer);
        }
        void DefaultClient::removeAlexaDialogStateObserver(
            shared_ptr<DialogUXStateObserverInterface> observer) {
            m_dialogUXStateAggregator->removeObserver(observer);
        }
        void DefaultClient::addMessageObserver(shared_ptr<MessageObserverInterface> observer) {
            m_connectionManager->addMessageObserver(observer);
        }
        void DefaultClient::removeMessageObserver(
            shared_ptr<MessageObserverInterface> observer) {
            m_connectionManager->removeMessageObserver(observer);
        }
        void DefaultClient::addConnectionObserver(
            shared_ptr<ConnectionStatusObserverInterface> observer) {
            m_connectionManager->addConnectionStatusObserver(observer);
        }
        void DefaultClient::removeConnectionObserver(
            shared_ptr<ConnectionStatusObserverInterface> observer) {
            m_connectionManager->removeConnectionStatusObserver(observer);
        }
        void DefaultClient::addInternetConnectionObserver(
            shared_ptr<InternetConnectionObserverInterface> observer) {
            m_internetConnectionMonitor->addInternetConnectionObserver(observer);
        }
        void DefaultClient::removeInternetConnectionObserver(
            shared_ptr<InternetConnectionObserverInterface> observer) {
            m_internetConnectionMonitor->removeInternetConnectionObserver(observer);
        }
        void DefaultClient::addAlertsObserver(shared_ptr<AlertObserverInterface> observer) {
            m_alertsCapabilityAgent->addObserver(observer);
        }
        void DefaultClient::removeAlertsObserver(shared_ptr<AlertObserverInterface> observer) {
            m_alertsCapabilityAgent->removeObserver(observer);
        }
        void DefaultClient::addAudioPlayerObserver(
            shared_ptr<acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface> observer) {
            m_audioPlayer->addObserver(observer);
        }
        void DefaultClient::removeAudioPlayerObserver(
            shared_ptr<AudioPlayerObserverInterface> observer) {
            m_audioPlayer->removeObserver(observer);
        }
        void DefaultClient::addTemplateRuntimeObserver(
            shared_ptr<TemplateRuntimeObserverInterface> observer) {
            if (!m_templateRuntime) {
                ACSDK_ERROR(LX("addTemplateRuntimeObserverFailed").d("reason", "guiNotSupported"));
                return;
            }
            m_templateRuntime->addObserver(observer);
        }
        void DefaultClient::removeTemplateRuntimeObserver(
            shared_ptr<TemplateRuntimeObserverInterface> observer) {
            if (!m_templateRuntime) {
                ACSDK_ERROR(LX("removeTemplateRuntimeObserverFailed").d("reason", "guiNotSupported"));
                return;
            }
            m_templateRuntime->removeObserver(observer);
        }
        void DefaultClient::TemplateRuntimeDisplayCardCleared() {
            if (!m_templateRuntime) {
                ACSDK_ERROR(LX("TemplateRuntimeDisplayCardClearedFailed").d("reason", "guiNotSupported"));
                return;
            }
            m_templateRuntime->displayCardCleared();
        }
        void DefaultClient::addNotificationsObserver(
            shared_ptr<NotificationsObserverInterface> observer) {
            m_notificationsCapabilityAgent->addObserver(observer);
        }
        void DefaultClient::removeNotificationsObserver(
            shared_ptr<NotificationsObserverInterface> observer) {
            m_notificationsCapabilityAgent->removeObserver(observer);
        }
        void DefaultClient::addExternalMediaPlayerObserver(
            shared_ptr<ExternalMediaPlayerObserverInterface> observer) {
            m_externalMediaPlayer->addObserver(observer);
        }
        void DefaultClient::removeExternalMediaPlayerObserver(
            std::shared_ptr<avsCommon::sdkInterfaces::externalMediaPlayer::ExternalMediaPlayerObserverInterface> observer) {
            m_externalMediaPlayer->removeObserver(observer);
        }
        void DefaultClient::addCaptionPresenter(std::shared_ptr<captions::CaptionPresenterInterface> presenter) {
        #ifdef ENABLE_CAPTIONS
            if (m_captionManager) m_captionManager->setCaptionPresenter(presenter);
        #endif
        }
        void DefaultClient::setCaptionMediaPlayers(
            const vector<shared_ptr<MediaPlayerInterface>>& mediaPlayers) {
        #ifdef ENABLE_CAPTIONS
            if (m_captionManager) m_captionManager->setMediaPlayers(mediaPlayers);
        #endif
        }
        void DefaultClient::addBluetoothDeviceObserver(
            shared_ptr<BluetoothDeviceObserverInterface> observer) {
            if (!m_bluetooth) {
                ACSDK_DEBUG5(LX(__func__).m("bluetooth is disabled, not adding observer"));
                return;
            }
            m_bluetooth->addObserver(observer);
        }
        void DefaultClient::removeBluetoothDeviceObserver(
            shared_ptr<BluetoothDeviceObserverInterface> observer) {
            if (!m_bluetooth) return;
            m_bluetooth->removeObserver(observer);
        }
        #ifdef ENABLE_REVOKE_AUTH
        void DefaultClient::addRevokeAuthorizationObserver(
            shared_ptr<RevokeAuthorizationObserverInterface> observer) {
            if (!m_revokeAuthorizationHandler) {
                ACSDK_ERROR(LX("addRevokeAuthorizationObserver").d("reason", "revokeAuthorizationNotSupported"));
                return;
            }
            m_revokeAuthorizationHandler->addObserver(observer);
        }
        void DefaultClient::removeRevokeAuthorizationObserver(
            shared_ptr<RevokeAuthorizationObserverInterface> observer) {
            if (!m_revokeAuthorizationHandler) {
                ACSDK_ERROR(LX("removeRevokeAuthorizationObserver").d("reason", "revokeAuthorizationNotSupported"));
                return;
            }
            m_revokeAuthorizationHandler->removeObserver(observer);
        }
        #endif
        shared_ptr<DeviceSettingsManager> DefaultClient::getSettingsManager() {
            return m_deviceSettingsManager;
        }
        shared_ptr<PlaybackRouterInterface> DefaultClient::getPlaybackRouter() const {
            return m_playbackRouter;
        }
        shared_ptr<RegistrationManager> DefaultClient::getRegistrationManager() {
            return m_registrationManager;
        }
        shared_ptr<EqualizerController> DefaultClient::getEqualizerController() {
            return m_equalizerController;
        }
        void DefaultClient::addEqualizerControllerListener(
            shared_ptr<EqualizerControllerListenerInterface> listener) {
            if (m_equalizerController) m_equalizerController->addListener(listener);
        }
        void DefaultClient::removeEqualizerControllerListener(
            shared_ptr<EqualizerControllerListenerInterface> listener) {
            if (m_equalizerController) m_equalizerController->removeListener(listener);
        }
        void DefaultClient::addContextManagerObserver(
            shared_ptr<ContextManagerObserverInterface> observer) {
            if (m_contextManager) m_contextManager->addContextManagerObserver(observer);
        }
        void DefaultClient::removeContextManagerObserver(
            shared_ptr<ContextManagerObserverInterface> observer) {
            if (m_contextManager) m_contextManager->removeContextManagerObserver(observer);
        }
        void DefaultClient::addSpeakerManagerObserver(
            shared_ptr<SpeakerManagerObserverInterface> observer) {
            m_speakerManager->addSpeakerManagerObserver(observer);
        }
        void DefaultClient::removeSpeakerManagerObserver(
            shared_ptr<SpeakerManagerObserverInterface> observer) {
            m_speakerManager->removeSpeakerManagerObserver(observer);
        }
        shared_ptr<SpeakerManagerInterface> DefaultClient::getSpeakerManager() {
            return m_speakerManager;
        }
        void DefaultClient::addSpeechSynthesizerObserver(
            shared_ptr<SpeechSynthesizerObserverInterface> observer) {
            m_speechSynthesizer->addObserver(observer);
        }
        void DefaultClient::removeSpeechSynthesizerObserver(
            shared_ptr<SpeechSynthesizerObserverInterface> observer) {
            m_speechSynthesizer->removeObserver(observer);
        }
        bool DefaultClient::setFirmwareVersion(avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion) {
            {
                lock_guard<mutex> lock(m_softwareInfoSenderMutex);
                if (!m_softwareInfoSender) {
                    m_softwareInfoSender = SoftwareInfoSender::create(firmwareVersion,true,m_softwareInfoSenderObservers,
                                                            m_connectionManager, m_connectionManager, m_exceptionSender);
                    if (m_softwareInfoSender) return true;
                    ACSDK_ERROR(LX("setFirmwareVersionFailed").d("reason", "unableToCreateSoftwareInfoSender"));
                    return false;
                }
            }
            return m_softwareInfoSender->setFirmwareVersion(firmwareVersion);
        }
        future<bool> DefaultClient::notifyOfWakeWord(AudioProvider wakeWordAudioProvider, AudioInputStream::Index beginIndex, AudioInputStream::Index endIndex,
                                                     string keyword, steady_clock::time_point startOfSpeechTimestamp, shared_ptr<const vector<char>> KWDMetadata) {
            ACSDK_DEBUG5(LX(__func__).d("keyword", keyword).d("connected", m_connectionManager->isConnected()));
            if (!m_connectionManager->isConnected()) {
                promise<bool> ret;
                if (AudioInputProcessor::KEYWORD_TEXT_STOP == keyword) {
                    ACSDK_INFO(LX("notifyOfWakeWord").d("action", "localStop").d("reason", "stopUtteredWhileNotConnected"));
                    stopForegroundActivity();
                    ret.set_value(true);
                    return ret.get_future();
                } else {
                    ACSDK_INFO(LX("notifyOfWakeWord").d("action", "ignoreAlexaWakeWord").d("reason", "networkDisconnected"));
                    ret.set_value(false);
                    return ret.get_future();
                }
            }
            return m_audioInputProcessor->recognize(wakeWordAudioProvider, Initiator::WAKEWORD, startOfSpeechTimestamp, beginIndex, endIndex, keyword, KWDMetadata);
        }
        future<bool> DefaultClient::notifyOfTapToTalk(AudioProvider tapToTalkAudioProvider, AudioInputStream::Index beginIndex,
                                                      steady_clock::time_point startOfSpeechTimestamp) {
            ACSDK_DEBUG5(LX(__func__));
            return m_audioInputProcessor->recognize(tapToTalkAudioProvider,Initiator::TAP, startOfSpeechTimestamp, beginIndex);
        }
        future<bool> DefaultClient::notifyOfHoldToTalkStart(AudioProvider holdToTalkAudioProvider, steady_clock::time_point startOfSpeechTimestamp) {
            ACSDK_DEBUG5(LX(__func__));
            return m_audioInputProcessor->recognize(holdToTalkAudioProvider,Initiator::PRESS_AND_HOLD, startOfSpeechTimestamp);
        }
        future<bool> DefaultClient::notifyOfHoldToTalkEnd() {
            return m_audioInputProcessor->stopCapture();
        }
        future<bool> DefaultClient::notifyOfTapToTalkEnd() {
            return m_audioInputProcessor->stopCapture();
        }
        void DefaultClient::addCallStateObserver(
            shared_ptr<CallStateObserverInterface> observer) {
            if (m_callManager) m_callManager->addObserver(observer);
        }
        void DefaultClient::removeCallStateObserver(shared_ptr<CallStateObserverInterface> observer) {
            if (m_callManager) m_callManager->removeObserver(observer);
        }
        unique_ptr<EndpointBuilderInterface> DefaultClient::createEndpointBuilder() {
            return EndpointBuilder::create(m_deviceInfo, m_contextManager, m_exceptionSender, m_alexaMessageSender);
        }
        shared_ptr<EndpointBuilderInterface> DefaultClient::getDefaultEndpointBuilder() {
            return m_defaultEndpointBuilder;
        }
        future<EndpointRegistrationManager::RegistrationResult> DefaultClient::registerEndpoint(
            shared_ptr<EndpointInterface> endpoint) {
            if (m_endpointRegistrationManager) return m_endpointRegistrationManager->registerEndpoint(endpoint);
            else {
                ACSDK_ERROR(LX("registerEndpointFailed").d("reason", "invalid EndpointRegistrationManager"));
                promise<EndpointRegistrationManager::RegistrationResult> promise;
                promise.set_value(EndpointRegistrationManager::RegistrationResult::INTERNAL_ERROR);
                return promise.get_future();
            }
        }
        future<EndpointRegistrationManager::DeregistrationResult> DefaultClient::
            deregisterEndpoint(EndpointIdentifier endpointId) {
            if (m_endpointRegistrationManager) return m_endpointRegistrationManager->deregisterEndpoint(endpointId);
            else {
                ACSDK_ERROR(LX("deregisterEndpointFailed").d("reason", "invalid EndpointRegistrationManager"));
                promise<EndpointRegistrationManager::DeregistrationResult> promise;
                promise.set_value(EndpointRegistrationManager::DeregistrationResult::INTERNAL_ERROR);
                return promise.get_future();
            }
        }
        bool DefaultClient::isCommsEnabled() {
            return (m_callManager != nullptr);
        }
        void DefaultClient::acceptCommsCall() {
            if (m_callManager) m_callManager->acceptCall();
        }
        void DefaultClient::sendDtmf(avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone dtmfTone) {
            if (m_callManager) m_callManager->sendDtmf(dtmfTone);
        }
        void DefaultClient::stopCommsCall() {
            if (m_callManager) m_callManager->stopCall();
        }
        void DefaultClient::audioPlayerLocalStop() {
            if (m_audioPlayer) m_audioPlayer->stopPlayback();
        }
        bool DefaultClient::isCommsCallMuted() {
            if (m_callManager) return m_callManager->isSelfMuted();
            return false;
        }
        void DefaultClient::muteCommsCall() {
            if (m_callManager) m_callManager->muteSelf();
        }
        void DefaultClient::unmuteCommsCall() {
            if (m_callManager) m_callManager->unmuteSelf();
        }
        void DefaultClient::onSystemClockSynchronized() {
            m_systemClockMonitor->notifySystemClockSynchronized();
        }
        DefaultClient::~DefaultClient() {
            while(!m_shutdownObjects.empty()) {
                if (m_shutdownObjects.back()) m_shutdownObjects.back()->shutdown();
                m_shutdownObjects.pop_back();
            }
            if (m_directiveSequencer) {
                ACSDK_DEBUG5(LX("DirectiveSequencerShutdown"));
                m_directiveSequencer->shutdown();
            }
            if (m_speakerManager) {
                ACSDK_DEBUG5(LX("SpeakerManagerShutdown"));
                m_speakerManager->shutdown();
            }
            if (m_templateRuntime) {
                ACSDK_DEBUG5(LX("TemplateRuntimeShutdown"));
                m_templateRuntime->shutdown();
            }
            if (m_audioInputProcessor) {
                ACSDK_DEBUG5(LX("AIPShutdown"));
                removeInternetConnectionObserver(m_audioInputProcessor);
                m_audioInputProcessor->shutdown();
            }
            if (m_audioPlayer) {
                ACSDK_DEBUG5(LX("AudioPlayerShutdown"));
                m_audioPlayer->shutdown();
            }
            if (m_externalMediaPlayer) {
                ACSDK_DEBUG5(LX("ExternalMediaPlayerShutdown"));
                m_externalMediaPlayer->shutdown();
            }
            if (m_speechSynthesizer) {
                ACSDK_DEBUG5(LX("SpeechSynthesizerShutdown"));
                m_speechSynthesizer->shutdown();
            }
            if (m_alertsCapabilityAgent) {
                m_systemClockMonitor->removeSystemClockMonitorObserver(m_alertsCapabilityAgent);
                ACSDK_DEBUG5(LX("AlertsShutdown"));
                m_alertsCapabilityAgent->shutdown();
            }
            if (m_playbackController) {
                ACSDK_DEBUG5(LX("PlaybackControllerShutdown"));
                m_playbackController->shutdown();
            }
            if (m_softwareInfoSender) {
                ACSDK_DEBUG5(LX("SoftwareInfoShutdown"));
                m_softwareInfoSender->shutdown();
            }
            if (m_messageRouter) {
                ACSDK_DEBUG5(LX("MessageRouterShutdown."));
                m_messageRouter->shutdown();
            }
            if (m_connectionManager) {
                ACSDK_DEBUG5(LX("ConnectionManagerShutdown."));
                m_connectionManager->shutdown();
            }
            if (m_certifiedSender) {
                ACSDK_DEBUG5(LX("CertifiedSenderShutdown."));
                m_certifiedSender->shutdown();
            }
            if (m_audioActivityTracker) {
                ACSDK_DEBUG5(LX("AudioActivityTrackerShutdown."));
                m_audioActivityTracker->shutdown();
            }
            if (m_visualActivityTracker) {
                ACSDK_DEBUG5(LX("VisualActivityTrackerShutdown."));
                m_visualActivityTracker->shutdown();
            }
            if (m_playbackRouter) {
                ACSDK_DEBUG5(LX("PlaybackRouterShutdown."));
                m_playbackRouter->shutdown();
            }
            if (m_notificationsCapabilityAgent) {
                ACSDK_DEBUG5(LX("NotificationsShutdown."));
                m_notificationsCapabilityAgent->shutdown();
            }
            if (m_notificationsRenderer) {
                ACSDK_DEBUG5(LX("NotificationsRendererShutdown."));
                m_notificationsRenderer->shutdown();
            }
        #ifdef ENABLE_CAPTIONS
            if (m_captionManager) {
                ACSDK_DEBUG5(LX("CaptionManagerShutdown."));
                m_captionManager->shutdown();
            }
        #endif
            if (m_bluetooth) {
                ACSDK_DEBUG5(LX("BluetoothShutdown."));
                m_bluetooth->shutdown();
            }
            if (m_userInactivityMonitor) {
                ACSDK_DEBUG5(LX("UserInactivityMonitorShutdown."));
                m_userInactivityMonitor->shutdown();
            }
            if (m_apiGatewayCapabilityAgent) {
                ACSDK_DEBUG5(LX("CallApiGatewayCapabilityAgentShutdown."));
                m_apiGatewayCapabilityAgent->shutdown();
            }
            if (m_alexaCapabilityAgent) m_alexaCapabilityAgent->removeEventProcessedObserver(m_capabilitiesDelegate);
            if (m_alexaMessageSender) {
                ACSDK_DEBUG5(LX("CallAlexaInterfaceMessageSenderShutdown."));
                m_alexaMessageSender->shutdown();
            }
        #ifdef ENABLE_PCC
            if (m_phoneCallControllerCapabilityAgent) {
                ACSDK_DEBUG5(LX("PhoneCallControllerCapabilityAgentShutdown"));
                m_phoneCallControllerCapabilityAgent->shutdown();
            }
        #endif
        #ifdef ENABLE_MCC
            if (m_meetingClientControllerCapabilityAgent) {
                ACSDK_DEBUG5(LX("MeetingClientControllerCapabilityAgentShutdown"));
                m_meetingClientControllerCapabilityAgent->shutdown();
            }
        #endif
            if (m_dndCapabilityAgent) {
                ACSDK_DEBUG5(LX("DNDCapabilityAgentShutdown"));
                removeConnectionObserver(m_dndCapabilityAgent);
                m_dndCapabilityAgent->shutdown();
            }
            if (nullptr != m_equalizerCapabilityAgent) {
                for (auto& equalizer : m_equalizerRuntimeSetup->getAllEqualizers()) m_equalizerController->unregisterEqualizer(equalizer);
                for (auto& listener : m_equalizerRuntimeSetup->getAllEqualizerControllerListeners()) m_equalizerController->removeListener(listener);
                ACSDK_DEBUG5(LX("EqualizerCapabilityAgentShutdown"));
                m_equalizerCapabilityAgent->shutdown();
            }
            if (m_deviceSettingStorage) {
                ACSDK_DEBUG5(LX("CloseSettingStorage"));
                m_deviceSettingStorage->close();
            }
            if (m_diagnostics) {
                m_diagnostics->setDiagnosticDependencies(nullptr, nullptr);
                auto deviceProperties = m_diagnostics->getDevicePropertyAggregator();
                if (deviceProperties) {
                    deviceProperties->setContextManager(nullptr);
                    removeSpeakerManagerObserver(deviceProperties);
                    removeAlertsObserver(deviceProperties);
                    removeConnectionObserver(deviceProperties);
                    removeNotificationsObserver(deviceProperties);
                    removeAudioPlayerObserver(deviceProperties);
                    removeAlexaDialogStateObserver(deviceProperties);
                }
                auto protocolTrace = m_diagnostics->getProtocolTracer();
                if (protocolTrace) removeMessageObserver(protocolTrace);
            }
        }
    }
}
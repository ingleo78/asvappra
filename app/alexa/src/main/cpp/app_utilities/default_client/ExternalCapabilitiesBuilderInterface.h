#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_EXTERNALCAPABILITIESBUILDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_EXTERNALCAPABILITIESBUILDERINTERFACE_H_

#include <list>
#include <utility>
#include <capability_agents/AIP/AudioInputProcessor.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/DialogUXStateAggregator.h>
#include <avs/ExceptionEncounteredSender.h>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <sdkinterfaces/AVSGatewayManagerInterface.h>
#include <sdkinterfaces/Audio/AudioFactoryInterface.h>
#include <sdkinterfaces/CallManagerInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <sdkinterfaces/InternetConnectionMonitorInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/SpeakerManagerInterface.h>
#include <media_player/MediaPlayerInterface.h>
#include <util/Optional.h>
#include <util/RequiresShutdown.h>
#include <certified_sender/CertifiedSender.h>
#include <capability_agents/ExternalMediaPlayer/ExternalMediaPlayer.h>
#include <registration_manager/CustomerDataManager.h>
#include <settings/Storage/DeviceSettingStorageInterface.h>
#include <capability_agents/SpeakerManager/DefaultChannelVolumeFactory.h>
#include <capability_agents/System/ReportStateHandler.h>
#include <capability_agents/System/UserInactivityMonitor.h>
#include <capability_agents/TemplateRuntime/TemplateRuntime.h>

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace audio;
        using namespace utils;
        using namespace capabilityAgents;
        using namespace aip;
        using namespace templateRuntime;
        using namespace system;
        using namespace capabilityAgents::externalMediaPlayer;
        using namespace certifiedSender;
        using namespace registrationManager;
        using namespace settings;
        using namespace settings::storage;
        using namespace mediaPlayer;
        class ExternalCapabilitiesBuilderInterface {
        public:
            struct Capability {
                Optional<CapabilityConfiguration> configuration;
                shared_ptr<DirectiveHandlerInterface> directiveHandler;
            };
            virtual ~ExternalCapabilitiesBuilderInterface() = default;
            virtual ExternalCapabilitiesBuilderInterface& withVisualFocusManager(shared_ptr<FocusManagerInterface> visualFocusManager) = 0;
            virtual ExternalCapabilitiesBuilderInterface& withSettingsStorage(shared_ptr<DeviceSettingStorageInterface> settingStorage) = 0;
            virtual ExternalCapabilitiesBuilderInterface& withTemplateRunTime(shared_ptr<TemplateRuntime> templateRuntime) = 0;
            virtual shared_ptr<CallManagerInterface> getCallManager() = 0;
            virtual ExternalCapabilitiesBuilderInterface& withInternetConnectionMonitor(shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor) = 0;
            virtual ExternalCapabilitiesBuilderInterface& withDialogUXStateAggregator(shared_ptr<DialogUXStateAggregator> dialogUXStateAggregator) = 0;
            virtual pair<list<Capability>, list<shared_ptr<RequiresShutdown>>> buildCapabilities(shared_ptr<ExternalMediaPlayer> externalMediaPlayer,
                                                                                                 shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                                                                                 shared_ptr<MessageSenderInterface> messageSender,
                                                                                                 shared_ptr<ExceptionEncounteredSender> exceptionSender,
                                                                                                 shared_ptr<CertifiedSender> certifiedSender,
                                                                                                 shared_ptr<FocusManagerInterface> audioFocusManager,
                                                                                                 shared_ptr<CustomerDataManager> dataManager,
                                                                                                 shared_ptr<ReportStateHandler> stateReportHandler,
                                                                                                 shared_ptr<AudioInputProcessor> audioInputProcessor,
                                                                                                 shared_ptr<SpeakerManagerInterface> speakerManager,
                                                                                                 shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                                                                 shared_ptr<UserInactivityMonitor> userInactivityMonitor,
                                                                                                 shared_ptr<ContextManagerInterface> contextManager,
                                                                                                 shared_ptr<AVSGatewayManagerInterface> avsGatewayManager,
                                                                                                 shared_ptr<MediaPlayerInterface> ringtoneMediaPlayer,
                                                                                                 shared_ptr<AudioFactoryInterface> audioFactory,
                                                                                                 shared_ptr<ChannelVolumeInterface> ringtoneChannelVolumeInterface,
                                                                                             #ifdef ENABLE_COMMS_AUDIO_PROXY
                                                                                                 shared_ptr<MediaPlayerInterface> commsMediaPlayer,
                                                                                                 shared_ptr<SpeakerInterface> commsSpeaker,
                                                                                                 shared_ptr<AudioInputStream> sharedDataStream,
                                                                                             #endif
                                                                                                 shared_ptr<PowerResourceManagerInterface> powerResourceManager) = 0;
        };
    }
}
#endif
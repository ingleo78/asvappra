#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_EXTERNALMEDIAPLAYER_INCLUDE_EXTERNALMEDIAPLAYER_EXTERNALMEDIAPLAYER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_EXTERNALMEDIAPLAYER_INCLUDE_EXTERNALMEDIAPLAYER_EXTERNALMEDIAPLAYER_H_

#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <capability_agents/ExternalMediaPlayer/AuthorizedSender.h>
#include <avs/CapabilityAgent.h>
#include <avs/DirectiveHandlerConfiguration.h>
#include <avs/NamespaceAndName.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ExternalMediaAdapterInterface.h>
#include <sdkinterfaces/ExternalMediaPlayerInterface.h>
#include <sdkinterfaces/ExternalMediaPlayerObserverInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/LocalPlaybackHandlerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/PlaybackHandlerInterface.h>
#include <sdkinterfaces/PlaybackRouterInterface.h>
#include <sdkinterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <sdkinterfaces/SpeakerManagerInterface.h>
#include <media_player/MediaPlayerInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <certified_sender/CertifiedSender.h>
#include <capability_agents/ExternalMediaPlayer/AuthorizedSender.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace externalMediaPlayer {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace configuration;
            using namespace mediaPlayer;
            using namespace metrics;
            using namespace threading;
            using namespace certifiedSender;
            using namespace rapidjson;
            using namespace sdkInterfaces::externalMediaPlayer;
            class ExternalMediaPlayer : public CapabilityAgent, public RequiresShutdown, public CapabilityConfigurationInterface,
                                        public ExternalMediaPlayerInterface, public MediaPropertiesInterface, public RenderPlayerInfoCardsProviderInterface,
                                        public PlaybackHandlerInterface, public LocalPlaybackHandlerInterface, public enable_shared_from_this<ExternalMediaPlayer> {
            public:
                using AdapterMediaPlayerMap = unordered_map<std::string, shared_ptr<MediaPlayerInterface>>;
                using AdapterSpeakerMap = unordered_map<std::string, shared_ptr<ChannelVolumeInterface>>;
                using AdapterCreateFunction = shared_ptr<ExternalMediaAdapterInterface> (*)(shared_ptr<MetricRecorderInterface>,
                                                                                            shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                                                            shared_ptr<ChannelVolumeInterface> speaker,
                                                                                            shared_ptr<SpeakerManagerInterface> speakerManager,
                                                                                            shared_ptr<MessageSenderInterface> messageSender,
                                                                                            shared_ptr<FocusManagerInterface> focusManager,
                                                                                            shared_ptr<ContextManagerInterface> contextManager,
                                                                                            shared_ptr<ExternalMediaPlayerInterface> externalMediaPlayer);
                using AdapterCreationMap = unordered_map<std::string, AdapterCreateFunction>;
                static constexpr const char* SPI_VERSION = "1.0";
                static shared_ptr<ExternalMediaPlayer> create(const AdapterMediaPlayerMap& mediaPlayers, const AdapterSpeakerMap& speakers,
                                                              const AdapterCreationMap& adapterCreationMap,
                                                              shared_ptr<SpeakerManagerInterface> speakerManager,
                                                              shared_ptr<MessageSenderInterface> messageSender,
                                                              shared_ptr<CertifiedSender> certifiedMessageSender,
                                                              shared_ptr<FocusManagerInterface> focusManager,
                                                              shared_ptr<ContextManagerInterface> contextManager,
                                                              shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                              shared_ptr<PlaybackRouterInterface> playbackRouter,
                                                              shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
                void onContextAvailable(const std::string& jsonContext);
                void onContextFailure(const ContextRequestError error) override;
                void provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                void onDeregistered() override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                virtual void onButtonPressed(PlaybackButton button) override;
                virtual void onTogglePressed(PlaybackToggle toggle, bool action) override;
                virtual void setPlayerInFocus(const std::string& playerInFocus) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
                void setObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer) override;
                bool localOperation(PlaybackOperation op) override;
                bool localSeekTo(milliseconds location, bool fromStart) override;
                milliseconds getAudioItemOffset() override;
                milliseconds getAudioItemDuration() override;
                void addObserver(const shared_ptr<ExternalMediaPlayerObserverInterface> observer);
                void removeObserver(const shared_ptr<ExternalMediaPlayerObserverInterface> observer);
                bool init(const AdapterMediaPlayerMap& mediaPlayers, const AdapterSpeakerMap& speakers, const AdapterCreationMap& adapterCreationMap,
                          shared_ptr<FocusManagerInterface> focusManager);
                map<std::string, shared_ptr<ExternalMediaAdapterInterface>> getAdaptersMap();
            private:
                ExternalMediaPlayer(shared_ptr<SpeakerManagerInterface> speakerManager, shared_ptr<MessageSenderInterface> messageSender,
                                    shared_ptr<CertifiedSender> certifiedMessageSender, shared_ptr<ContextManagerInterface> contextManager,
                                    shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender, shared_ptr<PlaybackRouterInterface> playbackRouter,
                                    shared_ptr<MetricRecorderInterface> metricRecorder);
                std::string provideSessionState();
                std::string providePlaybackState();
                bool parseDirectivePayload(shared_ptr<DirectiveInfo> info, Document* document);
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void doShutdown() override;
                void sendReportDiscoveredPlayersEvent();
                void sendAuthorizationCompleteEvent(const unordered_map<std::string, std::string>& authorized, const unordered_set<std::string>& deauthorized);
                void createAdapters(const AdapterMediaPlayerMap& mediaPlayers, const AdapterSpeakerMap& speakers,
                                    const AdapterCreationMap& adapterCreationMap, shared_ptr<MessageSenderInterface> messageSender,
                                    shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ContextManagerInterface> contextManager);
                void setHandlingCompleted(shared_ptr<DirectiveInfo> info);
                void sendExceptionEncounteredAndReportFailed(shared_ptr<DirectiveInfo> info, const std::string& message,
                                                             ExceptionErrorType type = ExceptionErrorType::INTERNAL_ERROR);
                void executeProvideState(const NamespaceAndName& stateProviderName, bool sendToken = false, unsigned int stateRequestToken = 0);
                shared_ptr<ExternalMediaAdapterInterface> preprocessDirective(shared_ptr<DirectiveInfo> info, Document* document);
                void handleAuthorizeDiscoveredPlayers(shared_ptr<DirectiveInfo> info, RequestType request);
                void handleLogin(shared_ptr<DirectiveInfo> info, RequestType request);
                void handleLogout(shared_ptr<DirectiveInfo> info, RequestType request);
                void handlePlay(shared_ptr<DirectiveInfo> info, RequestType request);
                void handlePlayControl(shared_ptr<DirectiveInfo> info, RequestType request);
                void handleSeek(shared_ptr<DirectiveInfo> info, RequestType request);
                void handleAdjustSeek(shared_ptr<DirectiveInfo> info, RequestType request);
                void notifyObservers(const std::string& playerId, const ObservableSessionProperties* sessionProperties);
                void notifyObservers(const std::string& playerId, const ObservablePlaybackStateProperties* playbackProperties);
                void notifyObservers(const std::string& playerId, const ObservableSessionProperties* sessionProperties,
                                     const ObservablePlaybackStateProperties* playbackProperties);
                shared_ptr<ExternalMediaAdapterInterface> getAdapterByPlayerId(const std::string& playerId);
                shared_ptr<ExternalMediaAdapterInterface> getAdapterByLocalPlayerId(const std::string& playerId);
                void notifyRenderPlayerInfoCardsObservers();
                std::string m_agentString;
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                shared_ptr<SpeakerManagerInterface> m_speakerManager;
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<CertifiedSender> m_certifiedMessageSender;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<PlaybackRouterInterface> m_playbackRouter;
                map<std::string, shared_ptr<ExternalMediaAdapterInterface>> m_adapters;
                mutex m_authorizedMutex;
                unordered_map<std::string, std::string> m_authorizedAdapters;
                std::string m_playerInFocus;
                shared_ptr<ExternalMediaAdapterInterface> m_adapterInFocus;
                mutex m_inFocusAdapterMutex;
                mutex m_adaptersMutex;
                shared_ptr<AuthorizedSender> m_authorizedSender;
                mutex m_observersMutex;
                unordered_set<shared_ptr<ExternalMediaPlayerObserverInterface>> m_observers;
                shared_ptr<RenderPlayerInfoCardsObserverInterface> m_renderPlayerObserver;
                queue<pair<std::string, std::string>> m_eventQueue;
                Executor m_executor;
                typedef void (ExternalMediaPlayer::*DirectiveHandler)(shared_ptr<DirectiveInfo> info, RequestType request);
                static unordered_map<NamespaceAndName, pair<RequestType, ExternalMediaPlayer::DirectiveHandler>> m_directiveToHandlerMap;
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
            };
        }
    }
}
#endif
#include <utility>
#include <vector>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/ExternalMediaPlayer/AdapterUtils.h>
#include <avs/ExternalMediaPlayer/ExternalMediaAdapterConstants.h>
#include <avs/SpeakerConstants/SpeakerConstants.h>
#include <json/JSONUtils.h>
#include <memory/Memory.h>
#include <util/Metrics.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/DataPointStringBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include <util/string/StringUtils.h>
#include "ExternalMediaPlayer.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace externalMediaPlayer {
            using namespace attachment;
            using namespace json;
            using namespace logger;
            using namespace avs::externalMediaPlayer;
            using namespace sdkInterfaces::externalMediaPlayer;
            using namespace utils::string;
            static const std::string TAG("ExternalMediaPlayer");
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            static const std::string EXTERNALMEDIAPLAYER_STATE_NAMESPACE = "ExternalMediaPlayer";
            static const std::string PLAYBACKSTATEREPORTER_STATE_NAMESPACE = "Alexa.PlaybackStateReporter";
            static const std::string EXTERNALMEDIAPLAYER_NAME = "ExternalMediaPlayerState";
            static const std::string PLAYBACKSTATEREPORTER_NAME = "playbackState";
            static const std::string EXTERNALMEDIAPLAYER_NAMESPACE = "ExternalMediaPlayer";
            static const std::string PLAYBACKCONTROLLER_NAMESPACE = "Alexa.PlaybackController";
            static const std::string PLAYLISTCONTROLLER_NAMESPACE = "Alexa.PlaylistController";
            static const std::string SEEKCONTROLLER_NAMESPACE = "Alexa.SeekController";
            static const std::string FAVORITESCONTROLLER_NAMESPACE = "Alexa.FavoritesController";
            static const std::string ALEXA_INTERFACE_TYPE = "AlexaInterface";
            static const std::string EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_TYPE = ALEXA_INTERFACE_TYPE;
            static const std::string EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_NAME = "ExternalMediaPlayer";
            static const std::string EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_VERSION = "1.2";
            static const std::string PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_NAME = PLAYBACKSTATEREPORTER_STATE_NAMESPACE;
            static const std::string PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_VERSION = "1.0";
            static const std::string PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_NAME = PLAYBACKCONTROLLER_NAMESPACE;
            static const std::string PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";
            static const std::string PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_NAME = PLAYLISTCONTROLLER_NAMESPACE;
            static const std::string PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";
            static const std::string SEEKCONTROLLER_CAPABILITY_INTERFACE_NAME = SEEKCONTROLLER_NAMESPACE;
            static const std::string SEEKCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";
            static const std::string FAVORITESCONTROLLER_CAPABILITY_INTERFACE_NAME = FAVORITESCONTROLLER_NAMESPACE;
            static const std::string FAVORITESCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";
            static const NamespaceAndName PLAY_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Play"};
            static const NamespaceAndName LOGIN_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Login"};
            static const NamespaceAndName LOGOUT_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Logout"};
            static const NamespaceAndName AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE,"AuthorizeDiscoveredPlayers"};
            static const NamespaceAndName RESUME_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Play"};
            static const NamespaceAndName PAUSE_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Pause"};
            static const NamespaceAndName STOP_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Stop"};
            static const NamespaceAndName NEXT_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Next"};
            static const NamespaceAndName PREVIOUS_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Previous"};
            static const NamespaceAndName STARTOVER_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "StartOver"};
            static const NamespaceAndName REWIND_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Rewind"};
            static const NamespaceAndName FASTFORWARD_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "FastForward"};
            static const NamespaceAndName ENABLEREPEATONE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableRepeatOne"};
            static const NamespaceAndName ENABLEREPEAT_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableRepeat"};
            static const NamespaceAndName DISABLEREPEAT_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "DisableRepeat"};
            static const NamespaceAndName ENABLESHUFFLE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableShuffle"};
            static const NamespaceAndName DISABLESHUFFLE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "DisableShuffle"};
            static const NamespaceAndName SEEK_DIRECTIVE{SEEKCONTROLLER_NAMESPACE, "SetSeekPosition"};
            static const NamespaceAndName ADJUSTSEEK_DIRECTIVE{SEEKCONTROLLER_NAMESPACE, "AdjustSeekPosition"};
            static const NamespaceAndName FAVORITE_DIRECTIVE{FAVORITESCONTROLLER_NAMESPACE, "Favorite"};
            static const NamespaceAndName UNFAVORITE_DIRECTIVE{FAVORITESCONTROLLER_NAMESPACE, "Unfavorite"};
            static const NamespaceAndName SESSION_STATE{EXTERNALMEDIAPLAYER_STATE_NAMESPACE, EXTERNALMEDIAPLAYER_NAME};
            static const NamespaceAndName PLAYBACK_STATE{PLAYBACKSTATEREPORTER_STATE_NAMESPACE, PLAYBACKSTATEREPORTER_NAME};
            static const char PLAYERS[] = "players";
            static const char PLAYER_IN_FOCUS[] = "playerInFocus";
            static const int64_t MAX_PAST_OFFSET = -86400000;
            static const int64_t MAX_FUTURE_OFFSET = 86400000;
            static const std::string EMP_CONFIG_KEY("externalMediaPlayer");
            static const char AGENT_KEY[] = "agent";
            static const std::string EMP_AGENT_KEY("agentString");
            static const char AUTHORIZED[] = "authorized";
            static const char DEAUTHORIZED[] = "deauthorized";
            static const char LOCAL_PLAYER_ID[] = "localPlayerId";
            static const char METADATA[] = "metadata";
            static const char SPI_VERSION_KEY[] = "spiVersion";
            static const char VALIDATION_METHOD[] = "validationMethod";
            static const char VALIDATION_DATA[] = "validationData";
            static const char REPORT_DISCOVERED_PLAYERS[] = "ReportDiscoveredPlayers";
            static const char AUTHORIZATION_COMPLETE[] = "AuthorizationComplete";
            static const std::string AUDIO_PLAYER_METRIC_PREFIX = "AUDIO_PLAYER-";
            static const std::string PLAY_DIRECTIVE_RECEIVED = "PLAY_DIRECTIVE_RECEIVED";
            static const std::string STOP_DIRECTIVE_RECEIVED = "STOP_DIRECTIVE_RECEIVED";
            static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const std::string metricName,
                                     const DataPoint& dataPoint, const std::string& msgId, const std::string& trackId,
                                     const std::string& playerId) {
                auto metricBuilder = MetricEventBuilder{}.setActivityName(AUDIO_PLAYER_METRIC_PREFIX + metricName).addDataPoint(dataPoint);
                if (msgId.size() > 0) {
                    metricBuilder.addDataPoint(DataPointStringBuilder{}.setName("DIRECTIVE_MESSAGE_ID").setValue(msgId).build());
                }
                if (trackId.size() > 0) {
                    metricBuilder.addDataPoint(DataPointStringBuilder{}.setName("TRACK_ID").setValue(trackId).build());
                }
                if (playerId.size() > 0) {
                    metricBuilder.addDataPoint(DataPointStringBuilder{}.setName("PLAYER_ID").setValue(playerId).build());
                }
                auto metricEvent = metricBuilder.build();
                if (metricEvent == nullptr) {
                    ACSDK_ERROR(LX("Error creating metric."));
                    return;
                }
                recordMetric(metricRecorder, metricEvent);
            }
            static shared_ptr<CapabilityConfiguration> getExternalMediaPlayerCapabilityConfiguration();
            unordered_map<NamespaceAndName, pair<RequestType, ExternalMediaPlayer::DirectiveHandler>> ExternalMediaPlayer::m_directiveToHandlerMap = {
                    {AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE,
                     std::make_pair(RequestType::NONE, &ExternalMediaPlayer::handleAuthorizeDiscoveredPlayers)},
                    {LOGIN_DIRECTIVE, make_pair(RequestType::LOGIN, &ExternalMediaPlayer::handleLogin)},
                    {LOGOUT_DIRECTIVE, make_pair(RequestType::LOGOUT, &ExternalMediaPlayer::handleLogout)},
                    {PLAY_DIRECTIVE, make_pair(RequestType::PLAY, &ExternalMediaPlayer::handlePlay)},
                    {PAUSE_DIRECTIVE, make_pair(RequestType::PAUSE, &ExternalMediaPlayer::handlePlayControl)},
                    {STOP_DIRECTIVE, make_pair(RequestType::STOP, &ExternalMediaPlayer::handlePlayControl)},
                    {RESUME_DIRECTIVE, make_pair(RequestType::RESUME, &ExternalMediaPlayer::handlePlayControl)},
                    {NEXT_DIRECTIVE, make_pair(RequestType::NEXT, &ExternalMediaPlayer::handlePlayControl)},
                    {PREVIOUS_DIRECTIVE, make_pair(RequestType::PREVIOUS, &ExternalMediaPlayer::handlePlayControl)},
                    {STARTOVER_DIRECTIVE, make_pair(RequestType::START_OVER, &ExternalMediaPlayer::handlePlayControl)},
                    {FASTFORWARD_DIRECTIVE, make_pair(RequestType::FAST_FORWARD, &ExternalMediaPlayer::handlePlayControl)},
                    {REWIND_DIRECTIVE, make_pair(RequestType::REWIND, &ExternalMediaPlayer::handlePlayControl)},
                    {ENABLEREPEATONE_DIRECTIVE, make_pair(RequestType::ENABLE_REPEAT_ONE, &ExternalMediaPlayer::handlePlayControl)},
                    {ENABLEREPEAT_DIRECTIVE, make_pair(RequestType::ENABLE_REPEAT, &ExternalMediaPlayer::handlePlayControl)},
                    {DISABLEREPEAT_DIRECTIVE, make_pair(RequestType::DISABLE_REPEAT, &ExternalMediaPlayer::handlePlayControl)},
                    {ENABLESHUFFLE_DIRECTIVE, make_pair(RequestType::ENABLE_SHUFFLE, &ExternalMediaPlayer::handlePlayControl)},
                    {DISABLESHUFFLE_DIRECTIVE, make_pair(RequestType::DISABLE_SHUFFLE, &ExternalMediaPlayer::handlePlayControl)},
                    {FAVORITE_DIRECTIVE, make_pair(RequestType::FAVORITE, &ExternalMediaPlayer::handlePlayControl)},
                    {UNFAVORITE_DIRECTIVE, make_pair(RequestType::UNFAVORITE, &ExternalMediaPlayer::handlePlayControl)},
                    {SEEK_DIRECTIVE, make_pair(RequestType::SEEK, &ExternalMediaPlayer::handleSeek)},
                    {ADJUSTSEEK_DIRECTIVE, make_pair(RequestType::ADJUST_SEEK, &ExternalMediaPlayer::handleAdjustSeek)}
                };
            auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
            auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
            static DirectiveHandlerConfiguration g_configuration = {{AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {PLAY_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {LOGIN_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {LOGOUT_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {RESUME_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {PAUSE_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {STOP_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {NEXT_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {PREVIOUS_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {STARTOVER_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {REWIND_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {FASTFORWARD_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {ENABLEREPEATONE_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {ENABLEREPEAT_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {DISABLEREPEAT_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {ENABLESHUFFLE_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {DISABLESHUFFLE_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {SEEK_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {ADJUSTSEEK_DIRECTIVE, audioNonBlockingPolicy},
                                                                    {FAVORITE_DIRECTIVE, neitherNonBlockingPolicy},
                                                                    {UNFAVORITE_DIRECTIVE, neitherNonBlockingPolicy}};
            static unordered_map<PlaybackButton, RequestType> g_buttonToRequestType = {
                {PlaybackButton::PLAY, RequestType::PAUSE_RESUME_TOGGLE},
                {PlaybackButton::PAUSE, RequestType::PAUSE_RESUME_TOGGLE},
                {PlaybackButton::NEXT, RequestType::NEXT},
                {PlaybackButton::PREVIOUS, RequestType::PREVIOUS}
            };
            static std::unordered_map<PlaybackToggle, std::pair<RequestType, RequestType>> g_toggleToRequestType = {
                {PlaybackToggle::SHUFFLE, make_pair(RequestType::ENABLE_SHUFFLE, RequestType::DISABLE_SHUFFLE)},
                {PlaybackToggle::LOOP, make_pair(RequestType::ENABLE_REPEAT, RequestType::DISABLE_REPEAT)},
                {PlaybackToggle::REPEAT, make_pair(RequestType::ENABLE_REPEAT_ONE, RequestType::DISABLE_REPEAT_ONE)},
                {PlaybackToggle::THUMBS_UP, make_pair(RequestType::FAVORITE, RequestType::DESELECT_FAVORITE)},
                {PlaybackToggle::THUMBS_DOWN, make_pair(RequestType::UNFAVORITE, RequestType::DESELECT_UNFAVORITE)}
            };
            static shared_ptr<avs::CapabilityConfiguration> generateCapabilityConfiguration(const std::string& type, const std::string& interfaceName,
                                                                                       const std::string& version) {
                unordered_map<std::string, std::string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, type});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, interfaceName});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, version});
                return make_shared<CapabilityConfiguration>(configMap);
            }
            shared_ptr<ExternalMediaPlayer> ExternalMediaPlayer::create(const AdapterMediaPlayerMap& mediaPlayers, const AdapterSpeakerMap& speakers,
                                                                        const AdapterCreationMap& adapterCreationMap,
                                                                        shared_ptr<SpeakerManagerInterface> speakerManager,
                                                                        shared_ptr<MessageSenderInterface> messageSender,
                                                                        shared_ptr<certifiedSender::CertifiedSender> certifiedMessageSender,
                                                                        shared_ptr<FocusManagerInterface> focusManager,
                                                                        shared_ptr<ContextManagerInterface> contextManager,
                                                                        shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                        shared_ptr<PlaybackRouterInterface> playbackRouter,
                                                                        shared_ptr<MetricRecorderInterface> metricRecorder) {
                if (nullptr == speakerManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullSpeakerManager"));
                    return nullptr;
                }
                if (nullptr == messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                }
                if (nullptr == certifiedMessageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullCertifiedMessageSender"));
                    return nullptr;
                }
                if (nullptr == focusManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullFocusManager"));
                    return nullptr;
                }
                if (nullptr == contextManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                    return nullptr;
                }
                if (nullptr == exceptionSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
                    return nullptr;
                }
                if (nullptr == playbackRouter) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullPlaybackRouter"));
                    return nullptr;
                }
                auto externalMediaPlayer = shared_ptr<ExternalMediaPlayer>(new ExternalMediaPlayer(move(speakerManager), move(messageSender),
                                                                           move(certifiedMessageSender), move(contextManager),
                                                                           move(exceptionSender), move(playbackRouter), move(metricRecorder)));
                if (!externalMediaPlayer->init(mediaPlayers, speakers, adapterCreationMap, move(focusManager))) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "initFailed"));
                    return nullptr;
                }
                return externalMediaPlayer;
            }
            ExternalMediaPlayer::ExternalMediaPlayer(shared_ptr<SpeakerManagerInterface> speakerManager, shared_ptr<MessageSenderInterface> messageSender,
                                                     shared_ptr<CertifiedSender> certifiedMessageSender, shared_ptr<ContextManagerInterface> contextManager,
                                                     shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                     shared_ptr<PlaybackRouterInterface> playbackRouter, shared_ptr<MetricRecorderInterface> metricRecorder) :
                                                     CapabilityAgent{EXTERNALMEDIAPLAYER_NAMESPACE, move(exceptionSender)},
                                                     RequiresShutdown{"ExternalMediaPlayer"}, m_metricRecorder{move(metricRecorder)},
                                                     m_speakerManager{move(speakerManager)}, m_messageSender{move(messageSender)},
                                                     m_certifiedMessageSender{move(certifiedMessageSender)}, m_contextManager{move(contextManager)},
                                                     m_playbackRouter{move(playbackRouter)} {
                m_capabilityConfigurations.insert(getExternalMediaPlayerCapabilityConfiguration());
                m_capabilityConfigurations.insert(generateCapabilityConfiguration(ALEXA_INTERFACE_TYPE, PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_NAME,
                                                  PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_VERSION));
                m_capabilityConfigurations.insert(generateCapabilityConfiguration(ALEXA_INTERFACE_TYPE, PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_NAME,
                                                  PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_VERSION));
                m_capabilityConfigurations.insert(generateCapabilityConfiguration(ALEXA_INTERFACE_TYPE, PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_NAME,
                                                  PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_VERSION));
                m_capabilityConfigurations.insert(generateCapabilityConfiguration(ALEXA_INTERFACE_TYPE, SEEKCONTROLLER_CAPABILITY_INTERFACE_NAME,
                                                                                  SEEKCONTROLLER_CAPABILITY_INTERFACE_VERSION));
                m_capabilityConfigurations.insert(generateCapabilityConfiguration(ALEXA_INTERFACE_TYPE, FAVORITESCONTROLLER_CAPABILITY_INTERFACE_NAME,
                                                                                  FAVORITESCONTROLLER_CAPABILITY_INTERFACE_VERSION));
            }
            bool ExternalMediaPlayer::init(const AdapterMediaPlayerMap& mediaPlayers, const AdapterSpeakerMap& speakers,
                                           const AdapterCreationMap& adapterCreationMap, shared_ptr<FocusManagerInterface> focusManager) {
                ACSDK_DEBUG5(LX(__func__));
                m_authorizedSender = AuthorizedSender::create(m_messageSender);
                if (!m_authorizedSender) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "createAuthorizedSenderFailed"));
                    return false;
                }
                shared_ptr<StateProviderInterface> spi = make_shared<StateProviderInterface>();
                m_contextManager->setStateProvider(SESSION_STATE, spi);
                m_contextManager->setStateProvider(PLAYBACK_STATE, spi);
                auto config = ConfigurationNode::getRoot();
                auto empGroup = config[EMP_CONFIG_KEY];
                empGroup.getString(EMP_AGENT_KEY, &m_agentString, "");
                MessageSenderInterface msi = reinterpret_cast<MessageSenderInterface&>(m_authorizedSender);
                shared_ptr<MessageSenderInterface> smsi = make_shared<MessageSenderInterface>(msi);
                createAdapters(mediaPlayers, speakers, adapterCreationMap, smsi, focusManager, m_contextManager);
                return true;
            }
            shared_ptr<CapabilityConfiguration> getExternalMediaPlayerCapabilityConfiguration() {
                return generateCapabilityConfiguration(EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_TYPE, EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_NAME,
                                                       EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_VERSION);
            }
            void ExternalMediaPlayer::onContextAvailable(const std::string& jsonContext) {
                m_executor.submit([this, jsonContext] {
                    ACSDK_DEBUG5(LX("onContextAvailableLambda"));
                    while(!m_eventQueue.empty()) {
                        pair<std::string, std::string> nameAndPayload = m_eventQueue.front();
                        m_eventQueue.pop();
                        auto event = buildJsonEventString(nameAndPayload.first, "", nameAndPayload.second, jsonContext);
                        ACSDK_DEBUG5(LX("onContextAvailableLambda").d("event", event.second));
                        auto request = make_shared<MessageRequest>(event.second);
                        m_messageSender->sendMessage(request);
                    }
                });
            }
            void ExternalMediaPlayer::onContextFailure(const ContextRequestError error) {
                pair<std::string, std::string> nameAndPayload = m_eventQueue.front();
                m_eventQueue.pop();
                ACSDK_ERROR(LX(__func__).d("error", error).d("eventName", nameAndPayload.first).sensitive("payload", nameAndPayload.second));
            }
            void ExternalMediaPlayer::provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) {
                m_executor.submit([this, stateProviderName, stateRequestToken] {
                    executeProvideState(stateProviderName, true, stateRequestToken);
                });
            }
            void ExternalMediaPlayer::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void ExternalMediaPlayer::preHandleDirective(shared_ptr<DirectiveInfo> info) {}
            bool ExternalMediaPlayer::parseDirectivePayload(shared_ptr<DirectiveInfo> info, Document* document) {
                ParseResult result = document->Parse(info->directive->getPayload().data());
                if (result) return true;
                ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", GetParseError_En(result.Code()))
                    .d("offset", result.Offset()).d("messageId", info->directive->getMessageId()));
                sendExceptionEncounteredAndReportFailed(info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                return false;
            }
            void ExternalMediaPlayer::handleDirective(shared_ptr<DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                NamespaceAndName directiveNamespaceAndName(info->directive->getNamespace(), info->directive->getName());
                auto handlerIt = m_directiveToHandlerMap.find(directiveNamespaceAndName);
                if (handlerIt == m_directiveToHandlerMap.end()) {
                    ACSDK_ERROR(LX("handleDirectivesFailed").d("reason", "noDirectiveHandlerForDirective")
                        .d("nameSpace", info->directive->getNamespace()).d("name", info->directive->getName()));
                    sendExceptionEncounteredAndReportFailed(info, "Unhandled directive", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return;
                }
                ACSDK_DEBUG9(LX("handleDirectivesPayload").sensitive("Payload", info->directive->getPayload()));
                auto handler = (handlerIt->second.second);
                (this->*handler)(info, handlerIt->second.first);
            }
            shared_ptr<ExternalMediaAdapterInterface> ExternalMediaPlayer::preprocessDirective(shared_ptr<DirectiveInfo> info, Document* document) {
                ACSDK_DEBUG9(LX("preprocessDirective"));
                if (!parseDirectivePayload(info, document)) return nullptr;
                std::string playerId;
                if (!jsonUtils::retrieveValue(*document, PLAYER_ID, &playerId)) {
                    ACSDK_ERROR(LX("preprocessDirectiveFailed").d("reason", "nullPlayerId"));
                    sendExceptionEncounteredAndReportFailed(info, "No PlayerId in directive.");
                    return nullptr;
                }
                auto adapter = getAdapterByPlayerId(playerId);
                if (!adapter) {
                    ACSDK_ERROR(LX("preprocessDirectiveFailed").d("reason", "nullAdapter").d(PLAYER_ID, playerId));
                    sendExceptionEncounteredAndReportFailed(info, "nullAdapter.");
                    return nullptr;
                }
                return adapter;
            }
            void ExternalMediaPlayer::handleAuthorizeDiscoveredPlayers(shared_ptr<DirectiveInfo> info, RequestType request) {
                ACSDK_DEBUG5(LX(__func__));
                Document payload;
                unordered_map<std::string, std::string> authorizedForJson;
                unordered_map<std::string, std::string> newAuthorizedAdapters;
                unordered_set<std::string> newAuthorizedAdaptersKeys;
                unordered_set<std::string> deauthorizedLocal;
                if (!parseDirectivePayload(info, &payload)) return;
                bool parseAllSucceeded = true;
                Value::ConstMemberIterator playersIt;
                Value _payload{payload.GetString(), strlen(payload.GetString())};
                if (jsonUtils::findNode(_payload, PLAYERS, &playersIt)) {
                    for (Value::ConstValueIterator playerIt = playersIt->value.Begin();
                         playerIt != playersIt->value.End();
                         playerIt++) {
                        bool authorized = false;
                        std::string localPlayerId, playerId, defaultSkillToken;
                        if (!(*playerIt).IsObject()) {
                            ACSDK_ERROR(LX("handleAuthorizeDiscoveredPlayersFailed").d("reason", "unexpectedFormat"));
                            parseAllSucceeded = false;
                            continue;
                        }
                        if (!jsonUtils::retrieveValue(*playerIt, LOCAL_PLAYER_ID, &localPlayerId)) {
                            ACSDK_ERROR(LX("handleAuthorizeDiscoveredPlayersFailed").d("reason", "missingAttribute")
                                .d("attribute", LOCAL_PLAYER_ID));
                            parseAllSucceeded = false;
                            continue;
                        }
                        if (!json::jsonUtils::retrieveValue(*playerIt, AUTHORIZED, &authorized)) {
                            ACSDK_ERROR(LX("handleAuthorizeDiscoveredPlayersFailed").d("reason", "missingAttribute")
                                .d("attribute", AUTHORIZED));
                            parseAllSucceeded = false;
                            continue;
                        }
                        if (authorized) {
                            Value::ConstMemberIterator metadataIt;
                            if (!jsonUtils::findNode(*playerIt, METADATA, &metadataIt)) {
                                ACSDK_ERROR(LX("handleAuthorizeDiscoveredPlayersFailed").d("reason", "missingAttribute")
                                    .d("attribute", METADATA));
                                parseAllSucceeded = false;
                                continue;
                            }
                            if (!jsonUtils::retrieveValue(metadataIt->value, PLAYER_ID, &playerId)) {
                                ACSDK_ERROR(LX("handleAuthorizeDiscoveredPlayersFailed").d("reason", "missingAttribute")
                                    .d("attribute", PLAYER_ID));
                                parseAllSucceeded = false;
                                continue;
                            }
                            if (!jsonUtils::retrieveValue(metadataIt->value, SKILL_TOKEN, &defaultSkillToken)) {
                                ACSDK_ERROR(LX("handleAuthorizeDiscoveredPlayersFailed").d("reason", "missingAttribute")
                                    .d("attribute", SKILL_TOKEN));
                                parseAllSucceeded = false;
                                continue;
                            }
                        }
                        ACSDK_DEBUG(LX(__func__).d("localPlayerId", localPlayerId).d("authorized", authorized).d("playerId", playerId)
                            .d("defaultSkillToken", defaultSkillToken));
                        auto it = m_adapters.find(localPlayerId);
                        if (m_adapters.end() != it) {
                            m_executor.submit([it, localPlayerId, authorized, playerId, defaultSkillToken]() {
                                it->second->handleAuthorized(authorized, playerId, defaultSkillToken);
                            });
                            if (authorized) {
                                if (newAuthorizedAdapters.count(playerId) > 0) {
                                    ACSDK_WARN(LX("duplicatePlayerIdFound").d("playerId", playerId).d("priorSkillToken", authorizedForJson[playerId])
                                        .d("newSkillToken", defaultSkillToken).m("Overwriting prior entry"));
                                }
                                authorizedForJson[playerId] = defaultSkillToken;
                                newAuthorizedAdapters[playerId] = localPlayerId;
                                newAuthorizedAdaptersKeys.insert(playerId);
                            }
                        } else {
                            ACSDK_ERROR(LX("handleAuthorizeDiscoveredPlayersFailed").d("reason", "adapterNotFound"));
                            parseAllSucceeded = false;
                        }
                    }
                }
                if (!parseAllSucceeded) {
                    sendExceptionEncounteredAndReportFailed(info, "One or more player was not successfuly parsed", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                } else setHandlingCompleted(info);
                {
                    lock_guard<mutex> lock(m_authorizedMutex);
                    for (const auto& idToLocalId : m_authorizedAdapters) {
                        if (newAuthorizedAdapters.count(idToLocalId.first) == 0) {
                            deauthorizedLocal.insert(idToLocalId.second);
                            const auto& adapter = getAdapterByLocalPlayerId(idToLocalId.second);
                            if (adapter) adapter->handleAuthorized(false, "", "");
                        }
                    }
                    m_authorizedAdapters = newAuthorizedAdapters;
                }
                m_authorizedSender->updateAuthorizedPlayers(newAuthorizedAdaptersKeys);
                m_executor.submit([this, authorizedForJson, deauthorizedLocal]() {
                    sendAuthorizationCompleteEvent(authorizedForJson, deauthorizedLocal);
                });
            }
            map<std::string, std::shared_ptr<avsCommon::sdkInterfaces::externalMediaPlayer::ExternalMediaAdapterInterface>>
            ExternalMediaPlayer::getAdaptersMap() {
                return m_adapters;
            }
            shared_ptr<ExternalMediaAdapterInterface> ExternalMediaPlayer::getAdapterByPlayerId(const std::string& playerId) {
                ACSDK_DEBUG5(LX(__func__));
                std::string localPlayerId;
                {
                    auto lock = unique_lock<mutex>(m_authorizedMutex);
                    auto playerIdToLocalPlayerId = m_authorizedAdapters.find(playerId);
                    if (m_authorizedAdapters.end() == playerIdToLocalPlayerId) {
                        ACSDK_ERROR(LX("getAdapterByPlayerIdFailed").d("reason", "noMatchingLocalId").d(PLAYER_ID, playerId));
                        return nullptr;
                    } else localPlayerId = playerIdToLocalPlayerId->second;
                }
                return getAdapterByLocalPlayerId(localPlayerId);
            }
            std::shared_ptr<ExternalMediaAdapterInterface> ExternalMediaPlayer::getAdapterByLocalPlayerId(
                const std::string& localPlayerId) {
                ACSDK_DEBUG5(LX(__func__));
                auto lock = unique_lock<mutex>(m_adaptersMutex);
                if (localPlayerId.empty()) return nullptr;
                auto localPlayerIdToAdapter = m_adapters.find(localPlayerId);
                if (m_adapters.end() == localPlayerIdToAdapter) {
                    ACSDK_ERROR(LX("getAdapterByLocalPlayerIdFailed").d("reason", "noAdapterForLocalId").d("localPlayerId", localPlayerId));
                    return nullptr;
                }
                return localPlayerIdToAdapter->second;
            }
            void ExternalMediaPlayer::sendAuthorizationCompleteEvent(const unordered_map<std::string, std::string>& authorized,
                                                                     const unordered_set<std::string>& deauthorized) {
                ACSDK_DEBUG5(LX(__func__));
                Document payload(kObjectType);
                Value authorizedJson(kArrayType);
                Value deauthorizedJson(kArrayType);
                for (const auto& playerIdToSkillToken : authorized) {
                    Value player(kObjectType);
                    Value player_id{PLAYER_ID, strlen(PLAYER_ID)};
                    Value skill_token{SKILL_TOKEN, strlen(SKILL_TOKEN)};
                    Value first{playerIdToSkillToken.first.data(), playerIdToSkillToken.first.length()};
                    Value second{playerIdToSkillToken.second.data(), playerIdToSkillToken.second.length()};
                    player.AddMember(player_id, first);
                    player.AddMember(skill_token, second);
                    authorizedJson.PushBack(player);
                }
                for (const auto& localPlayerId : deauthorized) {
                    Value player(kObjectType);
                    Value local_player_id{LOCAL_PLAYER_ID, strlen(LOCAL_PLAYER_ID)};
                    Value _localPlayerId{localPlayerId.data(), localPlayerId.length()};
                    player.AddMember(local_player_id, _localPlayerId);
                    deauthorizedJson.PushBack(player);
                }
                payload.AddMember(AUTHORIZED, authorizedJson, payload.GetAllocator());
                payload.AddMember(DEAUTHORIZED, deauthorizedJson, payload.GetAllocator());
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                if (!payload.Accept(writer)) {
                    ACSDK_ERROR(LX("sendAuthorizationCompleteEventFailed").d("reason", "writerRefusedJsonObject"));
                    return;
                }
                m_eventQueue.push(make_pair(AUTHORIZATION_COMPLETE, buffer.GetString()));
                shared_ptr<ContextRequesterInterface> cri = make_shared<ContextRequesterInterface>();
                m_contextManager->getContext(cri);
            }
            void ExternalMediaPlayer::handleLogin(shared_ptr<DirectiveInfo> info, RequestType request) {
                Document payload;
                auto adapter = preprocessDirective(info, &payload);
                if (!adapter) return;
                std::string accessToken;
                if (!jsonUtils::retrieveValue(payload, "accessToken", &accessToken)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullAccessToken"));
                    sendExceptionEncounteredAndReportFailed(info, "missing accessToken in Login directive");
                    return;
                }
                std::string userName;
                if (!jsonUtils::retrieveValue(payload, USERNAME, &userName)) userName = "";
                int64_t refreshInterval;
                if (!jsonUtils::retrieveValue(payload, "tokenRefreshIntervalInMilliseconds", &refreshInterval)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullRefreshInterval"));
                    sendExceptionEncounteredAndReportFailed(info, "missing tokenRefreshIntervalInMilliseconds in Login directive");
                    return;
                }
                bool forceLogin;
                if (!jsonUtils::retrieveValue(payload, "forceLogin", &forceLogin)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullForceLogin"));
                    sendExceptionEncounteredAndReportFailed(info, "missing forceLogin in Login directive");
                    return;
                }
                setHandlingCompleted(info);
                adapter->handleLogin(accessToken, userName, forceLogin, std::chrono::milliseconds(refreshInterval));
            }
            void ExternalMediaPlayer::handleLogout(shared_ptr<DirectiveInfo> info, RequestType request) {
                Document payload;
                auto adapter = preprocessDirective(info, &payload);
                if (!adapter) return;
                setHandlingCompleted(info);
                adapter->handleLogout();
            }
            void ExternalMediaPlayer::handlePlay(shared_ptr<DirectiveInfo> info, RequestType request) {
                Document payload;
                auto adapter = preprocessDirective(info, &payload);
                if (!adapter) return;
                std::string playbackContextToken;
                if (!jsonUtils::retrieveValue(payload, "playbackContextToken", &playbackContextToken)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullPlaybackContextToken"));
                    sendExceptionEncounteredAndReportFailed(info, "missing playbackContextToken in Play directive");
                    return;
                }
                int64_t offset;
                if (!jsonUtils::retrieveValue(payload, "offsetInMilliseconds", &offset)) offset = 0;
                int64_t index;
                if (!jsonUtils::retrieveValue(payload, "index", &index)) index = 0;
                std::string skillToken;
                if (!jsonUtils::retrieveValue(payload, "skillToken", &skillToken)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullSkillToken"));
                    sendExceptionEncounteredAndReportFailed(info, "missing skillToken in Play directive");
                    return;
                }
                std::string playbackSessionId;
                if (!jsonUtils::retrieveValue(payload, "playbackSessionId", &playbackSessionId)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullPlaybackSessionId"));
                    sendExceptionEncounteredAndReportFailed(info, "missing playbackSessionId in Play directive");
                    return;
                }
                std::string navigation;
                if (!jsonUtils::retrieveValue(payload, "navigation", &navigation)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullNavigation"));
                    sendExceptionEncounteredAndReportFailed(info, "missing navigation in Play directive");
                    return;
                }
                bool preload;
                if (!jsonUtils::retrieveValue(payload, "preload", &preload)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullPreload"));
                    sendExceptionEncounteredAndReportFailed(info, "missing preload in Play directive");
                    return;
                }
                rapidjson::Value::ConstMemberIterator playRequestorJson;
                PlayRequestor playRequestor;
                Value _payload{payload.GetString(), strlen(payload.GetString())};
                if (jsonUtils::findNode(_payload, "playRequestor", &playRequestorJson)) {
                    if (!jsonUtils::retrieveValue(playRequestorJson->value, "type", &playRequestor.type)) {
                        ACSDK_ERROR(LX("handlePlayDirectiveFailed").d("reason", "missingPlayRequestorType")
                            .d("messageId", info->directive->getMessageId()));
                        sendExceptionEncounteredAndReportFailed(info, "missing playRequestor type in Play directive");
                        return;
                    }
                    if (!jsonUtils::retrieveValue(playRequestorJson->value, "id", &playRequestor.id)) {
                        ACSDK_ERROR(LX("handlePlayDirectiveFailed").d("reason", "missingPlayRequestorId")
                            .d("messageId", info->directive->getMessageId()));
                        sendExceptionEncounteredAndReportFailed(info, "missing playRequestor id in Play directive");
                        return;
                    }
                }
                auto messageId = info->directive->getMessageId();
                submitMetric(m_metricRecorder, PLAY_DIRECTIVE_RECEIVED,DataPointCounterBuilder{}.setName(PLAY_DIRECTIVE_RECEIVED)
                             .increment(1).build(), messageId, playbackSessionId, adapter->name());
                setHandlingCompleted(info);
                adapter->handlePlay(playbackContextToken, index, milliseconds(offset), skillToken, playbackSessionId, navigation,
                                    preload, playRequestor);
            }
            void ExternalMediaPlayer::handleSeek(shared_ptr<DirectiveInfo> info, RequestType request) {
                Document payload;
                auto adapter = preprocessDirective(info, &payload);
                if (!adapter) return;
                int64_t position;
                if (!jsonUtils::retrieveValue(payload, POSITIONINMS, &position)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullPosition"));
                    sendExceptionEncounteredAndReportFailed(info, "missing positionMilliseconds in SetSeekPosition directive");
                    return;
                }
                setHandlingCompleted(info);
                adapter->handleSeek(std::chrono::milliseconds(position));
            }
            void ExternalMediaPlayer::handleAdjustSeek(shared_ptr<DirectiveInfo> info, RequestType request) {
                Document payload;
                auto adapter = preprocessDirective(info, &payload);
                if (!adapter) return;
                int64_t deltaPosition;
                if (!jsonUtils::retrieveValue(payload, "deltaPositionMilliseconds", &deltaPosition)) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDeltaPositionMilliseconds"));
                    sendExceptionEncounteredAndReportFailed(info, "missing deltaPositionMilliseconds in AdjustSeekPosition directive");
                    return;
                }
                if (deltaPosition < MAX_PAST_OFFSET || deltaPosition > MAX_FUTURE_OFFSET) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "deltaPositionMillisecondsOutOfRange."));
                    sendExceptionEncounteredAndReportFailed(info, "missing deltaPositionMilliseconds in AdjustSeekPosition directive");
                    return;
                }
                setHandlingCompleted(info);
                adapter->handleAdjustSeek(milliseconds(deltaPosition));
            }
            void ExternalMediaPlayer::handlePlayControl(shared_ptr<DirectiveInfo> info, RequestType request) {
                Document payload;
                auto adapter = preprocessDirective(info, &payload);
                if (!adapter) return;
                if (STOP_DIRECTIVE.name == info->directive->getName() || PAUSE_DIRECTIVE.name == info->directive->getName()) {
                    auto messageId = info->directive->getMessageId();
                    submitMetric(m_metricRecorder, STOP_DIRECTIVE_RECEIVED,DataPointCounterBuilder{}.setName(STOP_DIRECTIVE_RECEIVED)
                                 .increment(1).build(), messageId,"", adapter->name());
                }
                setHandlingCompleted(info);
                adapter->handlePlayControl(request);
            }
            void ExternalMediaPlayer::cancelDirective(shared_ptr<DirectiveInfo> info) {
                removeDirective(info);
            }
            void ExternalMediaPlayer::onDeregistered() {}
            DirectiveHandlerConfiguration ExternalMediaPlayer::getConfiguration() const {
                return g_configuration;
            }
            void ExternalMediaPlayer::setObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_observersMutex};
                m_renderPlayerObserver = observer;
            }
            bool ExternalMediaPlayer::localOperation(PlaybackOperation op) {
                ACSDK_DEBUG5(LX(__func__));
                std::string playerInFocus;
                {
                    lock_guard<mutex> lock{m_inFocusAdapterMutex};
                    playerInFocus = m_playerInFocus;
                }
                std::string localPlayerId;
                if (!playerInFocus.empty()) {
                    auto lock = unique_lock<std::mutex>(m_authorizedMutex);
                    auto playerIdToLocalPlayerId = m_authorizedAdapters.find(playerInFocus);
                    if (m_authorizedAdapters.end() == playerIdToLocalPlayerId) {
                        ACSDK_ERROR(LX("stopPlaybackFailed").d("reason", "noMatchingLocalId").d(PLAYER_ID, m_playerInFocus));
                        return false;
                    }
                    localPlayerId = playerIdToLocalPlayerId->second;
                    lock.unlock();
                    auto adapter = getAdapterByLocalPlayerId(localPlayerId);
                    if (!adapter) {
                        ACSDK_ERROR(LX("AdapterNotFound").d("player", localPlayerId));
                        return false;
                    }
                    switch(op) {
                        case PlaybackOperation::STOP_PLAYBACK: adapter->handlePlayControl(RequestType::STOP); break;
                        case PlaybackOperation::PAUSE_PLAYBACK: adapter->handlePlayControl(RequestType::PAUSE); break;
                        case PlaybackOperation::RESUME_PLAYBACK: adapter->handlePlayControl(RequestType::RESUME); break;
                    }
                    return true;
                }
                return false;
            }
            bool ExternalMediaPlayer::localSeekTo(milliseconds location, bool fromStart) {
                ACSDK_DEBUG5(LX(__func__));
                std::string playerInFocus;
                {
                    lock_guard<mutex> lock{m_inFocusAdapterMutex};
                    playerInFocus = m_playerInFocus;
                }
                std::string localPlayerId;
                if (!playerInFocus.empty()) {
                    auto lock = unique_lock<mutex>(m_authorizedMutex);
                    auto playerIdToLocalPlayerId = m_authorizedAdapters.find(playerInFocus);
                    if (m_authorizedAdapters.end() == playerIdToLocalPlayerId) {
                        ACSDK_ERROR(LX("stopPlaybackFailed").d("reason", "noMatchingLocalId").d(PLAYER_ID, m_playerInFocus));
                        return false;
                    }
                    localPlayerId = playerIdToLocalPlayerId->second;
                    lock.unlock();
                    auto adapter = getAdapterByLocalPlayerId(localPlayerId);
                    if (!adapter) {
                        ACSDK_ERROR(LX("AdapterNotFound").d("player", localPlayerId));
                        return false;
                    }
                    if (fromStart) adapter->handleSeek(location);
                    else adapter->handleAdjustSeek(location);
                    return true;
                }
                return false;
            }
            milliseconds ExternalMediaPlayer::getAudioItemOffset() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_inFocusAdapterMutex};
                if (!m_adapterInFocus) {
                    ACSDK_ERROR(LX("getAudioItemOffsetFailed").d("reason", "NoActiveAdapter").d("player", m_playerInFocus));
                    return milliseconds::zero();
                }
                return m_adapterInFocus->getOffset();
            }
            milliseconds ExternalMediaPlayer::getAudioItemDuration() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_inFocusAdapterMutex};
                if (!m_adapterInFocus) {
                    ACSDK_ERROR(LX("getAudioItemDurationFailed").d("reason", "NoActiveAdapter").d("player", m_playerInFocus));
                    return milliseconds::zero();
                }
                return m_adapterInFocus->getState().playbackState.duration;
            }
            void ExternalMediaPlayer::setPlayerInFocus(const std::string& playerInFocus) {
                ACSDK_DEBUG5(LX(__func__));
                {
                    auto lock = unique_lock<std::mutex>(m_authorizedMutex);
                    if (m_authorizedAdapters.find(playerInFocus) == m_authorizedAdapters.end()) {
                        ACSDK_ERROR(LX("setPlayerInFocusFailed").d("reason", "unauthorizedPlayer").d("playerId", playerInFocus));
                        return;
                    }
                }
                ACSDK_DEBUG(LX(__func__).d("playerInFocus", playerInFocus));
                auto adapterInFocus = getAdapterByPlayerId(playerInFocus);
                {
                    lock_guard<mutex> lock{m_inFocusAdapterMutex};
                    PlaybackHandlerInterface phi = reinterpret_cast<PlaybackHandlerInterface&>(*this);
                    shared_ptr<PlaybackHandlerInterface> sphi = make_shared<PlaybackHandlerInterface>(phi);
                    m_playbackRouter->setHandler(sphi);
                    if (m_playerInFocus == playerInFocus) {
                        ACSDK_DEBUG5(LX(__func__).m("Aborting - no change"));
                        return;
                    }
                    m_playerInFocus = playerInFocus;
                    m_adapterInFocus = adapterInFocus;
                }
            }
            void ExternalMediaPlayer::onButtonPressed(PlaybackButton button) {
                std::string playerInFocus;
                {
                    lock_guard<mutex> lock{m_inFocusAdapterMutex};
                    playerInFocus = m_playerInFocus;
                }
                std::string localPlayerId;
                if (!playerInFocus.empty()) {
                    auto lock = unique_lock<mutex>(m_authorizedMutex);
                    auto playerIdToLocalPlayerId = m_authorizedAdapters.find(playerInFocus);
                    if (m_authorizedAdapters.end() == playerIdToLocalPlayerId) {
                        ACSDK_ERROR(LX("onButtonPressedFailed").d("reason", "noMatchingLocalId").d(PLAYER_ID, m_playerInFocus));
                        return;
                    }
                    localPlayerId = playerIdToLocalPlayerId->second;
                    lock.unlock();
                    auto adapter = getAdapterByLocalPlayerId(localPlayerId);
                    if (!adapter) {
                        ACSDK_ERROR(LX("AdapterNotFound").d("player", localPlayerId));
                        return;
                    }
                    auto buttonIt = g_buttonToRequestType.find(button);
                    if (g_buttonToRequestType.end() == buttonIt) {
                        ACSDK_ERROR(LX("ButtonToRequestTypeNotFound").d("button", button));
                        return;
                    }
                    adapter->handlePlayControl(buttonIt->second);
                }
            }
            void ExternalMediaPlayer::onTogglePressed(PlaybackToggle toggle, bool action) {
                std::string playerInFocus;
                {
                    lock_guard<mutex> lock{m_inFocusAdapterMutex};
                    playerInFocus = m_playerInFocus;
                }
                std::string localPlayerId;
                if (!playerInFocus.empty()) {
                    auto lock = unique_lock<std::mutex>(m_authorizedMutex);
                    auto playerIdToLocalPlayerId = m_authorizedAdapters.find(m_playerInFocus);
                    if (m_authorizedAdapters.end() == playerIdToLocalPlayerId) {
                        ACSDK_ERROR(LX("onTogglePressedFailed").d("reason", "noMatchingLocalId").d(PLAYER_ID, m_playerInFocus));
                        return;
                    }
                    localPlayerId = playerIdToLocalPlayerId->second;
                    lock.unlock();
                    auto adapter = getAdapterByLocalPlayerId(localPlayerId);
                    if (!adapter) {
                        ACSDK_ERROR(LX("AdapterNotFound").d("player", localPlayerId));
                        return;
                    }
                    auto toggleIt = g_toggleToRequestType.find(toggle);
                    if (g_toggleToRequestType.end() == toggleIt) {
                        ACSDK_ERROR(LX("ToggleToRequestTypeNotFound").d("toggle", toggle));
                        return;
                    }
                    auto toggleStates = toggleIt->second;
                    if (action) adapter->handlePlayControl(toggleStates.first);
                    else adapter->handlePlayControl(toggleStates.second);
                }
            }
            void ExternalMediaPlayer::doShutdown() {
                m_executor.shutdown();
                m_contextManager->setStateProvider(SESSION_STATE, nullptr);
                m_contextManager->setStateProvider(PLAYBACK_STATE, nullptr);
                {
                    unique_lock<mutex> lock{m_adaptersMutex};
                    auto adaptersCopy = m_adapters;
                    m_adapters.clear();
                    lock.unlock();
                    for (const auto& adapter : adaptersCopy) {
                        if (!adapter.second) continue;
                        adapter.second->shutdown();
                    }
                }
                m_exceptionEncounteredSender.reset();
                m_contextManager.reset();
                m_playbackRouter.reset();
                m_speakerManager.reset();
            }
            void ExternalMediaPlayer::removeDirective(shared_ptr<DirectiveInfo> info) {
                if (info->directive && info->result) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void ExternalMediaPlayer::setHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                if (info && info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void ExternalMediaPlayer::sendExceptionEncounteredAndReportFailed(shared_ptr<DirectiveInfo> info, const std::string& message,
                                                                              ExceptionErrorType type) {
                if (info && info->directive) {
                    m_exceptionEncounteredSender->sendExceptionEncountered((char*)info->directive->getUnparsedDirective().data(), type, message);
                }
                if (info && info->result) info->result->setFailed(message);
                removeDirective(info);
            }
            void ExternalMediaPlayer::executeProvideState(const NamespaceAndName& stateProviderName, bool sendToken,
                                                          unsigned int stateRequestToken) {
                ACSDK_DEBUG(LX("executeProvideState").d("sendToken", sendToken).d("stateRequestToken", stateRequestToken));
                std::string state;
                if (stateProviderName == SESSION_STATE) state = provideSessionState();
                else if (stateProviderName == PLAYBACK_STATE) state = providePlaybackState();
                else {
                    ACSDK_ERROR(LX("executeProvideState").d("reason", "unknownStateProviderName"));
                    return;
                }
                SetStateResult result;
                if (sendToken) result = m_contextManager->setState(stateProviderName, state, StateRefreshPolicy::ALWAYS, stateRequestToken);
                else result = m_contextManager->setState(stateProviderName, state, StateRefreshPolicy::ALWAYS);
                if (result != SetStateResult::SUCCESS) {
                    ACSDK_ERROR(LX("executeProvideState").d("reason", "contextManagerSetStateFailedForEMPState"));
                }
            }
            std::string ExternalMediaPlayer::provideSessionState() {
                Document state(rapidjson::kObjectType);
                Document::AllocatorType& stateAlloc = state.GetAllocator();
                state.AddMember(StringRef(AGENT_KEY), std::string(m_agentString), stateAlloc);
                state.AddMember(StringRef(SPI_VERSION_KEY), std::string(ExternalMediaPlayer::SPI_VERSION), stateAlloc);
                state.AddMember(StringRef(PLAYER_IN_FOCUS), m_playerInFocus, stateAlloc);
                Value players(kArrayType);
                unordered_map<std::string, std::string> authorizedAdaptersCopy;
                {
                    lock_guard<mutex> lock(m_authorizedMutex);
                    authorizedAdaptersCopy = m_authorizedAdapters;
                }
                for (const auto& idToLocalId : authorizedAdaptersCopy) {
                    const auto& adapter = getAdapterByLocalPlayerId(idToLocalId.second);
                    if (!adapter) {
                        continue;
                    }
                    auto state = adapter->getState().sessionState;
                    Value playerJson = buildSessionState(state, stateAlloc);
                    players.PushBack(playerJson);
                    ObservableSessionProperties update{state.loggedIn, state.userName};
                    notifyObservers(state.playerId, &update);
                }
                Value _PLAYERS{PLAYERS, strlen(PLAYERS)};
                state.AddMember(_PLAYERS, players);
                StringBuffer buffer;
                rapidjson::Writer<StringBuffer> writer(buffer);
                if (!state.Accept(writer)) {
                    ACSDK_ERROR(LX("provideSessionStateFailed").d("reason", "writerRefusedJsonObject"));
                    return "";
                }
                return buffer.GetString();
            }
            std::string ExternalMediaPlayer::providePlaybackState() {
                Document state(kObjectType);
                Document::AllocatorType& stateAlloc = state.GetAllocator();
                Value _state(kObjectType);
                if (!buildDefaultPlayerState(&_state, stateAlloc)) return "";
                Value players(kArrayType);
                unordered_map<std::string, std::string> authorizedAdaptersCopy;
                {
                    lock_guard<mutex> lock(m_authorizedMutex);
                    authorizedAdaptersCopy = m_authorizedAdapters;
                }
                for (const auto& idToLocalId : authorizedAdaptersCopy) {
                    const auto& adapter = getAdapterByLocalPlayerId(idToLocalId.second);
                    if (!adapter) continue;
                    auto playbackState = adapter->getState().playbackState;
                    auto sessionState = adapter->getState().sessionState;
                    Value playerJson = buildPlaybackState(playbackState, stateAlloc);
                    players.PushBack(playerJson);
                    ObservablePlaybackStateProperties update;
                    update.state = playbackState.state;
                    update.trackName = playbackState.trackName;
                    update.playRequestor = playbackState.playRequestor;
                    notifyObservers(sessionState.playerId, &update);
                }
                notifyRenderPlayerInfoCardsObservers();
                state.AddMember(PLAYERS, players, stateAlloc);
                StringBuffer buffer;
                rapidjson::Writer<StringBuffer> writer(buffer);
                if (!state.Accept(writer)) {
                    ACSDK_ERROR(LX("providePlaybackState").d("reason", "writerRefusedJsonObject"));
                    return "";
                }
                return buffer.GetString();
            }
            void ExternalMediaPlayer::createAdapters(const AdapterMediaPlayerMap& mediaPlayers, const AdapterSpeakerMap& speakers,
                                                     const AdapterCreationMap& adapterCreationMap, shared_ptr<MessageSenderInterface> messageSender,
                                                     shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ContextManagerInterface> contextManager) {
                ACSDK_DEBUG0(LX("createAdapters"));
                for (auto& entry : adapterCreationMap) {
                    auto mediaPlayerIt = mediaPlayers.find(entry.first);
                    auto speakerIt = speakers.find(entry.first);
                    if (mediaPlayerIt == mediaPlayers.end()) {
                        ACSDK_ERROR(LX("adapterCreationFailed").d(PLAYER_ID, entry.first).d("reason", "nullMediaPlayer"));
                        continue;
                    }
                    if (speakerIt == speakers.end()) {
                        ACSDK_ERROR(LX("adapterCreationFailed").d(PLAYER_ID, entry.first).d("reason", "nullSpeaker"));
                        continue;
                    }
                    ExternalMediaPlayerInterface empi = reinterpret_cast<ExternalMediaPlayerInterface&>(*this);
                    shared_ptr<ExternalMediaPlayerInterface> pempi = make_shared<ExternalMediaPlayerInterface>(empi);
                    auto adapter = entry.second(m_metricRecorder, (*mediaPlayerIt).second, (*speakerIt).second, m_speakerManager,
                                                messageSender, focusManager, contextManager, pempi);
                    if (adapter) {
                        lock_guard<mutex> lock{m_adaptersMutex};
                        m_adapters[entry.first] = adapter;
                    } else { ACSDK_ERROR(LX("adapterCreationFailed").d(PLAYER_ID, entry.first)); }
                }
                sendReportDiscoveredPlayersEvent();
            }
            void ExternalMediaPlayer::sendReportDiscoveredPlayersEvent() {
                Document payload(rapidjson::kObjectType);
                Value _AGENT_KEY{AGENT_KEY, strlen(AGENT_KEY)};
                payload.AddMember(_AGENT_KEY, std::string(m_agentString), payload.GetAllocator());
                Value players(kArrayType);
                for (const auto& idToAdapter : m_adapters) {
                    Value player(kObjectType);
                    shared_ptr<ExternalMediaAdapterInterface> adapter = idToAdapter.second;
                    Value _LOCAL_PLAYER_ID{LOCAL_PLAYER_ID, strlen(LOCAL_PLAYER_ID)};
                    Value _SPI_VERSION_KEY{SPI_VERSION_KEY, strlen(SPI_VERSION_KEY)};
                    Value _VALIDATION_METHOD{VALIDATION_METHOD, strlen(VALIDATION_METHOD)};
                    Value _VALIDATION_DATA{VALIDATION_DATA, strlen(VALIDATION_DATA)};
                    std::string localPlayerId = adapter->getState().sessionState.localPlayerId;
                    std::string spiVersion = adapter->getState().sessionState.spiVersion;
                    std::string none = "NONE";
                    Value _localPlayerId{localPlayerId.data(), localPlayerId.length()};
                    Value _spiVersion{spiVersion.data(), spiVersion.length()};
                    Value _none{none.data(), none.length()};
                    player.AddMember(_LOCAL_PLAYER_ID, _localPlayerId);
                    player.AddMember(_SPI_VERSION_KEY, _spiVersion);
                    player.AddMember(_VALIDATION_METHOD, _none);
                    Value validationData(kArrayType);
                    player.AddMember(_VALIDATION_DATA, validationData);
                    players.PushBack(player);
                }
                payload.AddMember(PLAYERS, players, payload.GetAllocator());
                StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                if (!payload.Accept(writer)) {
                    ACSDK_ERROR(LX("sendReportDiscoveredPlayersEventFailed").d("reason", "writerRefusedJsonObject"));
                    return;
                }
                auto event = buildJsonEventString(REPORT_DISCOVERED_PLAYERS, "", buffer.GetString());
                auto request = std::make_shared<MessageRequest>(event.second);
                m_certifiedMessageSender->sendJSONMessage(request->getJsonContent());
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> ExternalMediaPlayer::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            void ExternalMediaPlayer::addObserver(shared_ptr<ExternalMediaPlayerObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observersMutex};
                m_observers.insert(observer);
            }
            void ExternalMediaPlayer::removeObserver(shared_ptr<ExternalMediaPlayerObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observersMutex};
                m_observers.erase(observer);
            }
            void ExternalMediaPlayer::notifyObservers(const std::string& playerId, const ObservableSessionProperties* sessionProperties) {
                notifyObservers(playerId, sessionProperties, nullptr);
            }
            void ExternalMediaPlayer::notifyObservers(const std::string& playerId, const ObservablePlaybackStateProperties* playbackProperties) {
                notifyObservers(playerId, nullptr, playbackProperties);
            }
            void ExternalMediaPlayer::notifyObservers(const std::string& playerId, const ObservableSessionProperties* sessionProperties,
                                                      const ObservablePlaybackStateProperties* playbackProperties) {
                if (playerId.empty()) {
                    ACSDK_ERROR(LX("notifyObserversFailed").d("reason", "emptyPlayerId"));
                    return;
                }
                unique_lock<mutex> lock{m_observersMutex};
                auto observers = m_observers;
                lock.unlock();
                for (const auto& observer : observers) {
                    if (sessionProperties) observer->onLoginStateProvided(playerId, *sessionProperties);
                    if (playbackProperties) observer->onPlaybackStateProvided(playerId, *playbackProperties);
                }
            }
            void ExternalMediaPlayer::notifyRenderPlayerInfoCardsObservers() {
                ACSDK_DEBUG5(LX(__func__));
                unique_lock<mutex> lock{m_inFocusAdapterMutex};
                if (m_adapterInFocus) {
                    auto adapterState = m_adapterInFocus->getState();
                    lock.unlock();
                    stringstream ss{adapterState.playbackState.state};
                    PlayerActivity playerActivity = PlayerActivity::IDLE;
                    ss >> playerActivity;
                    if (ss.fail()) {
                        ACSDK_ERROR(LX("notifyRenderPlayerInfoCardsFailed").d("reason", "invalidState")
                            .d("state", adapterState.playbackState.state));
                        return;
                    }
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.audioItemId = adapterState.playbackState.trackId;
                    context.offset = getAudioItemOffset();
                    MediaPropertiesInterface mpi = reinterpret_cast<MediaPropertiesInterface&>(*this);
                    context.mediaProperties = make_shared<MediaPropertiesInterface>(mpi);
                    {
                        lock_guard<mutex> lock{m_observersMutex};
                        if (m_renderPlayerObserver) {
                            m_renderPlayerObserver->onRenderPlayerCardsInfoChanged(playerActivity, context);
                        }
                    }
                }
            }
        }
    }
}
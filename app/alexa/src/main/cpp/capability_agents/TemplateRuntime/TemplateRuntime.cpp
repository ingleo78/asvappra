#include <ostream>
#include <json/en.h>
#include <avs/CapabilityConfiguration.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "TemplateRuntime.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace templateRuntime {
            using namespace configuration;
            using namespace json;
            using namespace rapidjson;
            static const string TEMPLATERUNTIME_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string TEMPLATERUNTIME_CAPABILITY_INTERFACE_NAME = "TemplateRuntime";
            static const string TEMPLATERUNTIME_CAPABILITY_INTERFACE_VERSION = "1.1";
            static const string TAG{"TemplateRuntime"};
            static const string TEMPLATERUNTIME_CONFIGURATION_ROOT_KEY = "templateRuntimeCapabilityAgent";
            static const string TEMPLATERUNTIME_TTS_FINISHED_KEY = "displayCardTTSFinishedTimeout";
            static const string TEMPLATERUNTIME_AUDIOPLAYBACK_FINISHED_KEY = "displayCardAudioPlaybackFinishedTimeout";
            static const string TEMPLATERUNTIME_AUDIOPLAYBACK_STOPPED_PAUSED_KEY = "displayCardAudioPlaybackStoppedPausedTimeout";
            #define LX(event) LogEntry(TAG, event)
            static const string CHANNEL_NAME = FocusManagerInterface::VISUAL_CHANNEL_NAME;
            static const string NAMESPACE{"TemplateRuntime"};
            static const string RENDER_TEMPLATE{"RenderTemplate"};
            static const string RENDER_PLAYER_INFO{"RenderPlayerInfo"};
            static const NamespaceAndName TEMPLATE{NAMESPACE, RENDER_TEMPLATE};
            static const NamespaceAndName PLAYER_INFO{NAMESPACE, RENDER_PLAYER_INFO};
            static const string AUDIO_ITEM_ID_TAG{"audioItemId"};
            static const size_t MAXIMUM_QUEUE_SIZE{100};
            static const milliseconds DEFAULT_TTS_FINISHED_TIMEOUT_MS{2000};
            static const milliseconds DEFAULT_AUDIO_FINISHED_TIMEOUT_MS{2000};
            static const milliseconds DEFAULT_AUDIO_STOPPED_PAUSED_TIMEOUT_MS{60000};
            static shared_ptr<CapabilityConfiguration> getTemplateRuntimeCapabilityConfiguration();
            shared_ptr<TemplateRuntime> TemplateRuntime::create(const unordered_set<shared_ptr<RenderPlayerInfoCardsProviderInterface>>& renderPlayerInfoCardInterface,
                                                                shared_ptr<FocusManagerInterface> focusManager,
                                                                shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender) {
                if (!focusManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullFocusManager"));
                    return nullptr;
                }
                if (!exceptionSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
                    return nullptr;
                }
                shared_ptr<TemplateRuntime> templateRuntime(new TemplateRuntime(renderPlayerInfoCardInterface, focusManager, exceptionSender));
                if (!templateRuntime->initialize()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Initialization error."));
                    return nullptr;
                }
                for (const auto& renderPlayerInfoCardProvider : renderPlayerInfoCardInterface) {
                    if (!renderPlayerInfoCardProvider) {
                        ACSDK_ERROR(LX("createFailed").d("reason", "nullRenderPlayerInfoCardInterface"));
                        return nullptr;
                    }
                    renderPlayerInfoCardProvider->setObserver(templateRuntime);
                }
                return templateRuntime;
            }
            bool TemplateRuntime::initialize() {
                auto configurationRoot = ConfigurationNode::getRoot()[TEMPLATERUNTIME_CONFIGURATION_ROOT_KEY];
                configurationRoot.getDuration<milliseconds>(TEMPLATERUNTIME_TTS_FINISHED_KEY, &m_ttsFinishedTimeout, DEFAULT_TTS_FINISHED_TIMEOUT_MS);
                configurationRoot.getDuration<milliseconds>(TEMPLATERUNTIME_AUDIOPLAYBACK_FINISHED_KEY, &m_audioPlaybackFinishedTimeout,
                                                            DEFAULT_AUDIO_FINISHED_TIMEOUT_MS);
                configurationRoot.getDuration<milliseconds>(TEMPLATERUNTIME_AUDIOPLAYBACK_STOPPED_PAUSED_KEY, &m_audioPlaybackStoppedPausedTimeout,
                                                            DEFAULT_AUDIO_STOPPED_PAUSED_TIMEOUT_MS);
                return true;
            }
            void TemplateRuntime::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX("handleDirectiveImmediately"));
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void TemplateRuntime::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("preHandleDirective"));
            }
            void TemplateRuntime::handleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("handleDirective"));
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("preHandleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                if (info->directive->getName() == TEMPLATE.name) handleRenderTemplateDirective(info);
                else if (info->directive->getName() == PLAYER_INFO.name) handleRenderPlayerInfoDirective(info);
                else handleUnknownDirective(info);
            }
            void TemplateRuntime::cancelDirective(shared_ptr<DirectiveInfo> info) {
                removeDirective(info);
            }
            DirectiveHandlerConfiguration TemplateRuntime::getConfiguration() const {
                ACSDK_DEBUG5(LX("getConfiguration"));
                DirectiveHandlerConfiguration configuration;
                auto visualNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, false);
                configuration[TEMPLATE] = visualNonBlockingPolicy;
                configuration[PLAYER_INFO] = visualNonBlockingPolicy;
                return configuration;
            }
            void TemplateRuntime::onFocusChanged(avsCommon::avs::FocusState newFocus, MixingBehavior) {
                m_executor.submit([this, newFocus]() { executeOnFocusChangedEvent(newFocus); });
            }
            void TemplateRuntime::onRenderPlayerCardsInfoChanged(PlayerActivity state, const Context& context) {
                ACSDK_DEBUG5(LX("onRenderPlayerCardsInfoChanged"));
                m_executor.submit([this, state, context]() {
                    ACSDK_DEBUG5(LX("onPlayerActivityChangedInExecutor"));
                    executeAudioPlayerInfoUpdates(state, context);
                });
            }
            void TemplateRuntime::onDialogUXStateChanged(DialogUXState newState) {
                ACSDK_DEBUG5(LX("onDialogUXStateChanged").d("state", newState));
                m_executor.submit([this, newState]() {
                    if (TemplateRuntime::State::DISPLAYING == m_state && m_lastDisplayedDirective &&
                        m_lastDisplayedDirective->directive->getName() == RENDER_TEMPLATE) {
                        if (avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE == newState) {
                            executeStartTimer(m_ttsFinishedTimeout);
                        } else if (DialogUXState::EXPECTING == newState || DialogUXState::SPEAKING == newState) {
                            executeStopTimer();
                        }
                    }
                });
            }
            void TemplateRuntime::addObserver(shared_ptr<TemplateRuntimeObserverInterface> observer) {
                ACSDK_DEBUG5(LX("addObserver"));
                if (!observer) {
                    ACSDK_ERROR(LX("addObserver").m("Observer is null."));
                    return;
                }
                m_executor.submit([this, observer]() {
                    ACSDK_DEBUG5(LX("addObserverInExecutor"));
                    if (!m_observers.insert(observer).second) {
                        ACSDK_ERROR(LX("addObserverInExecutor").m("Duplicate observer."));
                    }
                });
            }
            void TemplateRuntime::removeObserver(shared_ptr<TemplateRuntimeObserverInterface> observer) {
                ACSDK_DEBUG5(LX("removeObserver"));
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserver").m("Observer is null."));
                    return;
                }
                m_executor.submit([this, observer]() {
                    ACSDK_DEBUG5(LX("removeObserverInExecutor"));
                    if (m_observers.erase(observer) == 0) { ACSDK_WARN(LX("removeObserverInExecutor").m("Nonexistent observer.")); }
                });
            }
            TemplateRuntime::TemplateRuntime(const unordered_set<shared_ptr<RenderPlayerInfoCardsProviderInterface>>& renderPlayerInfoCardsInterfaces,
                                             shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender) :
                                             CapabilityAgent{NAMESPACE, exceptionSender}, RequiresShutdown{"TemplateRuntime"},
                                             m_isRenderTemplateLastReceived{false}, m_focus{FocusState::NONE}, m_state{TemplateRuntime::State::IDLE},
                                             m_renderPlayerInfoCardsInterfaces{renderPlayerInfoCardsInterfaces}, m_focusManager{focusManager} {
                m_capabilityConfigurations.insert(getTemplateRuntimeCapabilityConfiguration());
            }
            shared_ptr<CapabilityConfiguration> getTemplateRuntimeCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, TEMPLATERUNTIME_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, TEMPLATERUNTIME_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, TEMPLATERUNTIME_CAPABILITY_INTERFACE_VERSION});
                return make_shared<CapabilityConfiguration>(configMap);
            }
            void TemplateRuntime::doShutdown() {
                m_executor.shutdown();
                m_focusManager.reset();
                m_observers.clear();
                m_activeRenderPlayerInfoCardsProvider.reset();
                m_audioItemsInExecution.clear();
                m_audioPlayerInfo.clear();
                for (const auto renderPlayerInfoCardsInterface : m_renderPlayerInfoCardsInterfaces) {
                    renderPlayerInfoCardsInterface->setObserver(nullptr);
                }
                m_renderPlayerInfoCardsInterfaces.clear();
            }
            void TemplateRuntime::removeDirective(shared_ptr<DirectiveInfo> info) {
                if (info->directive && info->result) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void TemplateRuntime::displayCardCleared() {
                m_executor.submit([this]() { executeCardClearedEvent(); });
            }
            void TemplateRuntime::setHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                if (info && info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void TemplateRuntime::handleRenderTemplateDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("handleRenderTemplateDirective"));
                m_executor.submit([this, info]() {
                    ACSDK_DEBUG5(LX("handleRenderTemplateDirectiveInExecutor"));
                    m_isRenderTemplateLastReceived = true;
                    executeDisplayCardEvent(info);
                    setHandlingCompleted(info);
                });
            }
            void TemplateRuntime::handleRenderPlayerInfoDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("handleRenderPlayerInfoDirective"));
                m_executor.submit([this, info]() {
                    ACSDK_DEBUG5(LX("handleRenderPlayerInfoDirectiveInExecutor"));
                    m_isRenderTemplateLastReceived = false;
                    Document payload;
                    ParseResult result = payload.Parse(info->directive->getPayload().data());
                    if (!result) {
                        ACSDK_ERROR(LX("handleRenderPlayerInfoDirectiveInExecutorParseFailed").d("reason", GetParseError_En(result.Code()))
                            .d("offset", result.Offset()).d("messageId", info->directive->getMessageId()));
                        sendExceptionEncounteredAndReportFailed(info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    std::string audioItemId;
                    if (!jsonUtils::retrieveValue(payload, AUDIO_ITEM_ID_TAG, &audioItemId)) {
                        ACSDK_ERROR(LX("handleRenderPlayerInfoDirective").d("reason", "missingAudioItemId")
                            .d("messageId", info->directive->getMessageId()));
                        sendExceptionEncounteredAndReportFailed(info, "missing audioItemId");
                        return;
                    }
                    size_t found = string::npos;
                    for (auto& executionMap : m_audioItemsInExecution) {
                        if (!executionMap.second.audioItemId.empty()) found = audioItemId.find(executionMap.second.audioItemId);
                        if (found != string::npos) {
                            ACSDK_DEBUG3(LX("handleRenderPlayerInfoDirectiveInExecutor").d("audioItemId", audioItemId)
                                .m("Matching audioItemId in execution."));
                            executionMap.second.directive = info;
                            m_activeRenderPlayerInfoCardsProvider = executionMap.first;
                            m_audioPlayerInfo[m_activeRenderPlayerInfoCardsProvider].offset = executionMap.first->getAudioItemOffset();
                            executeStopTimer();
                            executeDisplayCardEvent(info);
                            m_audioItems.clear();
                            break;
                        }
                    }
                    if (string::npos == found) {
                        ACSDK_DEBUG3(LX("handleRenderPlayerInfoDirectiveInExecutor").d("audioItemId", audioItemId)
                            .m("Not matching audioItemId in execution."));
                        AudioItemPair itemPair{audioItemId, info};
                        if (m_audioItems.size() == MAXIMUM_QUEUE_SIZE) {
                            auto discardedAudioItem = m_audioItems.back();
                            m_audioItems.pop_back();
                            ACSDK_ERROR(LX("handleRenderPlayerInfoDirective").d("reason", "queueIsFull")
                                            .d("discardedAudioItemId", discardedAudioItem.audioItemId));
                        }
                        m_audioItems.push_front(itemPair);
                    }
                    setHandlingCompleted(info);
                });
            }
            void TemplateRuntime::handleUnknownDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "unknownDirective").d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));
                m_executor.submit([this, info] {
                    const string exceptionMessage ="unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
                    sendExceptionEncounteredAndReportFailed(info, exceptionMessage, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                });
            }
            void TemplateRuntime::executeAudioPlayerInfoUpdates(avsCommon::avs::PlayerActivity state, const Context& context) {
                ACSDK_DEBUG5(LX("executeAudioPlayerInfoUpdates").d("audioItemId", context.audioItemId).d("offset", context.offset.count())
                    .d("audioPlayerState", state).d("isRenderTemplatelastReceived", m_isRenderTemplateLastReceived));
                if (PlayerActivity::IDLE == state || PlayerActivity::BUFFER_UNDERRUN == state) return;
                if (!context.mediaProperties) {
                    ACSDK_ERROR(LX("executeAudioPlayerInfoUpdatesFailed").d("reason", "nullRenderPlayerInfoCardsInterface"));
                    return;
                }
                const auto& currentRenderPlayerInfoCardsProvider = context.mediaProperties;
                if (m_audioPlayerInfo[currentRenderPlayerInfoCardsProvider].audioPlayerState == state &&
                    m_audioItemsInExecution[currentRenderPlayerInfoCardsProvider].audioItemId == context.audioItemId) {
                    return;
                }
                auto isStateUpdated = (m_audioPlayerInfo[currentRenderPlayerInfoCardsProvider].audioPlayerState != state);
                m_audioPlayerInfo[currentRenderPlayerInfoCardsProvider].audioPlayerState = state;
                m_audioPlayerInfo[currentRenderPlayerInfoCardsProvider].offset = context.offset;
                if (m_audioItemsInExecution[currentRenderPlayerInfoCardsProvider].audioItemId != context.audioItemId) {
                    m_audioItemsInExecution[currentRenderPlayerInfoCardsProvider].audioItemId = context.audioItemId;
                    m_audioItemsInExecution[currentRenderPlayerInfoCardsProvider].directive.reset();
                    for (auto it = m_audioItems.begin(); it != m_audioItems.end(); ++it) {
                        auto found = it->audioItemId.find(context.audioItemId);
                        if (string::npos != found) {
                            ACSDK_DEBUG3(LX("executeAudioPlayerInfoUpdates").d("audioItemId", context.audioItemId)
                                .m("Found matching audioItemId in queue."));
                            m_audioItemsInExecution[currentRenderPlayerInfoCardsProvider].directive = it->directive;
                            m_activeRenderPlayerInfoCardsProvider = currentRenderPlayerInfoCardsProvider;
                            m_audioItems.erase(it, m_audioItems.end());
                            break;
                        }
                    }
                }
                if (m_isRenderTemplateLastReceived && state != PlayerActivity::PLAYING) return;
                m_isRenderTemplateLastReceived = false;
                if (m_audioItemsInExecution[currentRenderPlayerInfoCardsProvider].directive) {
                    if (isStateUpdated) executeAudioPlayerStartTimer(state);
                    executeDisplayCardEvent(m_audioItemsInExecution[currentRenderPlayerInfoCardsProvider].directive);
                } else {
                    if (TemplateRuntime::State::ACQUIRING == m_state) m_state = TemplateRuntime::State::RELEASING;
                }
            }
            void TemplateRuntime::executeAudioPlayerStartTimer(PlayerActivity state) {
                if (PlayerActivity::PLAYING == state) executeStopTimer();
                else if (PlayerActivity::PAUSED == state || PlayerActivity::STOPPED == state) {
                    executeStartTimer(m_audioPlaybackStoppedPausedTimeout);
                } else if (PlayerActivity::FINISHED == state) executeStartTimer(m_audioPlaybackFinishedTimeout);
            }
            void TemplateRuntime::executeRenderPlayerInfoCallbacks(bool isClearCard) {
                ACSDK_DEBUG3(LX("executeRenderPlayerInfoCallbacks").d("isClearCard", isClearCard ? "True" : "False"));
                if (isClearCard) {
                    for (auto& observer : m_observers) {
                        observer->clearPlayerInfoCard();
                    }
                } else {
                    if (!m_activeRenderPlayerInfoCardsProvider) {
                        ACSDK_ERROR(LX("executeRenderPlayerInfoCallbacksFailed").d("reason", "nullActiveRenderPlayerInfoCardsProvider"));
                        return;
                    }
                    if (!m_audioItemsInExecution[m_activeRenderPlayerInfoCardsProvider].directive) {
                        ACSDK_ERROR(LX("executeRenderPlayerInfoCallbacksFailed").d("reason", "nullAudioItemInExecution"));
                        return;
                    }
                    auto payload = m_audioItemsInExecution[m_activeRenderPlayerInfoCardsProvider].directive->directive->getPayload();
                    for (auto& observer : m_observers) {
                        observer->renderPlayerInfoCard(payload, m_audioPlayerInfo[m_activeRenderPlayerInfoCardsProvider], m_focus);
                    }
                }
            }
            void TemplateRuntime::executeRenderTemplateCallbacks(bool isClearCard) {
                ACSDK_DEBUG3(LX("executeRenderTemplateCallbacks").d("isClear", isClearCard ? "True" : "False"));
                for (auto& observer : m_observers) {
                    if (isClearCard) observer->clearTemplateCard();
                    else observer->renderTemplateCard(m_lastDisplayedDirective->directive->getPayload(), m_focus);
                }
            }
            void TemplateRuntime::executeDisplayCard() {
                if (m_lastDisplayedDirective) {
                    if (m_lastDisplayedDirective->directive->getName() == RENDER_TEMPLATE) {
                        executeStopTimer();
                        executeRenderTemplateCallbacks(false);
                    } else executeRenderPlayerInfoCallbacks(false);
                }
            }
            void TemplateRuntime::executeClearCard() {
                if (m_lastDisplayedDirective) {
                    if (m_lastDisplayedDirective->directive->getName() == RENDER_TEMPLATE) executeRenderTemplateCallbacks(true);
                    else executeRenderPlayerInfoCallbacks(true);
                }
            }
            void TemplateRuntime::executeStartTimer(milliseconds timeout) {
                if (TemplateRuntime::State::DISPLAYING == m_state) {
                    ACSDK_DEBUG3(LX("executeStartTimer").d("timeoutInMilliseconds", timeout.count()));
                    m_clearDisplayTimer.start(timeout, [this] { m_executor.submit([this] { executeTimerEvent(); }); });
                }
            }
            void TemplateRuntime::executeStopTimer() {
                ACSDK_DEBUG3(LX("executeStopTimer"));
                m_clearDisplayTimer.stop();
            }
            std::string TemplateRuntime::stateToString(const TemplateRuntime::State state) {
                switch(state) {
                    case TemplateRuntime::State::IDLE: return "IDLE";
                    case TemplateRuntime::State::ACQUIRING: return "ACQUIRING";
                    case TemplateRuntime::State::DISPLAYING: return "DISPLAYING";
                    case TemplateRuntime::State::RELEASING: return "RELEASING";
                    case TemplateRuntime::State::REACQUIRING: return "REACQUIRING";
                }
                return "UNKNOWN";
            }
            void TemplateRuntime::executeTimerEvent() {
                State nextState = m_state;
                switch (m_state) {
                    case TemplateRuntime::State::DISPLAYING:
                        executeClearCard();
                        m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                        nextState = TemplateRuntime::State::RELEASING;
                        break;
                    case TemplateRuntime::State::IDLE: case TemplateRuntime::State::ACQUIRING: case TemplateRuntime::State::RELEASING:
                    case TemplateRuntime::State::REACQUIRING: break;
                }
                ACSDK_DEBUG3(LX("executeTimerEvent").d("prevState", stateToString(m_state)).d("nextState", stateToString(nextState)));
                m_state = nextState;
            }
            void TemplateRuntime::executeOnFocusChangedEvent(FocusState newFocus) {
                ACSDK_DEBUG5(LX("executeOnFocusChangedEvent").d("prevFocus", m_focus).d("newFocus", newFocus));
                bool weirdFocusState = false;
                State nextState = m_state;
                m_focus = newFocus;
                switch(m_state) {
                    case TemplateRuntime::State::IDLE:
                        switch(newFocus) {
                            case FocusState::FOREGROUND: case FocusState::BACKGROUND: weirdFocusState = true; break;
                            case FocusState::NONE: break;
                        }
                        break;
                    case TemplateRuntime::State::ACQUIRING:
                        switch(newFocus) {
                            case FocusState::FOREGROUND: case FocusState::BACKGROUND:
                                executeDisplayCard();
                                nextState = TemplateRuntime::State::DISPLAYING;
                                break;
                            case FocusState::NONE:
                                ACSDK_ERROR(LX("executeOnFocusChangedEvent").d("prevState", stateToString(m_state))
                                    .d("nextFocus", newFocus).m("Unexpected focus state event."));
                                nextState = TemplateRuntime::State::IDLE;
                                break;
                        }
                        break;
                    case TemplateRuntime::State::DISPLAYING:
                        switch (newFocus) {
                            case FocusState::FOREGROUND: case FocusState::BACKGROUND: executeDisplayCard(); break;
                            case FocusState::NONE:
                                executeClearCard();
                                nextState = TemplateRuntime::State::IDLE;
                                break;
                        }
                        break;
                    case TemplateRuntime::State::RELEASING:
                        switch(newFocus) {
                            case FocusState::FOREGROUND: case FocusState::BACKGROUND:
                                m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                                nextState = TemplateRuntime::State::RELEASING;
                                break;
                            case FocusState::NONE: nextState = TemplateRuntime::State::IDLE; break;
                        }
                        break;
                    case TemplateRuntime::State::REACQUIRING:
                        switch(newFocus) {
                            case FocusState::FOREGROUND: case FocusState::BACKGROUND: weirdFocusState = true; break;
                            case FocusState::NONE:
                                m_focusManager->acquireChannel(CHANNEL_NAME, shared_from_this(), NAMESPACE);
                                nextState = TemplateRuntime::State::ACQUIRING;
                                break;
                        }
                        break;
                }
                if (weirdFocusState) {
                    ACSDK_ERROR(LX("executeOnFocusChangedEvent").d("prevState", stateToString(m_state)).d("nextFocus", newFocus)
                        .m("Unexpected focus state event."));
                    m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                    nextState = TemplateRuntime::State::RELEASING;
                }
                ACSDK_DEBUG3(LX("executeOnFocusChangedEvent").d("prevState", stateToString(m_state)).d("nextState", stateToString(nextState)));
                m_state = nextState;
            }
            void TemplateRuntime::executeDisplayCardEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                State nextState = m_state;
                m_lastDisplayedDirective = info;
                switch(m_state) {
                    case TemplateRuntime::State::IDLE:
                        m_focusManager->acquireChannel(CHANNEL_NAME, shared_from_this(), NAMESPACE);
                        nextState = TemplateRuntime::State::ACQUIRING;
                        break;
                    case TemplateRuntime::State::ACQUIRING: break;
                    case TemplateRuntime::State::DISPLAYING:
                        executeDisplayCard();
                        nextState = TemplateRuntime::State::DISPLAYING;
                        break;
                    case TemplateRuntime::State::RELEASING: nextState = TemplateRuntime::State::REACQUIRING; break;
                    case TemplateRuntime::State::REACQUIRING: break;
                }
                ACSDK_DEBUG3(LX("executeDisplayCardEvent").d("prevState", stateToString(m_state)).d("nextState", stateToString(nextState)));
                m_state = nextState;
            }
            void TemplateRuntime::executeCardClearedEvent() {
                State nextState = m_state;
                switch(m_state) {
                    case TemplateRuntime::State::IDLE: case TemplateRuntime::State::ACQUIRING: break;
                    case TemplateRuntime::State::DISPLAYING:
                        m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                        nextState = TemplateRuntime::State::RELEASING;
                        break;
                    case TemplateRuntime::State::RELEASING: case TemplateRuntime::State::REACQUIRING: break;
                }
                ACSDK_DEBUG3(LX("executeCardClearedEvent").d("prevState", stateToString(m_state)).d("nextState", stateToString(nextState)));
                m_state = nextState;
            }
            unordered_set<std::shared_ptr<avsCommon::avs::CapabilityConfiguration>> TemplateRuntime::
                getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            void TemplateRuntime::addRenderPlayerInfoCardsProvider(shared_ptr<RenderPlayerInfoCardsProviderInterface> cardsProvider) {
                ACSDK_DEBUG5(LX("addRenderPlayerInfoCardsProvider"));
                if (!cardsProvider) {
                    ACSDK_ERROR(LX("addRenderPlayerInfoCardsProviderFailed").d("reason", "nullRenderPlayerInfoCardsProviderInterface"));
                    return;
                }
                cardsProvider->setObserver(shared_from_this());
                m_renderPlayerInfoCardsInterfaces.insert(cardsProvider);
            }
        }
    }
}
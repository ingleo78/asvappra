#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/CapabilityConfiguration.h>
#include <json/JSONUtils.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include <timing/TimeUtils.h>
#include "NotificationsCapabilityAgent.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        static const string NOTIFICATIONS_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
        static const string NOTIFICATIONS_CAPABILITY_INTERFACE_NAME = "Notifications";
        static const string NOTIFICATIONS_CAPABILITY_INTERFACE_VERSION = "1.0";
        static const string TAG("NotificationsCapabilityAgent");
        #define LX(event) LogEntry(TAG, event)
        static const string NAMESPACE = "Notifications";
        static const string NOTIFICATION_METRIC_SOURCE_PREFIX = "NOTIFICATION-";
        static const NamespaceAndName INDICATOR_STATE_CONTEXT_KEY{NAMESPACE, "IndicatorState"};
        static const NamespaceAndName SET_INDICATOR{NAMESPACE, "SetIndicator"};
        static const NamespaceAndName CLEAR_INDICATOR{NAMESPACE, "ClearIndicator"};
        static const string PERSIST_VISUAL_INDICATOR_KEY = "persistVisualIndicator";
        static const string PLAY_AUDIO_INDICATOR_KEY = "playAudioIndicator";
        static const string ASSET_KEY = "asset";
        static const string ASSET_ID_KEY = "assetId";
        static const string ASSET_URL_KEY = "url";
        static const char IS_ENABLED_KEY[] = "isEnabled";
        static const char IS_VISUAL_INDICATOR_PERSISTED_KEY[] = "isVisualIndicatorPersisted";
        static const milliseconds SHUTDOWN_TIMEOUT{500};
        static shared_ptr<CapabilityConfiguration> getNotificationsCapabilityConfiguration();
        static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const string& eventName) {
            if (!metricRecorder) return;
            auto metricEvent = MetricEventBuilder{}.setActivityName(NOTIFICATION_METRIC_SOURCE_PREFIX + eventName)
                                   .addDataPoint(DataPointCounterBuilder{}.setName(eventName).increment(1).build()).build();
            if (metricEvent == nullptr) {
                ACSDK_ERROR(LX("Error creating metric."));
                return;
            }
            recordMetric(metricRecorder, metricEvent);
        }
        shared_ptr<NotificationsCapabilityAgent> NotificationsCapabilityAgent::create(shared_ptr<NotificationsStorageInterface> notificationsStorage,
                                                                                      shared_ptr<NotificationRendererInterface> renderer,
                                                                                      shared_ptr<ContextManagerInterface> contextManager,
                                                                                      shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                                      shared_ptr<NotificationsAudioFactoryInterface> notificationsAudioFactory,
                                                                                      shared_ptr<CustomerDataManager> dataManager,
                                                                                      shared_ptr<MetricRecorderInterface> metricRecorder) {
            if (nullptr == notificationsStorage) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullNotificationsStorage"));
                return nullptr;
            }
            if (nullptr == renderer) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullRenderer"));
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
            if (nullptr == notificationsAudioFactory) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullNotificationsAudioFactory"));
                return nullptr;
            }
            if (nullptr == dataManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullDataManager"));
                return nullptr;
            }
            auto notificationsCapabilityAgent = shared_ptr<NotificationsCapabilityAgent>(new NotificationsCapabilityAgent(notificationsStorage, renderer,
                                                                                         contextManager, exceptionSender, notificationsAudioFactory, dataManager,
                                                                                         metricRecorder));
            if (!notificationsCapabilityAgent->init()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "initFailed"));
                return nullptr;
            }
            return notificationsCapabilityAgent;
        }
        NotificationsCapabilityAgent::NotificationsCapabilityAgent(shared_ptr<NotificationsStorageInterface> notificationsStorage, shared_ptr<NotificationRendererInterface> renderer,
                                                                   shared_ptr<ContextManagerInterface> contextManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                   shared_ptr<NotificationsAudioFactoryInterface> notificationsAudioFactory,
                                                                   shared_ptr<CustomerDataManager> dataManager, shared_ptr<MetricRecorderInterface> metricRecorder) :
                                                                   CapabilityAgent{NAMESPACE, move(exceptionSender)}, RequiresShutdown{"NotificationsCapabilityAgent"},
                                                                   CustomerDataHandler{move(dataManager)}, m_metricRecorder{move(metricRecorder)},
                                                                   m_notificationsStorage{move(notificationsStorage)}, m_contextManager{move(contextManager)},
                                                                   m_renderer{move(renderer)}, m_notificationsAudioFactory{std::move(notificationsAudioFactory)},
                                                                   m_isEnabled{false}, m_currentState{NotificationsCapabilityAgentState::IDLE} {
            m_capabilityConfigurations.insert(getNotificationsCapabilityConfiguration());
        }
        shared_ptr<CapabilityConfiguration> getNotificationsCapabilityConfiguration() {
            unordered_map<string, string> configMap;
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, NOTIFICATIONS_CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, NOTIFICATIONS_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, NOTIFICATIONS_CAPABILITY_INTERFACE_VERSION});
            return make_shared<CapabilityConfiguration>(configMap);
        }
        bool NotificationsCapabilityAgent::init() {
            ACSDK_DEBUG5(LX(__func__));
            m_renderer->addObserver(shared_from_this());
            m_contextManager->setStateProvider(INDICATOR_STATE_CONTEXT_KEY, shared_from_this());
            if (!m_notificationsStorage->open()) {
                ACSDK_INFO(LX(__func__).m("database file does not exist.  Creating."));
                if (!m_notificationsStorage->createDatabase()) {
                    ACSDK_ERROR(LX("initFailed").d("reason", "NotificationIndicatorDatabaseCreationFailed"));
                    return false;
                }
            }
            m_executor.submit([this] { executeInit(); });
            return true;
        }
        void NotificationsCapabilityAgent::executeInit() {
            ACSDK_DEBUG5(LX(__func__));
            IndicatorState currentIndicatorState = IndicatorState::OFF;
            if (!m_notificationsStorage->getIndicatorState(&currentIndicatorState)) {
                ACSDK_ERROR(LX("executeInitFailed").d("reason", "getIndicatorStateFailed"));
                return;
            }
            notifyObserversOfIndicatorState(currentIndicatorState);
            int queueSize = 0;
            if (!m_notificationsStorage->getQueueSize(&queueSize)) {
                ACSDK_ERROR(LX("executeInitFailed").d("reason", "getQueueSizeFailed"));
                return;
            }
            m_isEnabled = (queueSize > 0 || (IndicatorState::ON == currentIndicatorState));
            executeProvideState();
            if (queueSize > 0) {
                ACSDK_DEBUG5(LX(__func__).d("queueSize", queueSize).m("NotificationIndicator queue wasn't empty on startup"));
                executeStartQueueNotEmpty();
            }
        }
        void NotificationsCapabilityAgent::notifyObserversOfIndicatorState(IndicatorState state) {
            ACSDK_DEBUG5(LX(__func__).d("indicatorState", indicatorStateToInt(state)));
            for (const auto& observer : m_observers) observer->onSetIndicator(state);
        }
        void NotificationsCapabilityAgent::notifyObserversOfNotificationReceived() {
            for (const auto& observer : m_observers) observer->onNotificationReceived();
        }
        void NotificationsCapabilityAgent::provideState(
            const NamespaceAndName& stateProviderName,
            unsigned int stateRequestToken) {
            ACSDK_DEBUG5(LX(__func__).d("stateRequestToken", stateRequestToken));
            m_executor.submit([this, stateRequestToken] { executeProvideState(true, stateRequestToken); });
        }
        void NotificationsCapabilityAgent::addObserver(shared_ptr<NotificationsObserverInterface> observer) {
            ACSDK_DEBUG5(LX(__func__));
            if (!observer) {
                ACSDK_ERROR(LX(__func__).m("Observer is null."));
                return;
            }
            m_executor.submit([this, observer] {
                if (!m_observers.insert(observer).second) { ACSDK_ERROR(LX(__func__).m("Duplicate observer.")); }
                IndicatorState currentIndicatorState = IndicatorState::OFF;
                if (!m_notificationsStorage->getIndicatorState(&currentIndicatorState)) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "getIndicatorStateFailed, could not notify newly added observer"));
                    return;
                }
                observer->onSetIndicator(currentIndicatorState);
            });
        }
        void NotificationsCapabilityAgent::removeObserver(shared_ptr<NotificationsObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                return;
            }
            m_executor.submit([this, observer]() {
                if (!m_observers.erase(observer)) { ACSDK_WARN(LX("removeObserverFailed").m("Failed to erase observer")); }
            });
        }
        void NotificationsCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
            handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
        }
        void NotificationsCapabilityAgent::preHandleDirective(shared_ptr<DirectiveInfo> info) {}
        void NotificationsCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG0(LX("handleDirective").d("name", info->directive->getName()).d("messageId", info->directive->getMessageId()));
            if (!info) {
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                return;
            }
            submitMetric(m_metricRecorder, info->directive->getName());
            if (info->directive->getName() == SET_INDICATOR.name) handleSetIndicatorDirective(info);
            else if (info->directive->getName() == CLEAR_INDICATOR.name) handleClearIndicatorDirective(info);
            else {
                sendExceptionEncounteredAndReportFailed(info,"unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName(),
                                                  ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "unknownDirective").d("namespace", info->directive->getNamespace()).d("name", info->directive->getName()));
            }
        }
        void NotificationsCapabilityAgent::cancelDirective(shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG5(LX("cancelDirective").d("name", info->directive->getName()));
            removeDirective(info->directive->getMessageId());
        }
        void NotificationsCapabilityAgent::handleSetIndicatorDirective(shared_ptr<DirectiveInfo> info) {
            Document payload;
            if (!parseDirectivePayload(info, &payload)) {
                ACSDK_ERROR(LX("handleSetIndicatorDirectiveFailed").d("reason", "could not parse directive payload"));
                sendExceptionEncounteredAndReportFailed(info, "failed to parse directive");
                return;
            }
            bool persistVisualIndicator = false;
            if (!jsonUtils::retrieveValue(payload, PERSIST_VISUAL_INDICATOR_KEY, &persistVisualIndicator)) {
                ACSDK_ERROR(LX("handleSetIndicatorDirectiveFailed").d("reason", "payload missing persistVisualIndicator")
                                .d("messageId", info->directive->getMessageId()));
                sendExceptionEncounteredAndReportFailed(info, "missing persistVisualIndicator");
                return;
            }
            bool playAudioIndicator = false;
            if (!jsonUtils::retrieveValue(payload, PLAY_AUDIO_INDICATOR_KEY, &playAudioIndicator)) {
                ACSDK_ERROR(LX("handleSetIndicatorDirectiveFailed").d("reason", "payload missing playAudioIndicator")
                                .d("messageId", info->directive->getMessageId()));
                sendExceptionEncounteredAndReportFailed(info, "missing playAudioIndicator");
                return;
            }
            string assetId;
            string url;
            if (playAudioIndicator) {
                Value::ConstMemberIterator assetJson;
                Value _payload{payload.GetString(), strlen(payload.GetString())};
                if (!jsonUtils::findNode(_payload, ASSET_KEY, &assetJson)) {
                    ACSDK_ERROR(LX("handleSetIndicatorDirectiveFailed").d("reason", "payload missing asset").d("messageId", info->directive->getMessageId()));
                    sendExceptionEncounteredAndReportFailed(info, "missing asset");
                    return;
                }
                if (!jsonUtils::retrieveValue(assetJson->value, ASSET_ID_KEY, &assetId)) {
                    ACSDK_ERROR(LX("handleSetIndicatorDirectiveFailed").d("reason", "payload missing assetId").d("messageId", info->directive->getMessageId()));
                    sendExceptionEncounteredAndReportFailed(info, "missing assetId");
                    return;
                }
                if (!jsonUtils::retrieveValue(assetJson->value, ASSET_URL_KEY, &url)) {
                    ACSDK_ERROR(LX("handleSetIndicatorDirectiveFailed").d("reason", "payload missing url").d("messageId", info->directive->getMessageId()));
                    sendExceptionEncounteredAndReportFailed(info, "missing url");
                    return;
                }
            }
            const NotificationIndicator nextNotificationIndicator(persistVisualIndicator, playAudioIndicator, assetId, url);
            m_executor.submit([this, nextNotificationIndicator, info] { executeSetIndicator(nextNotificationIndicator, info); });
        }
        void NotificationsCapabilityAgent::executeRenderNotification(const NotificationIndicator& notificationIndicator) {
            m_currentAssetId = notificationIndicator.asset.assetId;
            executeSetState(NotificationsCapabilityAgentState::PLAYING);
            if (notificationIndicator.playAudioIndicator) {
                if (!m_renderer->renderNotification(m_notificationsAudioFactory->notificationDefault(), notificationIndicator.asset.url)) {
                    ACSDK_ERROR(LX("executeRenderNotificationFailed").d("reason", "failed to render the notification indicator"));
                    executeSetState(NotificationsCapabilityAgentState::IDLE);
                }
            } else executeOnPlayFinished();
        }
        void NotificationsCapabilityAgent::executePossibleIndicatorStateChange(const IndicatorState& nextIndicatorState) {
            IndicatorState storedIndicatorState = IndicatorState::OFF;
            if (!m_notificationsStorage->getIndicatorState(&storedIndicatorState)) {
                ACSDK_ERROR(LX("executePossibleIndicatorStateChangeFailed").d("reason", "failed to get stored indicator state"));
                return;
            }
            if (nextIndicatorState != storedIndicatorState) {
                if (!m_notificationsStorage->setIndicatorState(nextIndicatorState)) {
                    ACSDK_ERROR(LX("executePossibleIndicatorStateChangeFailed").d("reason", "failed to set new indicator state"));
                    return;
                }
                notifyObserversOfIndicatorState(nextIndicatorState);
                executeProvideState();
            }
        }
        void NotificationsCapabilityAgent::executeSetIndicator(const NotificationIndicator& nextNotificationIndicator, shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG5(LX(__func__).d("currentState", m_currentState));
            switch (m_currentState) {
                case NotificationsCapabilityAgentState::PLAYING:
                    if (nextNotificationIndicator.asset.assetId == m_currentAssetId) {
                        ACSDK_WARN(LX("ignoringSetIndicatorDirective").d("incoming assetId matches current assetId", m_currentAssetId));
                        setHandlingCompleted(info);
                        return;
                    }
                case NotificationsCapabilityAgentState::IDLE: case NotificationsCapabilityAgentState::CANCELING_PLAY:
                    if (!m_notificationsStorage->enqueue(nextNotificationIndicator)) {
                        ACSDK_ERROR(LX("executeSetIndicatorFailed").d("reason", "failed to enqueue notification indicator"));
                        sendExceptionEncounteredAndReportFailed(info, "failed to store notification indicator in the queue");
                        return;
                    }
                    executePossibleIndicatorStateChange(intToIndicatorState(nextNotificationIndicator.persistVisualIndicator));
                    if (NotificationsCapabilityAgentState::IDLE == m_currentState) executeRenderNotification(nextNotificationIndicator);
                    break;
                case NotificationsCapabilityAgentState::SHUTTING_DOWN:
                    if (!m_notificationsStorage->enqueue(nextNotificationIndicator)) {
                        ACSDK_ERROR(LX("executeSetIndicatorFailed").d("reason", "failed to enqueue notification indicator"));
                        sendExceptionEncounteredAndReportFailed(info, "failed to store notification indicator in the queue");
                        return;
                    }
                    ACSDK_WARN(LX(__func__).m("notification indicator was queued while NotificationsCapabilityAgent was shutting down"));
                    break;
                case NotificationsCapabilityAgentState::SHUTDOWN:
                    ACSDK_ERROR(LX(__func__).m("SHUTDOWN while NotificationsCapabilityAgent was shutdown"));
                    sendExceptionEncounteredAndReportFailed(info,"failed to execute SetIndicator because NotificationsCapabilityAgent was shutdown");
                    return;
            }
            setHandlingCompleted(info);
            notifyObserversOfNotificationReceived();
            m_isEnabled = true;
            executeProvideState();
        }
        void NotificationsCapabilityAgent::handleClearIndicatorDirective(std::shared_ptr<DirectiveInfo> info) {
            m_executor.submit([this, info] { executeClearIndicator(info); });
        }
        void NotificationsCapabilityAgent::executeClearIndicator(std::shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG5(LX(__func__).d("currentState", m_currentState));
            int queueSize = 0;
            switch(m_currentState) {
                case NotificationsCapabilityAgentState::IDLE:
                    if (!m_notificationsStorage->getQueueSize(&queueSize)) { ACSDK_ERROR(LX(__func__).m("failed to get queue size")); }
                    if (queueSize > 0) { ACSDK_WARN(LX(__func__).m("expected queue size to be 0 when IDLE")); }
                    break;
                case NotificationsCapabilityAgentState::PLAYING:
                    executeSetState(NotificationsCapabilityAgentState::CANCELING_PLAY);
                    if (!m_renderer->cancelNotificationRendering()) {
                        ACSDK_ERROR(LX(__func__).m("failed to cancel notification rendering as a result of ClearIndicator directive"));
                    }
                    break;
                case NotificationsCapabilityAgentState::CANCELING_PLAY: break;
                case NotificationsCapabilityAgentState::SHUTTING_DOWN:
                    ACSDK_WARN(LX(__func__).m("attempting to process ClearIndicator directive while shutting down"));
                    break;
                case NotificationsCapabilityAgentState::SHUTDOWN:
                    ACSDK_WARN(LX(__func__).m("attempting to process ClearIndicator directive while already shutdown"));
                    sendExceptionEncounteredAndReportFailed(info,"failed to execute ClearIndicator because NotificationsCapabilityAgent was shutdown");
                    return;
            }
            if (!m_notificationsStorage->clearNotificationIndicators()) {
                ACSDK_ERROR(LX("executeClearIndicatorFailed").d("reason", "could not clear storage of NotificationIndicators"));
                sendExceptionEncounteredAndReportFailed(info, "failed to clear out NotificationIndicators");
            }
            setHandlingCompleted(info);
            m_isEnabled = false;
            executePossibleIndicatorStateChange(IndicatorState::OFF);
            executeProvideState();
        }
        void NotificationsCapabilityAgent::executeProvideState(bool sendToken, unsigned int stateRequestToken) {
            ACSDK_DEBUG5(LX("executeProvideState").d("sendToken", sendToken).d("stateRequestToken", stateRequestToken));
            auto policy = StateRefreshPolicy::ALWAYS;
            Document state(kObjectType);
            Value is_enabled_key{IS_ENABLED_KEY, strlen(IS_ENABLED_KEY)};
            Value _m_isEnabled{m_isEnabled};
            state.AddMember(is_enabled_key, _m_isEnabled);
            IndicatorState currentIndicatorState = IndicatorState::OFF;
            if (!m_notificationsStorage->getIndicatorState(&currentIndicatorState)) {
                ACSDK_ERROR(LX("executeProvideState").d("reason", "getIndicatorStateFailed"));
                submitMetric(m_metricRecorder, "getIndicatorStateFailed");
                return;
            }
            bool isVisualIndicatorPersisted = (currentIndicatorState == IndicatorState::ON);
            Value is_visual_indicator_persisted_key{IS_VISUAL_INDICATOR_PERSISTED_KEY, strlen(IS_VISUAL_INDICATOR_PERSISTED_KEY)};
            Value _isVisualIndicatorPersisted{isVisualIndicatorPersisted};
            state.AddMember(is_visual_indicator_persisted_key, _isVisualIndicatorPersisted);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!state.Accept(writer)) {
                ACSDK_ERROR(LX("executeProvideState").d("reason", "writerRefusedJsonObject"));
                return;
            }
            SetStateResult result;
            if (sendToken) result = m_contextManager->setState(INDICATOR_STATE_CONTEXT_KEY, buffer.GetString(), policy, stateRequestToken);
            else result = m_contextManager->setState(INDICATOR_STATE_CONTEXT_KEY, buffer.GetString(), policy);
            if (result != SetStateResult::SUCCESS) { ACSDK_ERROR(LX("executeProvideState").d("reason", "contextManagerSetStateFailed")); }
        }
        void NotificationsCapabilityAgent::executeStartQueueNotEmpty() {
            ACSDK_DEBUG5(LX(__func__));
            if (m_currentState != NotificationsCapabilityAgentState::IDLE) {
                ACSDK_ERROR(LX("executeStartQueueNotEmptyFailed").d("reason", "Expected to be in idle state before attempting to play")
                                .d("currentState", m_currentState));
                return;
            }
            NotificationIndicator nextNotificationIndicator;
            if (!m_notificationsStorage->peek(&nextNotificationIndicator)) {
                ACSDK_ERROR(LX("executeStartQueueNotEmptyFailed").d("reason", "peeking at the next notification in the queue failed."));
                return;
            }
            executePossibleIndicatorStateChange(intToIndicatorState(nextNotificationIndicator.persistVisualIndicator));
            executeRenderNotification(nextNotificationIndicator);
        }
        void NotificationsCapabilityAgent::onNotificationRenderingFinished() {
            ACSDK_DEBUG5(LX(__func__).d("currentAssetId", m_currentAssetId));
            m_executor.submit([this] { executeOnPlayFinished(); });
        }
        void NotificationsCapabilityAgent::executeOnPlayFinished() {
            int queueSize = 0;
            m_currentAssetId = "";
            if (!m_notificationsStorage->getQueueSize(&queueSize)) {
                ACSDK_ERROR(LX("executeOnPlayFinishedFailed").d("reason", "failed to retrieve queue size"));
                return;
            }
            if (queueSize < 0) {
                ACSDK_ERROR(LX("executeOnPlayFinishedFailed").d("unexpected queue size", queueSize));
                return;
            } else if (0 == queueSize) executePlayFinishedZeroQueued();
            else if (1 == queueSize) executePlayFinishedOneQueued();
            else executePlayFinishedMultipleQueued();
        }
        void NotificationsCapabilityAgent::executePlayFinishedZeroQueued() {
            ACSDK_DEBUG5(LX(__func__).d("currentState", m_currentState));
            switch(m_currentState) {
                case NotificationsCapabilityAgentState::IDLE: case NotificationsCapabilityAgentState::PLAYING:
                    ACSDK_WARN(LX(__func__).d("notification rendering finished in unexpected state", m_currentState));
                case NotificationsCapabilityAgentState::CANCELING_PLAY:
                    executeSetState(NotificationsCapabilityAgentState::IDLE);
                    break;
                case NotificationsCapabilityAgentState::SHUTTING_DOWN:
                    executeSetState(NotificationsCapabilityAgentState::SHUTDOWN);
                    break;
                case NotificationsCapabilityAgentState::SHUTDOWN:
                    ACSDK_WARN(LX(__func__).m("notification rendering finished while NotificationsCapabilityAgent was shutdown"));
                    break;
            }
            m_isEnabled = false;
            executeProvideState();
        }
        void NotificationsCapabilityAgent::executePlayFinishedOneQueued() {
            ACSDK_DEBUG5(LX(__func__).d("currentState", m_currentState));
            NotificationIndicator nextNotificationIndicator;
            switch(m_currentState) {
                case NotificationsCapabilityAgentState::IDLE: ACSDK_WARN(LX(__func__).d("unexpected state", m_currentState)); return;
                case NotificationsCapabilityAgentState::PLAYING:
                    if (!m_notificationsStorage->dequeue()) {
                        ACSDK_ERROR(LX("executePlayFinishedOneQueuedFailed").d("reason", "failed to dequeue NotificationIndicator"));
                    }
                    executeSetState(NotificationsCapabilityAgentState::IDLE);
                    m_isEnabled = false;
                    executeProvideState();
                    return;
                case NotificationsCapabilityAgentState::CANCELING_PLAY:
                    if (!m_notificationsStorage->peek(&nextNotificationIndicator)) {
                        ACSDK_ERROR(LX("executePlayFinishedOneQueuedFailed").m("Expected peek() to succeed"));
                        return;
                    }
                    executePossibleIndicatorStateChange(intToIndicatorState(nextNotificationIndicator.persistVisualIndicator));
                    executeRenderNotification(nextNotificationIndicator);
                    return;
                case NotificationsCapabilityAgentState::SHUTTING_DOWN:
                    executeSetState(NotificationsCapabilityAgentState::SHUTDOWN);
                    return;
                case NotificationsCapabilityAgentState::SHUTDOWN:
                    ACSDK_WARN(LX(__func__).m("notification rendering finished while NotificationsCapabilityAgent was shutdown"));
                    return;
            }
        }
        void NotificationsCapabilityAgent::executePlayFinishedMultipleQueued() {
            ACSDK_DEBUG5(LX(__func__).d("currentState", m_currentState));
            NotificationIndicator nextNotificationIndicator;
            switch (m_currentState) {
                case NotificationsCapabilityAgentState::IDLE:
                    ACSDK_WARN(LX(__func__).d("unexpected state", m_currentState));
                    return;
                case NotificationsCapabilityAgentState::PLAYING:
                    if (!m_notificationsStorage->dequeue()) {
                        ACSDK_ERROR(LX("executePlayFinishedMultipleQueuedFailed").d("reason", "failed to dequeue NotificationIndicator"));
                    }
                case NotificationsCapabilityAgentState::CANCELING_PLAY:
                    if (!m_notificationsStorage->peek(&nextNotificationIndicator)) {
                        ACSDK_ERROR(LX("executePlayFinishedMultipleQueuedFailed").m("Expected peek() to succeed"));
                        return;
                    }
                    executePossibleIndicatorStateChange(intToIndicatorState(nextNotificationIndicator.persistVisualIndicator));
                    executeRenderNotification(nextNotificationIndicator);
                    return;
                case NotificationsCapabilityAgentState::SHUTTING_DOWN:
                    executeSetState(NotificationsCapabilityAgentState::SHUTDOWN);
                    m_shutdownTrigger.notify_one();
                    return;
                case NotificationsCapabilityAgentState::SHUTDOWN:
                    ACSDK_WARN(LX(__func__).m("notification rendering finished while NotificationsCapabilityAgent was shutdown"));
                    return;
            }
        }
        void NotificationsCapabilityAgent::executeSetState(NotificationsCapabilityAgentState newState) {
            ACSDK_DEBUG5(LX(__func__).d("previousState", m_currentState).d("newState", newState));
            if (NotificationsCapabilityAgentState::IDLE == newState ||
                NotificationsCapabilityAgentState::SHUTTING_DOWN == newState) {
                m_currentAssetId = "";
            }
            m_currentState = newState;
        }
        DirectiveHandlerConfiguration NotificationsCapabilityAgent::getConfiguration() const {
            auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
            DirectiveHandlerConfiguration configuration;
            configuration[SET_INDICATOR] = neitherNonBlockingPolicy;
            configuration[CLEAR_INDICATOR] = neitherNonBlockingPolicy;
            return configuration;
        }
        void NotificationsCapabilityAgent::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
            if (info) {
                if (info->result) info->result->setCompleted();
                removeDirective(info->directive->getMessageId());
            }
        }
        bool NotificationsCapabilityAgent::parseDirectivePayload(
            shared_ptr<DirectiveInfo> info,
            Document* document) {
            ParseResult result = document->Parse(info->directive->getPayload().data());
            if (result) return true;
            ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", rapidjson::GetParseError_En(result.Code())).d("offset", result.Offset())
                .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }
        void NotificationsCapabilityAgent::doShutdown() {
            ACSDK_DEBUG5(LX(__func__));
            unique_lock<mutex> lock(m_shutdownMutex);
            m_executor.submit([this] { executeShutdown(); });
            bool successfulShutdown = m_shutdownTrigger.wait_for(lock, SHUTDOWN_TIMEOUT, [this]() { return m_currentState == NotificationsCapabilityAgentState::SHUTDOWN; });
            if (!successfulShutdown) {
                ACSDK_ERROR(LX("doShutdownFailed").d("reason", "transition to SHUTDOWN state timed out"));
            }
            m_executor.shutdown();
            m_renderer.reset();
            m_contextManager->setStateProvider(INDICATOR_STATE_CONTEXT_KEY, nullptr);
            m_contextManager.reset();
            m_notificationsStorage.reset();
        }
        void NotificationsCapabilityAgent::executeShutdown() {
            ACSDK_DEBUG5(LX(__func__).d("currentState", m_currentState));
            switch(m_currentState) {
                case NotificationsCapabilityAgentState::IDLE:
                    executeSetState(NotificationsCapabilityAgentState::SHUTDOWN);
                    m_shutdownTrigger.notify_one();
                    return;
                case NotificationsCapabilityAgentState::PLAYING:
                    if (!m_renderer->cancelNotificationRendering()) {
                        ACSDK_ERROR(LX(__func__).m("failed to cancel notification rendering during shutdown of NotificationsCapabilityAgent"));
                    }
                    executeSetState(NotificationsCapabilityAgentState::SHUTTING_DOWN);
                    return;
                case NotificationsCapabilityAgentState::CANCELING_PLAY:
                    executeSetState(NotificationsCapabilityAgentState::SHUTTING_DOWN);
                    return;
                case NotificationsCapabilityAgentState::SHUTTING_DOWN:
                    ACSDK_WARN(LX(__func__).m("executeShutdown called while already shutting down"));
                    return;
                case NotificationsCapabilityAgentState::SHUTDOWN:
                    ACSDK_WARN(LX(__func__).m("executeShutdown called while already shutdown"));
                    return;
            }
        }
        void NotificationsCapabilityAgent::clearData() {
            ACSDK_DEBUG5(LX(__func__));
            unique_lock<mutex> lock(m_shutdownMutex);
            if (m_currentState == NotificationsCapabilityAgentState::PLAYING) {
                if (!m_renderer->cancelNotificationRendering()) {
                    ACSDK_ERROR(LX(__func__).m("failed to cancel notification rendering during clearData"));
                }
            }
            if (m_currentState == NotificationsCapabilityAgentState::SHUTDOWN ||
                m_currentState == NotificationsCapabilityAgentState::SHUTTING_DOWN) {
                ACSDK_WARN(LX(__func__).m("should not be trying to clear data during shutdown."));
            } else executeSetState(NotificationsCapabilityAgentState::IDLE);
            auto result = m_executor.submit([this]() {
                m_notificationsStorage->clearNotificationIndicators();
                m_notificationsStorage->setIndicatorState(IndicatorState::OFF);
                notifyObserversOfIndicatorState(IndicatorState::OFF);
            });
            result.wait();
        }
        unordered_set<shared_ptr<CapabilityConfiguration>> NotificationsCapabilityAgent::
            getCapabilityConfigurations() {
            return m_capabilityConfigurations;
        }
    }
}
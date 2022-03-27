#include <algorithm>
#include <util/error/FinallyGuard.h>
#include <logger/Logger.h>
#include <metrics/MetricEventBuilder.h>
#include <metrics/DataPointCounterBuilder.h>
#include "ContextManager.h"

namespace alexaClientSDK {
    namespace contextManager {
        using namespace rapidjson;
        using namespace error;
        static const string TAG("ContextManager");
        #define LX(event) LogEntry(TAG, event)
        static const ContextRequestToken EMPTY_TOKEN = 0;
        static const string STATE_PROVIDER_TIMEOUT_METRIC_PREFIX = "ERROR.StateProviderTimeout.";
        shared_ptr<ContextManagerInterface> ContextManager::createContextManagerInterface(const shared_ptr<DeviceInfo>& deviceInfo,
                                                                                          const shared_ptr<MultiTimer>& multiTimer,
                                                                                          const shared_ptr<MetricRecorderInterface>& metricRecorder) {
            if (!deviceInfo) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullDeviceInfo"));
                return nullptr;
            }
            return create(*deviceInfo, multiTimer, metricRecorder);
        }
        shared_ptr<ContextManager> ContextManager::create(const DeviceInfo& deviceInfo, shared_ptr<MultiTimer> multiTimer,
                                                          shared_ptr<MetricRecorderInterface> metricRecorder) {
            if (!multiTimer) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullMultiTimer"));
                return nullptr;
            }
            shared_ptr<ContextManager> contextManager(new ContextManager(deviceInfo.getDefaultEndpointId(), multiTimer, metricRecorder));
            return contextManager;
        }
        ContextManager::~ContextManager() {
            m_shutdown = true;
            m_executor.shutdown();
            m_observers.clear();
            m_pendingRequests.clear();
            m_pendingStateRequest.clear();
        }
        static void NoopCallback() {}
        ContextManager::RequestTracker::RequestTracker() : timerToken{0}, contextRequester{nullptr}, skipReportableStateProperties{false} {}
        ContextManager::RequestTracker::RequestTracker(MultiTimer::Token timerToken, shared_ptr<ContextRequesterInterface> contextRequester,
                                                       bool skipReportableProperties) : timerToken{timerToken}, contextRequester{contextRequester},
                                                       skipReportableStateProperties{skipReportableProperties} {}
        void ContextManager::setStateProvider(const CapabilityTag& stateProviderName, shared_ptr<StateProviderInterface> stateProvider) {
            if (!stateProvider) removeStateProvider(move(stateProviderName));
            else addStateProvider(stateProviderName, move(stateProvider));
        }
        void ContextManager::addStateProvider(const CapabilityTag& capabilityIdentifier, shared_ptr<StateProviderInterface> stateProvider) {
            ACSDK_DEBUG5(LX(__func__).sensitive("capability", capabilityIdentifier));
            if (!stateProvider) {
                ACSDK_ERROR(LX("addStateProviderFailed").d("reason", "nullStateProvider"));
                return;
            }
            lock_guard<mutex> stateProviderLock(m_endpointsStateMutex);
            auto& endpointId = capabilityIdentifier.endpointId.empty() ? m_defaultEndpointId : capabilityIdentifier.endpointId;
            auto& capabilitiesState = m_endpointsState[endpointId];
            capabilitiesState[capabilityIdentifier] = StateInfo(move(stateProvider), Optional<CapabilityState>());
        }
        void ContextManager::removeStateProvider(const CapabilityTag& capabilityIdentifier) {
            ACSDK_DEBUG5(LX(__func__).sensitive("capability", capabilityIdentifier));
            lock_guard<mutex> stateProviderLock(m_endpointsStateMutex);
            auto& endpointId = capabilityIdentifier.endpointId.empty() ? m_defaultEndpointId : capabilityIdentifier.endpointId;
            auto& capabilitiesState = m_endpointsState[endpointId];
            capabilitiesState.erase(capabilityIdentifier);
        }
        SetStateResult ContextManager::setState(const CapabilityTag& capabilityIdentifier, const string& jsonState,
                                                const StateRefreshPolicy& refreshPolicy, const ContextRequestToken stateRequestToken) {
            ACSDK_DEBUG5(LX(__func__).sensitive("capability", capabilityIdentifier));
            if (EMPTY_TOKEN == stateRequestToken) {
                m_executor.submit([this, capabilityIdentifier, jsonState, refreshPolicy] {
                    updateCapabilityState(capabilityIdentifier, jsonState, refreshPolicy);
                });
                return SetStateResult::SUCCESS;
            }
            lock_guard<mutex> requestsLock{m_requestsMutex};
            auto requestIt = m_pendingStateRequest.find(stateRequestToken);
            if (requestIt == m_pendingStateRequest.end()) {
                ACSDK_ERROR(LX("setStateFailed").d("reason", "outdatedStateToken").sensitive("capability", capabilityIdentifier)
                    .sensitive("suppliedToken", stateRequestToken));
                return SetStateResult::STATE_TOKEN_OUTDATED;
            }
            auto capabilityIt = requestIt->second.find(capabilityIdentifier);
            if (capabilityIt == requestIt->second.end()) {
                ACSDK_ERROR(LX("setStateFailed").d("reason", "capabilityNotPending").sensitive("capability", capabilityIdentifier)
                    .sensitive("suppliedToken", stateRequestToken));
                return SetStateResult::STATE_PROVIDER_NOT_REGISTERED;
            }
            m_executor.submit([this, capabilityIdentifier, jsonState, refreshPolicy, stateRequestToken] {
                updateCapabilityState(capabilityIdentifier, jsonState, refreshPolicy);
                if (jsonState.empty() && (StateRefreshPolicy::ALWAYS == refreshPolicy)) {
                    ACSDK_ERROR(LX("setStateFailed").d("missingState", capabilityIdentifier.nameSpace + "::" + capabilityIdentifier.name));
                    function<void()> contextFailureCallback = NoopCallback;
                    {
                        lock_guard<mutex> requestsLock{m_requestsMutex};
                        contextFailureCallback = getContextFailureCallbackLocked(stateRequestToken, ContextRequestError::BUILD_CONTEXT_ERROR);
                    }
                    contextFailureCallback();
                } else {
                    function<void()> contextAvailableCallback = NoopCallback;
                    {
                        lock_guard<mutex> requestsLock{m_requestsMutex};
                        auto requestIt = m_pendingStateRequest.find(stateRequestToken);
                        if (requestIt != m_pendingStateRequest.end()) requestIt->second.erase(capabilityIdentifier);
                        contextAvailableCallback = getContextAvailableCallbackIfReadyLocked(stateRequestToken, "");
                    }
                    contextAvailableCallback();
                }
            });
            return SetStateResult::SUCCESS;
        }
        ContextManager::ContextManager(const string& defaultEndpointId, shared_ptr<MultiTimer> multiTimer, shared_ptr<MetricRecorderInterface> metricRecorder) :
                                       m_metricRecorder{move(metricRecorder)}, m_requestCounter{0}, m_shutdown{false},
                                       m_defaultEndpointId{defaultEndpointId}, m_multiTimer{multiTimer} {}
        void ContextManager::reportStateChange(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState,
                                               AlexaStateChangeCauseType cause) {
            ACSDK_DEBUG5(LX(__func__).sensitive("capability", capabilityIdentifier));
            m_executor.submit([this, capabilityIdentifier, capabilityState, cause] {
                updateCapabilityState(capabilityIdentifier, capabilityState);
                lock_guard<std::mutex> observerMutex{m_observerMutex};
                for (auto& observer : m_observers) {
                    observer->onStateChanged(capabilityIdentifier, capabilityState, cause);
                }
            });
        }
        void ContextManager::provideStateResponse(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState,
                                                  ContextRequestToken stateRequestToken) {
            ACSDK_DEBUG5(LX(__func__).sensitive("capability", capabilityIdentifier));
            m_executor.submit([this, capabilityIdentifier, capabilityState, stateRequestToken] {
                function<void()> contextAvailableCallback = NoopCallback;
                {
                    lock_guard<mutex> requestsLock{m_requestsMutex};
                    auto requestIt = m_pendingStateRequest.find(stateRequestToken);
                    if (requestIt == m_pendingStateRequest.end()) {
                        ACSDK_ERROR(LX("provideStateResponseFailed").d("reason", "outdatedStateToken").sensitive("capability", capabilityIdentifier)
                            .sensitive("suppliedToken", stateRequestToken));
                        return;
                    }
                    auto capabilityIt = requestIt->second.find(capabilityIdentifier);
                    if (capabilityIt == requestIt->second.end()) {
                        ACSDK_ERROR(LX("provideStateResponseFailed").d("reason", "capabilityNotPending").sensitive("capability", capabilityIdentifier)
                            .sensitive("suppliedToken", stateRequestToken));
                        return;
                    }
                    updateCapabilityState(capabilityIdentifier, capabilityState);
                    if (requestIt != m_pendingStateRequest.end()) requestIt->second.erase(capabilityIdentifier);
                    contextAvailableCallback =getContextAvailableCallbackIfReadyLocked(stateRequestToken, capabilityIdentifier.endpointId);
                }
                contextAvailableCallback();
            });
        }
        void ContextManager::provideStateUnavailableResponse(const CapabilityTag& capabilityIdentifier, ContextRequestToken stateRequestToken,
                                                             bool isEndpointUnreachable) {
            ACSDK_DEBUG5(LX(__func__).sensitive("capability", capabilityIdentifier));
            m_executor.submit([this, capabilityIdentifier, stateRequestToken, isEndpointUnreachable] {
                function<void()> contextAvailableCallback = NoopCallback;
                function<void()> contextFailureCallback = NoopCallback;
                {
                    lock_guard<mutex> requestsLock{m_requestsMutex};
                    auto requestIt = m_pendingStateRequest.find(stateRequestToken);
                    if (requestIt == m_pendingStateRequest.end()) {
                        ACSDK_ERROR(LX("provideStateUnavailableResponseFailed").d("reason", "outdatedStateToken")
                            .sensitive("capability", capabilityIdentifier).sensitive("suppliedToken", stateRequestToken));
                        return;
                    }
                    auto capabilityIt = requestIt->second.find(capabilityIdentifier);
                    if (capabilityIt == requestIt->second.end()) {
                        ACSDK_ERROR(LX("provideStateUnavailableResponseFailed").d("reason", "capabilityNotPending")
                            .sensitive("capability", capabilityIdentifier).sensitive("suppliedToken", stateRequestToken));
                        return;
                    }
                    if (!isEndpointUnreachable) {
                        auto& endpointState = m_endpointsState[capabilityIdentifier.endpointId];
                        auto cachedState = endpointState.find(capabilityIdentifier);
                        if ((cachedState != endpointState.end()) && cachedState->second.capabilityState.hasValue()) {
                            if (requestIt != m_pendingStateRequest.end()) requestIt->second.erase(capabilityIdentifier);
                            contextAvailableCallback = getContextAvailableCallbackIfReadyLocked(stateRequestToken, capabilityIdentifier.endpointId);
                        } else {
                            contextFailureCallback = getContextFailureCallbackLocked(stateRequestToken, ContextRequestError::BUILD_CONTEXT_ERROR);
                        }
                    } else {
                        contextFailureCallback = getContextFailureCallbackLocked(stateRequestToken, ContextRequestError::ENDPOINT_UNREACHABLE);
                    }
                }
                contextAvailableCallback();
                contextFailureCallback();
            });
        }
        void ContextManager::addContextManagerObserver(shared_ptr<ContextManagerObserverInterface> observer) {
            if (observer) {
                lock_guard<std::mutex> lock{m_observerMutex};
                m_observers.push_back(observer);
            }
        }
        void ContextManager::removeContextManagerObserver(const shared_ptr<ContextManagerObserverInterface>& observer) {
            lock_guard<mutex> lock{m_observerMutex};
            m_observers.remove(observer);
        }
        ContextRequestToken ContextManager::generateToken() {
            return ++m_requestCounter;
        }
        ContextRequestToken ContextManager::getContext(shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId,
                                                       const milliseconds& timeout) {
            ACSDK_DEBUG5(LX(__func__));
            return getContextInternal(contextRequester, endpointId, timeout, false);
        }
        ContextRequestToken ContextManager::getContextWithoutReportableStateProperties(shared_ptr<ContextRequesterInterface> contextRequester,
                                                                                       const string& endpointId, const milliseconds& timeout) {
            ACSDK_DEBUG5(LX(__func__));
            return getContextInternal(contextRequester, endpointId, timeout, true);
        }
        ContextRequestToken ContextManager::getContextInternal(shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId,
                                                               const milliseconds& timeout, bool bSkipReportableStateProperties) {
            ACSDK_DEBUG5(LX(__func__).sensitive("endpointId", endpointId));
            auto token = generateToken();
            m_executor.submit([this, contextRequester, endpointId, token, timeout, bSkipReportableStateProperties] {
                auto timerToken = m_multiTimer->submitTask(timeout, [this, token] {
                    m_executor.submit([this, token] {
                        function<void()> contextFailureCallback = NoopCallback;
                        {
                            lock_guard<mutex> lock{m_requestsMutex};
                            contextFailureCallback = getContextFailureCallbackLocked(token, ContextRequestError::STATE_PROVIDER_TIMEDOUT);
                        }
                        contextFailureCallback();
                    });
                });
                lock_guard<mutex> requestsLock{m_requestsMutex};
                auto& requestEndpointId = endpointId.empty() ? m_defaultEndpointId : endpointId;
                m_pendingRequests.emplace(token, RequestTracker(timerToken, contextRequester, bSkipReportableStateProperties));
                function<void()> contextAvailableCallback = NoopCallback;
                {
                    lock_guard<mutex> statesLock{m_endpointsStateMutex};
                    for (auto& capability : m_endpointsState[requestEndpointId]) {
                        auto stateInfo = capability.second;
                        auto stateProvider = capability.second.stateProvider;
                        if (stateProvider) {
                            bool requestState = false;
                            if (stateInfo.legacyCapability && stateInfo.refreshPolicy != StateRefreshPolicy::NEVER) {
                                requestState = true;
                            } else if (!stateInfo.legacyCapability && stateProvider->canStateBeRetrieved()) {
                                if (stateProvider->hasReportableStateProperties()) {
                                    if (!bSkipReportableStateProperties) requestState = true;
                                } else requestState = true;
                            }
                            if (requestState) {
                                stateProvider->provideState(capability.first, token);
                                m_pendingStateRequest[token].emplace(capability.first);
                            }
                        }
                    }
                    contextAvailableCallback = getContextAvailableCallbackIfReadyLocked(token, requestEndpointId);
                }
                contextAvailableCallback();
            });
            return token;
        }
        function<void()> ContextManager::getContextFailureCallbackLocked(unsigned int requestToken, ContextRequestError error) {
            ACSDK_DEBUG5(LX(__func__).d("token", requestToken));
            FinallyGuard clearRequestGuard{[this, requestToken] {
                auto requestIt = m_pendingRequests.find(requestToken);
                if (requestIt != m_pendingRequests.end()) {
                    m_multiTimer->cancelTask(requestIt->second.timerToken);
                    m_pendingRequests.erase(requestIt);
                }
                m_pendingStateRequest.erase(requestToken);
            }};
            auto& request = m_pendingRequests[requestToken];
            if (!request.contextRequester) {
                ACSDK_DEBUG0(LX(__func__).d("result", "nullRequester").d("token", requestToken));
                return NoopCallback;
            }
            for (auto& pendingState : m_pendingStateRequest[requestToken]) {
                auto metricName = STATE_PROVIDER_TIMEOUT_METRIC_PREFIX + pendingState.nameSpace;
                recordMetric(m_metricRecorder,MetricEventBuilder{}.setActivityName("CONTEXT_MANAGER-" + metricName)
                             .addDataPoint(DataPointCounterBuilder{}.setName(metricName).increment(1).build()).build());
            }
            auto contextRequester = request.contextRequester;
            return [contextRequester, error, requestToken]() {
                if (contextRequester) contextRequester->onContextFailure(error, requestToken);
            };
        }
        function<void()> ContextManager::getContextAvailableCallbackIfReadyLocked(unsigned int requestToken, const EndpointIdentifier& endpointId) {
            auto& pendingStates = m_pendingStateRequest[requestToken];
            if (!pendingStates.empty()) {
                ACSDK_DEBUG5(LX(__func__).d("result", "stateNotAvailableYet").d("pendingStates", pendingStates.size()));
                return NoopCallback;
            }
            ACSDK_DEBUG5(LX(__func__).sensitive("endpointId", endpointId).d("token", requestToken));
            FinallyGuard clearRequestGuard{[this, requestToken] {
                auto requestIt = m_pendingRequests.find(requestToken);
                if (requestIt != m_pendingRequests.end()) {
                    m_multiTimer->cancelTask(requestIt->second.timerToken);
                    m_pendingRequests.erase(requestIt);
                }
                m_pendingStateRequest.erase(requestToken);
            }};
            auto& request = m_pendingRequests[requestToken];
            if (!request.contextRequester) {
                ACSDK_ERROR(LX("getContextAvailableCallbackIfReadyLockedFailed").d("reason", "nullRequester").d("token", requestToken));
                return NoopCallback;
            }
            AVSContext context;
            auto& requestEndpointId = endpointId.empty() ? m_defaultEndpointId : endpointId;
            for (auto& capability : m_endpointsState[requestEndpointId]) {
                auto stateProvider = capability.second.stateProvider;
                auto stateInfo = capability.second;
                bool addState = false;
                if (stateInfo.legacyCapability) {
                    if ((stateInfo.refreshPolicy == StateRefreshPolicy::SOMETIMES) && !stateInfo.capabilityState.hasValue()) {
                        ACSDK_DEBUG5(LX(__func__).d("skipping state for legacy capabilityIdentifier", capability.first));
                    } else addState = true;
                } else {
                    if (stateProvider && stateProvider->canStateBeRetrieved()) {
                        if (stateProvider->hasReportableStateProperties()) {
                            if (!request.skipReportableStateProperties) addState = true;
                        } else addState = true;
                    }
                }
                if (addState) {
                    ACSDK_DEBUG5(LX(__func__).sensitive("addState", capability.first));
                    context.addState(capability.first, stateInfo.capabilityState.value());
                }
            }
            auto contextRequester = request.contextRequester;
            return [contextRequester, context, endpointId, requestToken]() {
                       if (contextRequester) contextRequester->onContextAvailable(endpointId, context, requestToken);
                   };
        }
        void ContextManager::updateCapabilityState(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState) {
            lock_guard<mutex> statesLock{m_endpointsStateMutex};
            auto& endpointId = capabilityIdentifier.endpointId.empty() ? m_defaultEndpointId : capabilityIdentifier.endpointId;
            auto& capabilitiesState = m_endpointsState[endpointId];
            auto& stateProvider = capabilitiesState[capabilityIdentifier].stateProvider;
            capabilitiesState[capabilityIdentifier] = StateInfo(stateProvider, capabilityState);
        }
        void ContextManager::updateCapabilityState(const CapabilityTag& capabilityIdentifier, const string& jsonState,
                                                   const StateRefreshPolicy& refreshPolicy) {
            lock_guard<std::mutex> statesLock{m_endpointsStateMutex};
            auto& endpointId = capabilityIdentifier.endpointId.empty() ? m_defaultEndpointId : capabilityIdentifier.endpointId;
            auto& capabilityInfo = m_endpointsState[endpointId];
            auto& stateProvider = capabilityInfo[capabilityIdentifier].stateProvider;
            capabilityInfo[capabilityIdentifier] = StateInfo(stateProvider, jsonState, refreshPolicy);
        }
        ContextManager::StateInfo::StateInfo(shared_ptr<StateProviderInterface> initStateProvider, const string& initJsonState,
                                             StateRefreshPolicy initRefreshPolicy) : stateProvider{initStateProvider},
                                             capabilityState{initJsonState.empty() ? Optional<CapabilityState>() : Optional<CapabilityState>(initJsonState)},
                                             legacyCapability{true}, refreshPolicy{initRefreshPolicy} {}
        ContextManager::StateInfo::StateInfo(shared_ptr<StateProviderInterface> initStateProvider, const Optional<CapabilityState>& initCapabilityState) :
                                             stateProvider{initStateProvider}, capabilityState{move(initCapabilityState)}, legacyCapability{false},
                                             refreshPolicy{StateRefreshPolicy::ALWAYS} {}
    }
}
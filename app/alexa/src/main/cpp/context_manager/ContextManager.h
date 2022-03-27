#ifndef ALEXA_CLIENT_SDK_CONTEXTMANAGER_INCLUDE_CONTEXTMANAGER_CONTEXTMANAGER_H_
#define ALEXA_CLIENT_SDK_CONTEXTMANAGER_INCLUDE_CONTEXTMANAGER_CONTEXTMANAGER_H_

#include <atomic>
#include <list>
#include <memory>
#include <utility>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <avs/CapabilityTag.h>
#include <avs/StateRefreshPolicy.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ContextRequesterInterface.h>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <sdkinterfaces/StateProviderInterface.h>
#include <util/DeviceInfo.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/Optional.h>
#include <threading/Executor.h>
#include <timing/MultiTimer.h>

namespace alexaClientSDK {
    namespace contextManager {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace endpoints;
        using namespace logger;
        using namespace metrics;
        using namespace timing;
        using namespace threading;
        class ContextManager : public ContextManagerInterface {
        public:
            static shared_ptr<ContextManagerInterface> createContextManagerInterface(const shared_ptr<DeviceInfo>& deviceInfo,
                                                                                     const shared_ptr<MultiTimer>& multiTimer = make_shared<MultiTimer>(),
                                                                                     const shared_ptr<MetricRecorderInterface>& metricRecorder = nullptr);
            static shared_ptr<ContextManager> create(const DeviceInfo& deviceInfo, shared_ptr<MultiTimer> multiTimer = make_shared<MultiTimer>(),
                                                     shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
            ~ContextManager() override;
            void setStateProvider(const CapabilityTag& stateProviderName, shared_ptr<StateProviderInterface> stateProvider) override;
            void addStateProvider(const CapabilityTag& capabilityIdentifier, shared_ptr<StateProviderInterface> stateProvider) override;
            void removeStateProvider(const CapabilityTag& capabilityIdentifier) override;
            SetStateResult setState(const CapabilityTag& stateProviderName, const string& jsonState, const StateRefreshPolicy& refreshPolicy,
                                    const ContextRequestToken stateRequestToken = 0) override;
            ContextRequestToken getContext(shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId,
                                           const milliseconds& timeout) override;
            ContextRequestToken getContextWithoutReportableStateProperties(shared_ptr<ContextRequesterInterface> contextRequester,
                                                                           const string& endpointId, const milliseconds& timeout) override;
            void reportStateChange(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState,
                                   AlexaStateChangeCauseType cause) override;
            void provideStateResponse(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState,
                                      ContextRequestToken stateRequestToken) override;
            void provideStateUnavailableResponse(const CapabilityTag& capabilityIdentifier, ContextRequestToken stateRequestToken,
                                                 bool isEndpointUnreachable) override;
            void addContextManagerObserver(shared_ptr<ContextManagerObserverInterface> observer) override;
            void removeContextManagerObserver(const shared_ptr<ContextManagerObserverInterface>& observer) override;
        private:
            struct StateInfo {
                shared_ptr<StateProviderInterface> stateProvider;
                Optional<CapabilityState> capabilityState;
                bool legacyCapability;
                avsCommon::avs::StateRefreshPolicy refreshPolicy;
                StateInfo(shared_ptr<StateProviderInterface> initStateProvider = nullptr, const string& initJsonState = "",
                          StateRefreshPolicy initRefreshPolicy = StateRefreshPolicy::ALWAYS);
                StateInfo(shared_ptr<StateProviderInterface> initStateProvider, const Optional<CapabilityState>& initCapabilityState);
                StateInfo(const StateInfo&) = default;
            };
            using CapabilitiesState = unordered_map<CapabilityTag, StateInfo>;
            struct RequestTracker {
                MultiTimer::Token timerToken;
                shared_ptr<ContextRequesterInterface> contextRequester;
                bool skipReportableStateProperties;
                RequestTracker();
                RequestTracker(MultiTimer::Token timerToken, shared_ptr<ContextRequesterInterface> contextRequester,
                               bool skipReportableProperties);
            };
        private:
            ContextManager(const string& defaultEndpointId, shared_ptr<MultiTimer> multiTimer, shared_ptr<MetricRecorderInterface> metricRecorder);
            void updateCapabilityState(const CapabilityTag& capabilityIdentifier, const string& jsonState, const StateRefreshPolicy& refreshPolicy);
            void updateCapabilityState(const CapabilityTag& capabilityIdentifier, const CapabilityState& capabilityState);
            ContextRequestToken getContextInternal(shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId,
                                                   const milliseconds& timeout, bool skipReportableStateProperties);
            function<void()> getContextAvailableCallbackIfReadyLocked(ContextRequestToken requestToken, const EndpointIdentifier& endpointId);
            function<void()> getContextFailureCallbackLocked(ContextRequestToken requestToken, ContextRequestError error);
            inline ContextRequestToken generateToken();
        private:
            mutex m_endpointsStateMutex;
            unordered_map<EndpointIdentifier, CapabilitiesState> m_endpointsState;
            mutex m_requestsMutex;
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            atomic<unsigned int> m_requestCounter;
            unordered_map<unsigned int, unordered_set<CapabilityTag>> m_pendingStateRequest;
            unordered_map<ContextRequestToken, RequestTracker> m_pendingRequests;
            mutex m_observerMutex;
            list<shared_ptr<ContextManagerObserverInterface>> m_observers;
            atomic_bool m_shutdown;
            const string m_defaultEndpointId;
            shared_ptr<MultiTimer> m_multiTimer;
            Executor m_executor;
        };
    }
}
#endif
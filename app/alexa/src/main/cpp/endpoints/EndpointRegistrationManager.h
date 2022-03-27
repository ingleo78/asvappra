#ifndef ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINTREGISTRATIONMANAGER_H_
#define ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINTREGISTRATIONMANAGER_H_

#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <sdkinterfaces/CapabilitiesDelegateInterface.h>
#include <sdkinterfaces/CapabilitiesObserverInterface.h>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <sdkinterfaces/Endpoints/EndpointInterface.h>
#include <sdkinterfaces/Endpoints/EndpointRegistrationManagerInterface.h>
#include <sdkinterfaces/Endpoints/EndpointRegistrationObserverInterface.h>
#include <threading/Executor.h>

namespace alexaClientSDK {
    namespace endpoints {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace threading;
        using namespace sdkInterfaces::endpoints;
        using State = CapabilitiesObserverInterface::State;
        using Error = CapabilitiesObserverInterface::Error;
        class EndpointRegistrationManager : public EndpointRegistrationManagerInterface {
        public:
            ~EndpointRegistrationManager();
            static unique_ptr<EndpointRegistrationManager> create(shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                                  shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                                                  const EndpointIdentifier& defaultEndpointId);
            void waitForPendingRegistrationsToEnqueue();
            future<RegistrationResult> registerEndpoint(shared_ptr<EndpointInterface> endpoint) override;
            future<DeregistrationResult> deregisterEndpoint(const EndpointIdentifier& endpointId) override;
            void addObserver(shared_ptr<EndpointRegistrationObserverInterface> observer) override;
            void removeObserver(const shared_ptr<EndpointRegistrationObserverInterface>& observer) override;
        private:
            using PendingRegistration = pair<shared_ptr<EndpointInterface>, promise<RegistrationResult>>;
            using PendingDeregistration = pair<shared_ptr<EndpointInterface>, promise<DeregistrationResult>>;
        public:
            class CapabilityRegistrationProxy : public CapabilitiesObserverInterface {
            public:
                void setCallback(function<void(const pair<RegistrationResult, vector<EndpointIdentifier>>& addedOrUpdatedEndpoints,
                                 const pair<DeregistrationResult, vector<EndpointIdentifier>>& deletedEndpoints)> callback);
                void onCapabilitiesStateChange(State newState, Error newError, const vector<EndpointIdentifier>& addedOrUpdatedEndpointIds,
                                               const vector<EndpointIdentifier>& deletedEndpointIds) override;
            private:
                function<void(const pair<RegistrationResult, vector<EndpointIdentifier>>& addedOrUpdatedEndpoints,
                         const pair<DeregistrationResult, vector<EndpointIdentifier>>& deletedEndpoints)> m_callback;
            };
        private:
            EndpointRegistrationManager(shared_ptr<DirectiveSequencerInterface> directiveSequencer, shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                        const EndpointIdentifier& defaultEndpointId);
            void executeRegisterEndpoint(const shared_ptr<EndpointInterface>& endpoint);
            void executeDeregisterEndpoint(const shared_ptr<EndpointInterface>& endpoint);
            bool addCapabilities(const shared_ptr<EndpointInterface>& endpoint, unordered_set<shared_ptr<DirectiveHandlerInterface>>* handlersAdded);
            bool addCapabilities(const shared_ptr<EndpointInterface>& endpoint);
            bool removeCapabilities(const shared_ptr<EndpointInterface>& endpoint, unordered_set<shared_ptr<DirectiveHandlerInterface>>* handlersRemoved);
            bool removeCapabilities(const shared_ptr<EndpointInterface>& endpoint);
            void updateAddedOrUpdatedEndpoints(const pair<RegistrationResult, vector<EndpointIdentifier>>& addedOrUpdatedEndpoints);
            void removeDeletedEndpoints(const pair<DeregistrationResult, vector<EndpointIdentifier>>& deletedEndpoints);
            void onCapabilityRegistrationStatusChanged(const pair<RegistrationResult, vector<EndpointIdentifier>>& addedOrUpdatedEndpoints,
                                                       const pair<DeregistrationResult, vector<EndpointIdentifier>>& deletedEndpoints);
            mutable mutex m_observersMutex;
            list<shared_ptr<EndpointRegistrationObserverInterface>> m_observers;
            shared_ptr<DirectiveSequencerInterface> m_directiveSequencer;
            shared_ptr<CapabilitiesDelegateInterface> m_capabilitiesDelegate;
            mutable mutex m_endpointsMutex;
            unordered_map<EndpointIdentifier, shared_ptr<EndpointInterface>> m_endpoints;
            unordered_map<EndpointIdentifier, PendingRegistration> m_pendingRegistrations;
            unordered_map<EndpointIdentifier, PendingDeregistration> m_pendingDeregistrations;
            const EndpointIdentifier m_defaultEndpointId;
            shared_ptr<CapabilityRegistrationProxy> m_capabilityRegistrationProxy;
            Executor m_executor;
        };
    }
}
#endif
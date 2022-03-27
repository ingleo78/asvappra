#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_CAPABILITIESDELEGATE_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_CAPABILITIESDELEGATE_H_

#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/CapabilitiesDelegateInterface.h>
#include <sdkinterfaces/CapabilitiesObserverInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <sdkinterfaces/PostConnectOperationProviderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <registration_manager/CustomerDataHandler.h>
#include <registration_manager/CustomerDataManager.h>
#include "Storage/CapabilitiesDelegateStorageInterface.h"
#include "DiscoveryEventSender.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace registrationManager;
        using namespace sdkInterfaces;
        using namespace endpoints;
        using namespace utils;
        using namespace threading;
        using namespace capabilitiesDelegate::storage;
        struct InProcessEndpointsToConfigMapStruct {
            unordered_map<string, string> pending;
            unordered_map<string, string> inFlight;
        };
        class CapabilitiesDelegate : public CapabilitiesDelegateInterface, public RequiresShutdown, public PostConnectOperationProviderInterface,
                                     public DiscoveryStatusObserverInterface, public CustomerDataHandler, public enable_shared_from_this<CapabilitiesDelegate> {
        public:
            static shared_ptr<CapabilitiesDelegate> create(const shared_ptr<AuthDelegateInterface>& authDelegate, const shared_ptr<CapabilitiesDelegateStorageInterface>& storage,
                                                           const shared_ptr<CustomerDataManager>& customerDataManager);
            bool addOrUpdateEndpoint(const AVSDiscoveryEndpointAttributes& endpointAttributes, const vector<CapabilityConfiguration>& capabilities) override;
            bool deleteEndpoint(const AVSDiscoveryEndpointAttributes& endpointAttributes, const vector<CapabilityConfiguration>& capabilities) override;
            void addCapabilitiesObserver(shared_ptr<CapabilitiesObserverInterface> observer) override;
            void removeCapabilitiesObserver(shared_ptr<CapabilitiesObserverInterface> observer) override;
            void invalidateCapabilities() override;
            void setMessageSender(const shared_ptr<MessageSenderInterface>& messageSender) override;
            void onAlexaEventProcessedReceived(const string& eventCorrelationToken) override;
            void doShutdown() override;
            shared_ptr<PostConnectOperationInterface> createPostConnectOperation() override;
            void onDiscoveryCompleted(const unordered_map<string, string>& addOrUpdateReportEndpoints, const unordered_map<string, string>& deleteReportEndpoints) override;
            void onDiscoveryFailure(MessageRequestObserverInterface::Status status) override;
            void onAVSGatewayChanged(const string& avsGateway) override;
            void clearData() override;
            void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;
            void addDiscoveryEventSender(const shared_ptr<DiscoveryEventSenderInterface>& discoveryEventSender);
        private:
            CapabilitiesDelegate(const shared_ptr<AuthDelegateInterface>& authDelegate, const shared_ptr<CapabilitiesDelegateStorageInterface>& storage,
                                 const shared_ptr<CustomerDataManager>& customerDataManager);
            bool init();
            string getAuthToken();
            void setCapabilitiesState(const CapabilitiesObserverInterface::State newState, const CapabilitiesObserverInterface::Error newError,
                                      const vector<EndpointIdentifier>& addOrUpdateReportEndpoints, const vector<EndpointIdentifier>& deleteReportEndpoints);
            bool updateEndpointConfigInStorage(const unordered_map<string, string>& addOrUpdateReportEndpoints, const unordered_map<string, string>& deleteReportEndpoints);
            void resetDiscoveryEventSender();
            void executeSendPendingEndpoints();
            bool isShuttingDown();
            void addStaleEndpointsToPendingDeleteLocked(unordered_map<string, string>* storedEndpointConfig);
            void filterUnchangedPendingAddOrUpdateEndpointsLocked(unordered_map<string, string>* storedEndpointConfig);
            void moveInFlightEndpointsToPendingLocked();
            void moveInFlightEndpointsToPending();
            void moveInFlightEndpointsToRegisteredEndpoints();
            mutex m_observerMutex;
            unordered_set<shared_ptr<CapabilitiesObserverInterface>> m_capabilitiesObservers;
            CapabilitiesObserverInterface::State m_capabilitiesState;
            CapabilitiesObserverInterface::Error m_capabilitiesError;
            shared_ptr<AuthDelegateInterface> m_authDelegate;
            shared_ptr<CapabilitiesDelegateStorageInterface> m_capabilitiesDelegateStorage;
            mutex m_isConnectedMutex;
            bool m_isConnected;
            mutex m_endpointsMutex;
            InProcessEndpointsToConfigMapStruct m_addOrUpdateEndpoints;
            InProcessEndpointsToConfigMapStruct m_deleteEndpoints;
            unordered_map<string, string> m_endpoints;
            mutex m_currentDiscoveryEventSenderMutex;
            shared_ptr<DiscoveryEventSenderInterface> m_currentDiscoveryEventSender;
            mutex m_messageSenderMutex;
            shared_ptr<MessageSenderInterface> m_messageSender;
            mutex m_isShuttingDownMutex;
            bool m_isShuttingDown;
            Executor m_executor;
        };
    }
}
#endif
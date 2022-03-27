#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_DISCOVERYEVENTSENDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_DISCOVERYEVENTSENDER_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <avs/WaitableMessageRequest.h>
#include <sdkinterfaces/AlexaEventProcessedObserverInterface.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/AuthObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/WaitEvent.h>
#include "DiscoveryEventSenderInterface.h"
#include "DiscoveryStatusObserverInterface.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        class DiscoveryEventSender : public DiscoveryEventSenderInterface, public enable_shared_from_this<DiscoveryEventSender> {
        public:
            static shared_ptr<DiscoveryEventSender> create(const unordered_map<string, string>& addOrUpdateReportEndpoints, const unordered_map<string, string>& deleteReportEndpoints,
                                                           const shared_ptr<AuthDelegateInterface>& authDelegate);
            ~DiscoveryEventSender();
            bool sendDiscoveryEvents(const shared_ptr<MessageSenderInterface>& messageSender) override;
            void stop() override;
            void addDiscoveryStatusObserver(const shared_ptr<DiscoveryStatusObserverInterface>& observer) override;
            void removeDiscoveryStatusObserver(const shared_ptr<DiscoveryStatusObserverInterface>& observer) override;
            void onAlexaEventProcessedReceived(const string& eventCorrelationToken) override;
            void onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) override;
        private:
            DiscoveryEventSender(
                const unordered_map<string, string>& addOrUpdateReportEndpoints,
                const unordered_map<string, string>& deleteReportEndpoints,
                const shared_ptr<AuthDelegateInterface>& authDelegate);
            bool sendDiscoveryEventWithRetries(const shared_ptr<MessageSenderInterface>& messageSender, const vector<string>& endpointConfigurations,
                                               bool isAddOrUpdateReportEvent = true);
            MessageRequestObserverInterface::Status sendDiscoveryEvent(const shared_ptr<MessageSenderInterface>& messageSender, const string& eventString,
                                                                       bool waitForEventProcessed = true);
            bool sendAddOrUpdateReportEvents(const shared_ptr<MessageSenderInterface>& messageSender);
            bool sendDeleteReportEvents(const shared_ptr<MessageSenderInterface>& messageSender);
            bool sendDiscoveryEvents(const vector<string>& endpointConfigurations, const shared_ptr<MessageSenderInterface>& messageSender,
                                     bool isAddOrUpdateReportEvent = true);
            void reportDiscoveryStatus(MessageRequestObserverInterface::Status status);
            string getAuthToken();
            bool isStopping();
            unordered_map<string, string> m_addOrUpdateReportEndpoints;
            unordered_map<string, string> m_deleteReportEndpoints;
            shared_ptr<AuthDelegateInterface> m_authDelegate;
            AuthObserverInterface::State m_currentAuthState;
            mutex m_authStatusMutex;
            condition_variable m_authStatusReady;
            string m_currentEventCorrelationToken;
            bool m_isStopping;
            WaitEvent m_eventProcessedWaitEvent;
            WaitEvent m_retryWait;
            mutex m_mutex;
            shared_ptr<WaitableMessageRequest> m_messageRequest;
            mutex m_observerMutex;
            shared_ptr<DiscoveryStatusObserverInterface> m_observer;
            mutex m_isSendDiscoveryEventsInvokedMutex;
            bool m_isSendDiscoveryEventsInvoked;
        };
    }
}
#endif
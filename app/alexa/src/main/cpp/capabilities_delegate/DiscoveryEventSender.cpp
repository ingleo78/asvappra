#include <logger/Logger.h>
#include <util/RetryTimer.h>
#include "Utils/DiscoveryUtils.h"
#include "DiscoveryEventSender.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace logger;
        using namespace capabilitiesDelegate::utils;
        static const string TAG("DiscoveryEventSender");
        #define LX(event) LogEntry(TAG, event)
        static const vector<int> RETRY_TABLE = { 1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000, 256000, };
        static RetryTimer RETRY_TIMER{RETRY_TABLE};
        static const int MAX_ENDPOINTS_PER_EVENT = 300;
        static const auto ASYNC_RESPONSE_TIMEOUT = seconds(2);
        shared_ptr<DiscoveryEventSender> DiscoveryEventSender::create(const unordered_map<string, string>& addOrUpdateReportEndpoints,
                                                                      const unordered_map<string, string>& deleteReportEndpoints,
                                                                      const shared_ptr<AuthDelegateInterface>& authDelegate) {
            if (addOrUpdateReportEndpoints.empty() && deleteReportEndpoints.empty()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "endpoint map empty"));
            } else if (!authDelegate) { ACSDK_ERROR(LX("createFailed").d("reason", "invalid auth delegate")); }
            else {
                auto instance = std::shared_ptr<DiscoveryEventSender>(new DiscoveryEventSender(addOrUpdateReportEndpoints, deleteReportEndpoints, authDelegate));
                return instance;
            }
            return nullptr;
        }
        DiscoveryEventSender::DiscoveryEventSender(const unordered_map<string, string>& addOrUpdateReportEndpoints, const unordered_map<string, string>& deleteReportEndpoints,
                                                   const shared_ptr<AuthDelegateInterface>& authDelegate) : m_addOrUpdateReportEndpoints{addOrUpdateReportEndpoints},
                                                   m_deleteReportEndpoints{deleteReportEndpoints}, m_authDelegate{authDelegate},
                                                   m_currentAuthState{AuthObserverInterface::State::UNINITIALIZED}, m_isStopping{false},
                                                   m_isSendDiscoveryEventsInvoked{false} {}
        DiscoveryEventSender::~DiscoveryEventSender() {
            stop();
        }
        bool DiscoveryEventSender::sendDiscoveryEvents(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__));
            {
                std::lock_guard<std::mutex> lock{m_isSendDiscoveryEventsInvokedMutex};
                if (m_isSendDiscoveryEventsInvoked) {
                    ACSDK_ERROR(LX("sendDiscoveryEventsFailed").m("sendDiscoveryEventsAlreadyInvoked"));
                    return false;
                }
                m_isSendDiscoveryEventsInvoked = true;
            }
            if (!messageSender) {
                ACSDK_ERROR(LX("sendDiscoveryEventsFailed").d("reason", "invalid message sender"));
                return false;
            }
            m_authDelegate->addAuthObserver(shared_from_this());
            bool result = false;
            if (sendAddOrUpdateReportEvents(messageSender) && sendDeleteReportEvents(messageSender)) {
                result = true;
                reportDiscoveryStatus(MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED);
            }
            m_authDelegate->removeAuthObserver(shared_from_this());
            return result;
        }
        void DiscoveryEventSender::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) {
            ACSDK_DEBUG5(LX(__func__).d("state", newState).d("error", newError));
            lock_guard<mutex> lock(m_authStatusMutex);
            m_currentAuthState = newState;
            if ((AuthObserverInterface::State::REFRESHED == m_currentAuthState)) m_authStatusReady.notify_one();
        }
        string DiscoveryEventSender::getAuthToken() {
            {
                unique_lock<mutex> lock(m_authStatusMutex);
                m_authStatusReady.wait(lock, [this]() { return (isStopping() || (AuthObserverInterface::State::REFRESHED == m_currentAuthState)); });
                if (isStopping()) {
                    ACSDK_DEBUG0(LX("getAuthTokenFailed").d("reason", "shutdownWhileWaitingForToken"));
                    return "";
                }
            }
            return m_authDelegate->getAuthToken();
        }
        void DiscoveryEventSender::stop() {
            ACSDK_DEBUG5(LX(__func__));
            shared_ptr<WaitableMessageRequest> requestCopy;
            {
                lock_guard<mutex> lock{m_mutex};
                if (m_isStopping) return;
                m_isStopping = true;
                requestCopy = m_messageRequest;
            }
            if (requestCopy) requestCopy->shutdown();
            {
                lock_guard<mutex> lock{m_authStatusMutex};
                m_authStatusReady.notify_one();
            }
            m_retryWait.wakeUp();
            m_eventProcessedWaitEvent.wakeUp();
            {
                lock_guard<mutex> lock{m_observerMutex};
                m_observer.reset();
            }
        }
        bool DiscoveryEventSender::isStopping() {
            lock_guard<std::mutex> lock{m_mutex};
            return m_isStopping;
        }
        MessageRequestObserverInterface::Status DiscoveryEventSender::sendDiscoveryEvent(
            const shared_ptr<MessageSenderInterface>& messageSender,
            const string& eventString,
            bool waitForEventProcessed) {
            ACSDK_DEBUG5(LX(__func__));
            unique_lock<mutex> lock{m_mutex};
            m_eventProcessedWaitEvent.reset();
            m_messageRequest.reset();
            m_messageRequest = make_shared<WaitableMessageRequest>(eventString);
            lock.unlock();
            messageSender->sendMessage(m_messageRequest);
            auto status = m_messageRequest->waitForCompletion();
            ACSDK_DEBUG5(LX(__func__).d("Discovery event status", status));
            if (MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED == status && waitForEventProcessed) {
                ACSDK_DEBUG5(LX(__func__).m("waiting for Event Processed directive"));
                if (!m_eventProcessedWaitEvent.wait(ASYNC_RESPONSE_TIMEOUT)) {
                    ACSDK_ERROR(LX("sendDiscoveryEventFailed").d("reason", "Timeout on waiting for Event Processed Directive"));
                    status = MessageRequestObserverInterface::Status::TIMEDOUT;
                } else if (isStopping()) status = MessageRequestObserverInterface::Status::CANCELED;
            }
            return status;
        }
        bool DiscoveryEventSender::sendDiscoveryEventWithRetries(const shared_ptr<MessageSenderInterface>& messageSender, const vector<string>& endpointConfigurations,
                                                                 bool isAddOrUpdateReportEvent) {
            ACSDK_DEBUG5(LX(__func__));
            m_retryWait.reset();
            int retryAttempt = 0;
            while(!isStopping()) {
                string eventString, authToken;
                authToken = getAuthToken();
                if (authToken.empty()) {
                    ACSDK_ERROR(LX("sendDiscoveryEventWithRetriesFailed").d("reason", "empty auth token"));
                    return false;
                }
                if (isAddOrUpdateReportEvent) {
                    auto eventAndEventCorrelationTokenPair = getAddOrUpdateReportEventJson(endpointConfigurations, authToken);
                    eventString = eventAndEventCorrelationTokenPair.first;
                    m_currentEventCorrelationToken = eventAndEventCorrelationTokenPair.second;
                } else eventString = getDeleteReportEventJson(endpointConfigurations, authToken);
                auto status = sendDiscoveryEvent(messageSender, eventString, isAddOrUpdateReportEvent);
                switch(status) {
                    case MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED: return true;
                    case MessageRequestObserverInterface::Status::INVALID_AUTH: m_authDelegate->onAuthFailure(authToken);
                    case MessageRequestObserverInterface::Status::BAD_REQUEST:
                        reportDiscoveryStatus(status);
                        return false;
                    default:
                        reportDiscoveryStatus(status);
                        break;
                }
                if (m_retryWait.wait(RETRY_TIMER.calculateTimeToRetry(retryAttempt++))) {
                    ACSDK_DEBUG5((LX(__func__).m("aborting operation")));
                    return false;
                }
            }
            return false;
        }
        bool DiscoveryEventSender::sendDiscoveryEvents(const vector<string>& endpointConfigurations, const shared_ptr<MessageSenderInterface>& messageSender,
                                                       bool isAddOrUpdateReportEvent) {
            int numFullEndpoints = endpointConfigurations.size() / MAX_ENDPOINTS_PER_EVENT;
            auto it = endpointConfigurations.begin();
            for (int num = 0; num < numFullEndpoints; ++num) {
                vector<string> endpointConfigs(it, it + MAX_ENDPOINTS_PER_EVENT);
                if (!sendDiscoveryEventWithRetries(messageSender, endpointConfigs, isAddOrUpdateReportEvent)) return false;
                it += MAX_ENDPOINTS_PER_EVENT;
            }
            vector<string> endpointConfigs(it, endpointConfigurations.end());
            return sendDiscoveryEventWithRetries(messageSender, endpointConfigs, isAddOrUpdateReportEvent);
        }
        bool DiscoveryEventSender::sendAddOrUpdateReportEvents(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__).d("num endpoints", m_addOrUpdateReportEndpoints.size()));
            if (m_addOrUpdateReportEndpoints.empty()) {
                ACSDK_DEBUG5(LX(__func__).m("endpoints list empty"));
                return true;
            }
            vector<string> allEndpointConfigs;
            for (const auto& endpointIdToConfigPair : m_addOrUpdateReportEndpoints) allEndpointConfigs.push_back(endpointIdToConfigPair.second);
            return sendDiscoveryEvents(allEndpointConfigs, messageSender, true);
        }
        bool DiscoveryEventSender::sendDeleteReportEvents(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__).d("num endpoints", m_deleteReportEndpoints.size()));
            if (m_deleteReportEndpoints.empty()) {
                ACSDK_DEBUG5(LX(__func__).m("endpoints list empty"));
                return true;
            }
            vector<string> allEndpointConfigs;
            for (const auto& endpointIdToConfigPair : m_deleteReportEndpoints) allEndpointConfigs.push_back(endpointIdToConfigPair.second);
            return sendDiscoveryEvents(allEndpointConfigs, messageSender, false);
        }

        void DiscoveryEventSender::onAlexaEventProcessedReceived(const std::string& eventCorrelationToken) {
            ACSDK_DEBUG5(LX(__func__));
            if (m_currentEventCorrelationToken == eventCorrelationToken) {
                ACSDK_DEBUG5(LX(__func__).m("valid event correlation token received"));
                m_eventProcessedWaitEvent.wakeUp();
            } else { ACSDK_WARN(LX(__func__).m("invalid event correlation token received")); }
        }
        void DiscoveryEventSender::addDiscoveryStatusObserver(
            const shared_ptr<DiscoveryStatusObserverInterface>& observer) {
            ACSDK_DEBUG5(LX(__func__));
            if (!observer) {
                ACSDK_ERROR(LX("addDiscoveryObserverFailed").d("reason", "nullObserver"));
                return;
            }
            lock_guard<mutex> lock(m_observerMutex);
            m_observer.reset();
            m_observer = observer;
        }
        void DiscoveryEventSender::removeDiscoveryStatusObserver(
            const shared_ptr<DiscoveryStatusObserverInterface>& observer) {
            ACSDK_DEBUG5(LX(__func__));
            if (!observer) {
                ACSDK_ERROR(LX("removeDiscoveryObserverFailed").d("reason", "nullObserver"));
                return;
            }
            lock_guard<std::mutex> lock(m_observerMutex);
            if (m_observer == observer) m_observer.reset();
            else { ACSDK_ERROR(LX("removeObserverFailed").d("reason", "observer not found")); }
        }
        void DiscoveryEventSender::reportDiscoveryStatus(MessageRequestObserverInterface::Status status) {
            ACSDK_DEBUG5(LX(__func__));
            shared_ptr<DiscoveryStatusObserverInterface> observer;
            {
                lock_guard<mutex> lock{m_observerMutex};
                observer = m_observer;
            }
            if (!observer) return;
            if (status == MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED) {
                observer->onDiscoveryCompleted(m_addOrUpdateReportEndpoints, m_deleteReportEndpoints);
            } else observer->onDiscoveryFailure(status);
        }
    }
}
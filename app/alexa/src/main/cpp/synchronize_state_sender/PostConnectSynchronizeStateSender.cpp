#include <avs/EventBuilder.h>
#include <logger/Logger.h>
#include <util/RetryTimer.h>
#include "PostConnectSynchronizeStateSender.h"

namespace alexaClientSDK {
    namespace synchronizeStateSender {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        static const string TAG("PostConnectSynchronizeStateSender");
        #define LX(event) LogEntry(TAG, event)
        static const string SYNCHRONIZE_STATE_NAMESPACE = "System";
        static const string SYNCHRONIZE_STATE_NAME = "SynchronizeState";
        static const vector<int> RETRY_TABLE = {
            250,    // Retry 1:  0.25s
            1000,   // Retry 2:  1.00s
            3000,   // Retry 3:  3.00s
            5000,   // Retry 4:  5.00s
            10000,  // Retry 5: 10.00s
            20000,  // Retry 6: 20.00s
        };
        static RetryTimer RETRY_TIMER{RETRY_TABLE};
        milliseconds CONTEXT_FETCH_TIMEOUT = milliseconds(2000);
        shared_ptr<PostConnectSynchronizeStateSender> PostConnectSynchronizeStateSender::create(shared_ptr<ContextManagerInterface> contextManager) {
            if (!contextManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
            } else return shared_ptr<PostConnectSynchronizeStateSender>(new PostConnectSynchronizeStateSender(contextManager));
            return nullptr;
        }
        PostConnectSynchronizeStateSender::PostConnectSynchronizeStateSender(shared_ptr<ContextManagerInterface> contextManager) : m_contextManager{contextManager},
                                                                             m_isStopping{false} {}
        unsigned int PostConnectSynchronizeStateSender::getOperationPriority() {
            return SYNCHRONIZE_STATE_PRIORITY;
        }
        void PostConnectSynchronizeStateSender::onContextFailure(const ContextRequestError error) {
            ACSDK_ERROR(LX(__func__).d("reason", error));
            m_wakeTrigger.notify_all();
        }
        void PostConnectSynchronizeStateSender::onContextAvailable(const string& jsonContext) {
            ACSDK_DEBUG5(LX(__func__));
            {
                lock_guard<mutex> lock{m_mutex};
                m_contextString = jsonContext;
            }
            m_wakeTrigger.notify_all();
        }
        bool PostConnectSynchronizeStateSender::fetchContext() {
            ACSDK_DEBUG5(LX(__func__));
            unique_lock<mutex> lock{m_mutex};
            m_contextString = "";
            m_contextManager->getContext(shared_from_this(), m_contextString, CONTEXT_FETCH_TIMEOUT);
            auto pred = [this] { return !m_contextString.empty() || m_isStopping; };
            if (!m_wakeTrigger.wait_for(lock, CONTEXT_FETCH_TIMEOUT, pred)) {
                ACSDK_DEBUG5(LX(__func__).d("reason", "context fetch timeout"));
                return false;
            }
            if (m_contextString.empty()) {
                ACSDK_ERROR(LX(__func__).m("invalid context received."));
                return false;
            }
            if (m_isStopping) {
                ACSDK_DEBUG5(LX(__func__).m("Stopped while context fetch in progress"));
                return false;
            }
            return true;
        }
        bool PostConnectSynchronizeStateSender::performOperation(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__));
            if (!messageSender) {
                ACSDK_ERROR(LX("performOperationFailed").d("reason", "nullPostConnectSender"));
                return false;
            }
            int retryAttempt = 0;
            while (!isStopping()) {
                if (fetchContext()) {
                    unique_lock<mutex> lock{m_mutex};
                    if (m_isStopping) return false;
                    auto event = buildJsonEventString(SYNCHRONIZE_STATE_NAMESPACE, SYNCHRONIZE_STATE_NAME, "", "{}",
                                                      m_contextString);
                    m_postConnectRequest = make_shared<WaitableMessageRequest>(event.second);
                    lock.unlock();
                    messageSender->sendMessage(m_postConnectRequest);
                    auto status = m_postConnectRequest->waitForCompletion();
                    ACSDK_DEBUG5(LX(__func__).d("SynchronizeState event status", status));
                    if (status == MessageRequestObserverInterface::Status::SUCCESS || status == MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT) {
                        return true;
                    } else if (status == MessageRequestObserverInterface::Status::CANCELED) {
                        return false;
                    }
                }
                unique_lock<mutex> lock{m_mutex};
                auto timeout = RETRY_TIMER.calculateTimeToRetry(retryAttempt++);
                if (m_wakeTrigger.wait_for(lock, timeout, [this] { return m_isStopping; })) {
                    return false;
                }
            }
            return false;
        }
        bool PostConnectSynchronizeStateSender::isStopping() {
            lock_guard<mutex> lock{m_mutex};
            return m_isStopping;
        }
        void PostConnectSynchronizeStateSender::abortOperation() {
            ACSDK_DEBUG5(LX(__func__));
            shared_ptr<WaitableMessageRequest> requestCopy;
            {
                lock_guard<mutex> lock{m_mutex};
                if (m_isStopping) return;
                m_isStopping = true;
                requestCopy = m_postConnectRequest;
            }
            if (requestCopy) requestCopy->shutdown();
            m_wakeTrigger.notify_all();
        }
    }
}

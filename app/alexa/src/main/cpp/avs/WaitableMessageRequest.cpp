#include <logger/Logger.h>
#include "WaitableMessageRequest.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace chrono;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            static const string TAG("WaitableMessageRequest");
            #define LX(event) LogEntry(TAG, event)
            static const auto CONNECTION_TIMEOUT = seconds(15);
            WaitableMessageRequest::WaitableMessageRequest(const string& jsonContent) : MessageRequest{jsonContent},
                                                           m_sendMessageStatus{MessageRequestObserverInterface::Status::TIMEDOUT}, m_responseReceived{false},
                                                           m_isRequestShuttingDown{false} {}
            void WaitableMessageRequest::sendCompleted(MessageRequestObserverInterface::Status sendMessageStatus) {
                MessageRequest::sendCompleted(sendMessageStatus);
                lock_guard<mutex> lock{m_requestMutex};
                if (!m_responseReceived) {
                    m_sendMessageStatus = sendMessageStatus;
                    m_responseReceived = true;
                } else { ACSDK_WARN(LX(__func__).d("reason", "sendCompletedCalled when m_responseReceived")); }
                m_requestCv.notify_one();
            }
            MessageRequestObserverInterface::Status WaitableMessageRequest::waitForCompletion() {
                unique_lock<mutex> lock(m_requestMutex);
                m_requestCv.wait_for(lock, CONNECTION_TIMEOUT, [this] { return m_isRequestShuttingDown || m_responseReceived; });
                return m_sendMessageStatus;
            }
            void WaitableMessageRequest::shutdown() {
                lock_guard<std::mutex> lock{m_requestMutex};
                m_isRequestShuttingDown = true;
                m_sendMessageStatus = MessageRequestObserverInterface::Status::CANCELED;
                m_requestCv.notify_one();
            }
        }
    }
}
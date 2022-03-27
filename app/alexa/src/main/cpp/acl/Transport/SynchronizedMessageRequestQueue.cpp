#include <avs/MessageRequest.h>
#include <logger/Logger.h>
#include "SynchronizedMessageRequestQueue.h"
#include "TestableConsumer.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace avsCommon::avs;
        static const std::string TAG("SynchronizedMessageRequestQueue");
        #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
        SynchronizedMessageRequestQueue::~SynchronizedMessageRequestQueue() {
            clearWaitingForSendAcknowledgement();
            clear();
        }
        void SynchronizedMessageRequestQueue::enqueueRequest(std::shared_ptr<MessageRequest> messageRequest) {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_requestQueue.enqueueRequest(std::move(messageRequest));
        }
        avsCommon::utils::Optional<std::chrono::time_point<std::chrono::steady_clock>> SynchronizedMessageRequestQueue::
            peekRequestTime() {
            std::lock_guard<std::mutex> lock{m_mutex};
            return m_requestQueue.peekRequestTime();
        }
        std::shared_ptr<MessageRequest> SynchronizedMessageRequestQueue::dequeueOldestRequest() {
            std::lock_guard<std::mutex> lock{m_mutex};
            return m_requestQueue.dequeueOldestRequest();
        }
        std::shared_ptr<MessageRequest> SynchronizedMessageRequestQueue::dequeueSendableRequest() {
            std::lock_guard<std::mutex> lock{m_mutex};
            return m_requestQueue.dequeueSendableRequest();
        }
        bool SynchronizedMessageRequestQueue::isMessageRequestAvailable() const {
            std::lock_guard<std::mutex> lock{m_mutex};
            return m_requestQueue.isMessageRequestAvailable();
        }
        void SynchronizedMessageRequestQueue::setWaitingForSendAcknowledgement() {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_requestQueue.setWaitingForSendAcknowledgement();
        }
        void SynchronizedMessageRequestQueue::clearWaitingForSendAcknowledgement() {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_requestQueue.clearWaitingForSendAcknowledgement();
        }
        bool SynchronizedMessageRequestQueue::empty() const {
            std::lock_guard<std::mutex> lock{m_mutex};
            return m_requestQueue.empty();
        }
        void SynchronizedMessageRequestQueue::clear() {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_requestQueue.clear();
        }
    }
}
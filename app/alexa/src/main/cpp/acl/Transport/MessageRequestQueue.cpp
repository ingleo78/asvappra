#include <avs/MessageRequest.h>
#include <logger/Logger.h>
#include "MessageRequestQueue.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace logger;
        static const string TAG("MessageRequestQueue");
        static const string EMPTY_QUEUE_NAME = "";
        #define LX(event) LogEntry(TAG, event)
        MessageRequestQueue::MessageRequestQueue() : m_isWaitingForAcknowledgement{false} {}
        void MessageRequestQueue::enqueueRequest(shared_ptr<MessageRequest> messageRequest) {
            if (messageRequest != nullptr) m_queue.push_back({steady_clock::now(), messageRequest});
            else { ACSDK_ERROR(LX("enqueueRequest").d("reason", "nullMessageRequest")); }
        }
        Optional<time_point<steady_clock>> MessageRequestQueue::peekRequestTime() {
            if (!m_queue.empty()) return m_queue.front().first;
            return Optional<time_point<steady_clock>>();
        }
        shared_ptr<MessageRequest> MessageRequestQueue::dequeueOldestRequest() {
            if (m_queue.empty()) return nullptr;
            auto result = m_queue.front().second;
            m_queue.pop_front();
            return result;
        }
        shared_ptr<MessageRequest> MessageRequestQueue::dequeueSendableRequest() {
            for (auto it = m_queue.begin(); it != m_queue.end(); it++) {
                if (!m_isWaitingForAcknowledgement || !it->second->getIsSerialized()) {
                    auto result = it->second;
                    m_queue.erase(it);
                    return result;
                }
            }
            return nullptr;
        }
        bool MessageRequestQueue::isMessageRequestAvailable() const {
            for (auto it = m_queue.begin(); it != m_queue.end(); it++) {
                if (!m_isWaitingForAcknowledgement || !it->second->getIsSerialized()) return true;
            }
            return false;
        }
        void MessageRequestQueue::setWaitingForSendAcknowledgement() {
            m_isWaitingForAcknowledgement = true;
        }
        void MessageRequestQueue::clearWaitingForSendAcknowledgement() {
            m_isWaitingForAcknowledgement = false;
        }
        bool MessageRequestQueue::empty() const {
            return m_queue.empty();
        }
        void MessageRequestQueue::clear() {
            m_queue.clear();
        }
    }
}
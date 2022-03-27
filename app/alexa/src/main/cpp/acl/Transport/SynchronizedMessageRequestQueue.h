#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_SYNCHRONIZEDMESSAGEREQUESTQUEUE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_SYNCHRONIZEDMESSAGEREQUESTQUEUE_H_

#include <deque>
#include <memory>
#include <unordered_map>
#include <avs/MessageRequest.h>
#include <functional/hash.h>
#include "MessageRequestQueue.h"
#include "MessageRequestQueueInterface.h"

namespace alexaClientSDK {
    namespace acl {
        class SynchronizedMessageRequestQueue : public MessageRequestQueueInterface {
        public:
            SynchronizedMessageRequestQueue() = default;
            ~SynchronizedMessageRequestQueue() override;
            void enqueueRequest(std::shared_ptr<avsCommon::avs::MessageRequest> messageRequest) override;
            avsCommon::utils::Optional<std::chrono::time_point<std::chrono::steady_clock>> peekRequestTime() override;
            std::shared_ptr<avsCommon::avs::MessageRequest> dequeueOldestRequest() override;
            std::shared_ptr<avsCommon::avs::MessageRequest> dequeueSendableRequest() override;
            bool isMessageRequestAvailable() const override;
            void setWaitingForSendAcknowledgement() override;
            void clearWaitingForSendAcknowledgement() override;
            bool empty() const override;
            void clear() override;
        private:
            mutable std::mutex m_mutex;
            MessageRequestQueue m_requestQueue;
        };
    }
}
#endif
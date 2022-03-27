#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEREQUESTQUEUE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEREQUESTQUEUE_H_

#include <deque>
#include <memory>
#include <unordered_map>
#include <utility>
#include <avs/MessageRequest.h>
#include <functional/hash.h>
#include "MessageRequestQueueInterface.h"

namespace alexaClientSDK {
    namespace acl {
        class MessageRequestQueue : public MessageRequestQueueInterface {
        public:
            MessageRequestQueue();
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
            bool m_isWaitingForAcknowledgement;
            std::deque<std::pair<std::chrono::time_point<std::chrono::steady_clock>, std::shared_ptr<avsCommon::avs::MessageRequest>>> m_queue;
        };
    }
}
#endif
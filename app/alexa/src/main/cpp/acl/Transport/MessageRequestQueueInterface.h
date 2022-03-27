#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEREQUESTQUEUEINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEREQUESTQUEUEINTERFACE_H_

#include <chrono>
#include <deque>
#include <memory>
#include <unordered_map>
#include <avs/MessageRequest.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace acl {
        class MessageRequestQueueInterface {
        public:
            virtual ~MessageRequestQueueInterface() = default;
            virtual void enqueueRequest(std::shared_ptr<avsCommon::avs::MessageRequest> messageRequest) = 0;
            virtual avsCommon::utils::Optional<std::chrono::time_point<std::chrono::steady_clock>> peekRequestTime() = 0;
            virtual std::shared_ptr<avsCommon::avs::MessageRequest> dequeueOldestRequest() = 0;
            virtual std::shared_ptr<avsCommon::avs::MessageRequest> dequeueSendableRequest() = 0;
            virtual bool isMessageRequestAvailable() const = 0;
            virtual void setWaitingForSendAcknowledgement() = 0;
            virtual void clearWaitingForSendAcknowledgement() = 0;
            virtual bool empty() const = 0;
            virtual void clear() = 0;
        };
    }
}
#endif
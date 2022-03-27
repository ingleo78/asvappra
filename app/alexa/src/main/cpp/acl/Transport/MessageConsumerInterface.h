#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGECONSUMERINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGECONSUMERINTERFACE_H_

#include <memory>

namespace alexaClientSDK {
    namespace acl {
        class MessageConsumerInterface {
        public:
            virtual ~MessageConsumerInterface() = default;
            virtual void consumeMessage(const std::string& contextId, const std::string& message) = 0;
        };
    }
}
#endif
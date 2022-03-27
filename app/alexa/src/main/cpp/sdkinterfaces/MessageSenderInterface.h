#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MESSAGESENDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MESSAGESENDERINTERFACE_H_

#include <avs/MessageRequest.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class MessageSenderInterface {
            public:
                virtual ~MessageSenderInterface() = default;
                virtual void sendMessage(std::shared_ptr<avs::MessageRequest> request);
            };
        }
    }
}
#endif
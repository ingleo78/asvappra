#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POSTCONNECTOPERATIONINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POSTCONNECTOPERATIONINTERFACE_H_

#include <memory>
#include "MessageSenderInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class PostConnectOperationInterface {
            public:
                static constexpr unsigned int VERIFY_GATEWAY_PRIORITY = 50;
                static constexpr unsigned int ENDPOINT_DISCOVERY_PRIORITY = 100;
                static constexpr unsigned int SYNCHRONIZE_STATE_PRIORITY = 150;
                virtual ~PostConnectOperationInterface() = default;
                virtual unsigned int getOperationPriority() = 0;
                virtual bool performOperation(const std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface>& messageSender) = 0;
                virtual void abortOperation() = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POSTCONNECTOPERATIONINTERFACE_H_

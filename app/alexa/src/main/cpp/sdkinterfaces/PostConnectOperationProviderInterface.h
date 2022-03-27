#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POSTCONNECTOPERATIONPROVIDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POSTCONNECTOPERATIONPROVIDERINTERFACE_H_

#include <memory>
#include "PostConnectOperationInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class PostConnectOperationProviderInterface {
            public:
                virtual ~PostConnectOperationProviderInterface() = default;
                virtual std::shared_ptr<avsCommon::sdkInterfaces::PostConnectOperationInterface> createPostConnectOperation() = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POSTCONNECTOPERATIONPROVIDERINTERFACE_H_

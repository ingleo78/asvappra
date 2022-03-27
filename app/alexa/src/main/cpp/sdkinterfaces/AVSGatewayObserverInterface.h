#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYOBSERVERINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AVSGatewayObserverInterface {
            public:
                virtual ~AVSGatewayObserverInterface() = default;
                virtual void onAVSGatewayChanged(const std::string& avsGateway) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYOBSERVERINTERFACE_H_

#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYASSIGNERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYASSIGNERINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AVSGatewayAssignerInterface {
            public:
                virtual ~AVSGatewayAssignerInterface() = default;
                virtual void setAVSGateway(const std::string& avsGateway) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSGATEWAYASSIGNERINTERFACE_H_

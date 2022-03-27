#ifndef ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_GATEWAYVERIFYSTATE_H_
#define ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_GATEWAYVERIFYSTATE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsGatewayManager {
        struct GatewayVerifyState {
            std::string avsGatewayURL;
            bool isVerified;
            GatewayVerifyState(const std::string& gatewayURL, bool isVerified = false);
        };
        inline GatewayVerifyState::GatewayVerifyState(const std::string& gatewayURL, bool isVerified) : avsGatewayURL{gatewayURL}, isVerified{isVerified} {}
    }
}
#endif
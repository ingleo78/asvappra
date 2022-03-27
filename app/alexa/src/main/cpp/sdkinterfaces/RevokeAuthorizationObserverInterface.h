#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_REVOKEAUTHORIZATIONOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_REVOKEAUTHORIZATIONOBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class RevokeAuthorizationObserverInterface {
            public:
                virtual ~RevokeAuthorizationObserverInterface() = default;
                virtual void onRevokeAuthorization();
            };
        }
    }
}
#endif
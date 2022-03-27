#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKREVOKEAUTHORIZATIONOBSERVER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKREVOKEAUTHORIZATIONOBSERVER_H_

#include <gmock/gmock.h>
#include "RevokeAuthorizationObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockRevokeAuthorizationObserver : public RevokeAuthorizationObserverInterface {
                public:
                    MOCK_METHOD0(onRevokeAuthorization, void());
                };
            }
        }
    }
}
#endif
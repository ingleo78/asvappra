#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYOBSERVER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYOBSERVER_H_

#include <gmock/gmock.h>
#include "AVSGatewayObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockAVSGatewayObserver : public AVSGatewayObserverInterface {
                public:
                    MOCK_METHOD1(onAVSGatewayChanged, void(const std::string& avsGateway));
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYOBSERVER_H_

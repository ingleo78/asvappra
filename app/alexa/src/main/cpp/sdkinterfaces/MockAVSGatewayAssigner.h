#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYASSIGNER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYASSIGNER_H_

#include <gmock/gmock.h>
#include "AVSGatewayAssignerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockAVSGatewayAssigner : public AVSGatewayAssignerInterface {
                public:
                    MOCK_METHOD1(setAVSGateway, void(const std::string& avsGateway));
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYASSIGNER_H_

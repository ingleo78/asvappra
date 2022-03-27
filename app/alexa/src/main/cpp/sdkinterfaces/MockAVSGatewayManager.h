#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSGATEWAYMANAGER_H_

#include <memory>
#include "AVSGatewayManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockAVSGatewayManager : public AVSGatewayManagerInterface {
                public:
                    MOCK_METHOD1(setAVSGatewayAssigner, bool(std::shared_ptr<AVSGatewayAssignerInterface> avsGatewayAssigner));
                    MOCK_CONST_METHOD0(getGatewayURL, std::string());
                    MOCK_METHOD1(setGatewayURL, bool(const std::string& avsGatewayURL));
                    MOCK_METHOD1(addObserver, void(std::shared_ptr<avsCommon::sdkInterfaces::AVSGatewayObserverInterface> observer));
                    MOCK_METHOD1(removeObserver, void(std::shared_ptr<avsCommon::sdkInterfaces::AVSGatewayObserverInterface> observer));
                };
            }
        }
    }
}
#endif
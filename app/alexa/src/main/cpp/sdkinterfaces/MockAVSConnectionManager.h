#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSCONNECTIONMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKAVSCONNECTIONMANAGER_H_

#include <gmock/gmock.h>
#include "AVSConnectionManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockAVSConnectionManager : public AVSConnectionManagerInterface {
                public:
                    MOCK_METHOD0(enable, void());
                    MOCK_METHOD0(disable, void());
                    MOCK_METHOD0(isEnabled, bool());
                    MOCK_METHOD0(reconnect, void());
                    MOCK_CONST_METHOD0(isConnected, bool());
                    MOCK_METHOD0(onWakeConnectionRetry, void());
                    MOCK_METHOD0(onWakeVerifyConnectivity, void());
                    MOCK_METHOD1(addMessageObserver, void(std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> observer));
                    MOCK_METHOD1(removeMessageObserver, void(std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> observer));
                    MOCK_METHOD1(addConnectionStatusObserver, void(std::shared_ptr<ConnectionStatusObserverInterface> observer));
                    MOCK_METHOD1(removeConnectionStatusObserver, void(std::shared_ptr<ConnectionStatusObserverInterface> observer));
                };
            }
        }
    }
}
#endif
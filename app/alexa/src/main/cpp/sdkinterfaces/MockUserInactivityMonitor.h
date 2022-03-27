#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKUSERINACTIVITYMONITOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKUSERINACTIVITYMONITOR_H_

#include <gmock/gmock.h>
#include "UserInactivityMonitorInterface.h"
#include "UserInactivityMonitorObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockUserInactivityMonitor : public UserInactivityMonitorInterface {
                public:
                    MOCK_METHOD0(onUserActive, void());
                    MOCK_METHOD0(timeSinceUserActivity, std::chrono::seconds());
                    MOCK_METHOD1(addObserver, void(std::shared_ptr<avsCommon::sdkInterfaces::UserInactivityMonitorObserverInterface>));
                    MOCK_METHOD1(removeObserver, void(std::shared_ptr<avsCommon::sdkInterfaces::UserInactivityMonitorObserverInterface>));
                };
            }
        }
    }
}
#endif
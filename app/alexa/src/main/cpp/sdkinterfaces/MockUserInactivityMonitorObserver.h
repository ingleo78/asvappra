#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKUSERINACTIVITYMONITOROBSERVER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKUSERINACTIVITYMONITOROBSERVER_H_

#include <gmock/gmock.h>
#include "UserInactivityMonitorObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockUserInactivityMonitorObserver : public UserInactivityMonitorObserverInterface {
                public:
                    MOCK_METHOD0(onUserInactivityReportSent, void());
                };
            }
        }
    }
}
#endif
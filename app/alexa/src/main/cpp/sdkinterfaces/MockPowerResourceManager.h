#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKPOWERRESOURCEMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKPOWERRESOURCEMANAGER_H_

#include <gmock/gmock.h>
#include "PowerResourceManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockPowerResourceManager : public PowerResourceManagerInterface {
                public:
                    MOCK_METHOD2(acquirePowerResource, void(const std::string& component, const PowerResourceLevel level));
                    MOCK_METHOD1(releasePowerResource, void(const std::string& component));
                    MOCK_METHOD1(isPowerResourceAcquired, bool(const std::string& component));
                };
            }
        }
    }
}
#endif
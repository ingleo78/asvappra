#include <gtest/gtest.h>
#include <memory>
#include <avs/ComponentConfiguration.h>
#include <sdkinterfaces/MockComponentReporterInterface.h>
#include <util/SDKVersion.h>
#include "SDKComponent.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace SDKComponent {
            namespace test {
                using namespace std;
                using namespace avsCommon;
                using namespace avs;
                using namespace sdkInterfaces;
                using namespace utils;
                using namespace sdkVersion;
                using namespace sdkInterfaces::test;
                using namespace testing;
                static const string SDK_COMPONENT_NAME = "com.amazon.sdk";
                TEST(SDKComponentTest, test_returnSDKVersion) {
                    shared_ptr<MockComponentReporterInterface> mockComponentReporter = make_shared<MockComponentReporterInterface>();
                    const shared_ptr<ComponentConfiguration> sdkConfig = ComponentConfiguration::createComponentConfiguration(SDK_COMPONENT_NAME, getCurrentVersion());
                    EXPECT_CALL(*mockComponentReporter, addConfiguration(sdkConfig)).Times(1).WillOnce(Return(true));
                    EXPECT_TRUE(SDKComponent::registerComponent(mockComponentReporter));
                }
            }
        }
    }
}
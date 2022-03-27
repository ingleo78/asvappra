#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCOMPONENTREPORTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCOMPONENTREPORTERINTERFACE_H_

#include <memory>
#include <gmock/gmock.h>
#include "ComponentReporterInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockComponentReporterInterface : public ComponentReporterInterface {
                public:
                    MOCK_METHOD1(addConfiguration, bool(std::shared_ptr<avs::ComponentConfiguration> config));
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCOMPONENTREPORTERINTERFACE_H_

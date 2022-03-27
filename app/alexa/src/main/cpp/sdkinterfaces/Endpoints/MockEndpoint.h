#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINT_H_

#include <gmock/gmock.h>
#include "EndpointInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                namespace test {
                    using namespace std;
                    using namespace avs;
                    class MockEndpoint : public EndpointInterface {
                    public:
                        MOCK_CONST_METHOD0(getEndpointId, EndpointIdentifier());
                        MOCK_CONST_METHOD0(getAttributes, AVSDiscoveryEndpointAttributes());
                        MOCK_CONST_METHOD0(getCapabilityConfigurations, vector<CapabilityConfiguration>());
                        MOCK_CONST_METHOD0(getCapabilities, unordered_map<CapabilityConfiguration, shared_ptr<DirectiveHandlerInterface>>());
                    };
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINT_H_

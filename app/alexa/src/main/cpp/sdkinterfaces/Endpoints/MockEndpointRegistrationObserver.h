#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTREGISTRATIONOBSERVER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTREGISTRATIONOBSERVER_H_

#include <gmock/gmock.h>
#include "EndpointRegistrationObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                namespace test {
                    class MockEndpointRegistrationObserver : public EndpointRegistrationObserverInterface {
                    public:
                        MOCK_METHOD3(onEndpointRegistration, void(const EndpointIdentifier& endpointId, const avs::AVSDiscoveryEndpointAttributes& attributes,
                                     const RegistrationResult result));
                        MOCK_METHOD2(
                            onEndpointDeregistration,
                            void(const EndpointIdentifier& endpointId, const DeregistrationResult result));
                    };
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTREGISTRATIONOBSERVER_H_

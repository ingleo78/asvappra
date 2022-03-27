#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTREGISTRATIONMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTREGISTRATIONMANAGER_H_

#include <gmock/gmock.h>
#include "EndpointRegistrationManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                namespace test {
                    class MockEndpointRegistrationManager : public EndpointRegistrationManagerInterface {
                    public:
                        MOCK_METHOD1(registerEndpoint, std::future<RegistrationResult>(std::shared_ptr<EndpointInterface> endpoint));
                        MOCK_METHOD1(deregisterEndpoint, std::future<DeregistrationResult>(const EndpointIdentifier& endpointId));
                        MOCK_METHOD1(addObserver, void(std::shared_ptr<EndpointRegistrationObserverInterface> observer));
                        MOCK_METHOD1(removeObserver, void(const std::shared_ptr<EndpointRegistrationObserverInterface>& observer));
                    };
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTREGISTRATIONMANAGER_H_

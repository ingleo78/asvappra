#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTREGISTRATIONMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTREGISTRATIONMANAGERINTERFACE_H_

#include <future>
#include <list>
#include <memory>
#include <string>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include "EndpointIdentifier.h"
#include "EndpointInterface.h"
#include "EndpointRegistrationObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                class EndpointRegistrationManagerInterface {
                public:
                    using RegistrationResult = EndpointRegistrationObserverInterface::RegistrationResult;
                    using DeregistrationResult = EndpointRegistrationObserverInterface::DeregistrationResult;
                    virtual ~EndpointRegistrationManagerInterface() = default;
                    virtual std::future<RegistrationResult> registerEndpoint(std::shared_ptr<EndpointInterface> endpoint);
                    virtual std::future<DeregistrationResult> deregisterEndpoint(const EndpointIdentifier& endpointId);
                    virtual void addObserver(std::shared_ptr<EndpointRegistrationObserverInterface> observer);
                    virtual void removeObserver(const std::shared_ptr<EndpointRegistrationObserverInterface>& observer);
                };
            }
        }
    }
}

#endif
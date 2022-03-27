#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTREGISTRATIONMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTREGISTRATIONMANAGERINTERFACE_H_

#include <future>
#include <list>
#include <memory>
#include <string>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <sdkinterfaces/Endpoints/EndpointInterface.h>
#include <sdkinterfaces/Endpoints/EndpointRegistrationObserverInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                using namespace std;
                class EndpointRegistrationManagerInterface {
                public:
                    virtual ~EndpointRegistrationManagerInterface() = default;
                    virtual future<RegistrationResult> registerEndpoint(shared_ptr<EndpointInterface> endpoint);
                    virtual future<DeregistrationResult> deregisterEndpoint(const EndpointIdentifier& endpointId);
                    virtual void addObserver(shared_ptr<EndpointRegistrationObserverInterface> observer);
                    virtual void removeObserver(const shared_ptr<EndpointRegistrationObserverInterface>& observer);
                };
            }
        }
    }
}
#endif
#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTINTERFACE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include "EndpointIdentifier.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                using namespace std;
                using namespace avs;
                class EndpointInterface {
                public:
                    virtual ~EndpointInterface() = default;
                    virtual EndpointIdentifier getEndpointId() const;
                    virtual AVSDiscoveryEndpointAttributes getAttributes() const;
                    virtual vector<CapabilityConfiguration> getCapabilityConfigurations() const;
                    virtual unordered_map<CapabilityConfiguration, shared_ptr<DirectiveHandlerInterface>> getCapabilities() const;
                };
            }
        }
    }
}
#endif
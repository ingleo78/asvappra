#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TOGGLECONTROLLER_TOGGLECONTROLLERATTRIBUTEBUILDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TOGGLECONTROLLER_TOGGLECONTROLLERATTRIBUTEBUILDERINTERFACE_H_

#include <avs/CapabilityResources.h>
#include <util/Optional.h>
#include "ToggleControllerAttributes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace toggleController {
                using namespace std;
                using namespace avs;
                using namespace utils;
                class ToggleControllerAttributeBuilderInterface {
                public:
                    virtual ~ToggleControllerAttributeBuilderInterface() = default;
                    virtual ToggleControllerAttributeBuilderInterface& withCapabilityResources(const CapabilityResources& capabilityResources);
                    virtual utils::Optional<ToggleControllerAttributes> build();
                };
            }
        }
    }
}
#endif
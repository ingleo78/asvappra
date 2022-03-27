#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLERATTRIBUTEBUILDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLERATTRIBUTEBUILDERINTERFACE_H_

#include <avs/CapabilityResources.h>
#include <util/Optional.h>
#include "ModeControllerAttributes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace modeController {
                using namespace std;
                using namespace avs;
                using namespace utils;
                class ModeControllerAttributeBuilderInterface {
                public:
                    virtual ~ModeControllerAttributeBuilderInterface() = default;
                    virtual ModeControllerAttributeBuilderInterface& withCapabilityResources(const CapabilityResources& capabilityResources);
                    virtual ModeControllerAttributeBuilderInterface& addMode(const string& mode, const ModeResources& modeResources);
                    virtual ModeControllerAttributeBuilderInterface& setOrdered(bool ordered);
                    virtual Optional<ModeControllerAttributes> build();
                };
            }
        }
    }
}
#endif
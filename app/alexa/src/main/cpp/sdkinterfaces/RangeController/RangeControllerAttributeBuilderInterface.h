#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLERATTRIBUTEBUILDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLERATTRIBUTEBUILDERINTERFACE_H_

#include <avs/CapabilityResources.h>
#include <util/Optional.h>
#include "RangeControllerAttributes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace rangeController {
                using namespace std;
                using namespace avs;
                using namespace utils;
                using namespace resources;
                class RangeControllerAttributeBuilderInterface {
                public:
                    virtual ~RangeControllerAttributeBuilderInterface() = default;
                    virtual RangeControllerAttributeBuilderInterface& withCapabilityResources(const CapabilityResources& capabilityResources);
                    virtual RangeControllerAttributeBuilderInterface& withUnitOfMeasure(const AlexaUnitOfMeasure& unitOfMeasure);
                    virtual RangeControllerAttributeBuilderInterface& addPreset(const pair<double, PresetResources>& preset);
                    virtual Optional<RangeControllerAttributes> build();
                };
            }
        }
    }
}
#endif
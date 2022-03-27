#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLERATTRIBUTES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLERATTRIBUTES_H_

#include <avs/CapabilityResources.h>
#include <avs/AlexaUnitOfMeasure.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace rangeController {
                using PresetResources = avsCommon::avs::CapabilityResources;
                using namespace std;
                using namespace avs;
                using namespace rangeController;
                using namespace resources;
                using namespace utils;
                struct RangeControllerAttributes {
                    RangeControllerAttributes();
                    RangeControllerAttributes(const CapabilityResources& capabilityResources, const Optional<AlexaUnitOfMeasure>& unitOfMeasure,
                                              const vector<pair<double, PresetResources>>& presets);
                    const CapabilityResources capabilityResources;
                    const Optional<AlexaUnitOfMeasure> unitOfMeasure;
                    vector<pair<double, PresetResources>>& presets;
                };
                inline RangeControllerAttributes::RangeControllerAttributes() : presets{presets}{}
                inline RangeControllerAttributes::RangeControllerAttributes(const CapabilityResources& capabilityResources,
                                                                            const Optional<AlexaUnitOfMeasure>& unitOfMeasure,
                                                                            const vector<pair<double, PresetResources>>& presets) :
                                                                            capabilityResources{capabilityResources}, unitOfMeasure{unitOfMeasure},
                                                                            presets{(vector<pair<double, PresetResources>>&)presets}{}
            }
        }
    }
}
#endif
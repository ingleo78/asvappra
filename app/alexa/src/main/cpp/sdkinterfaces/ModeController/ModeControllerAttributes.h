#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLERATTRIBUTES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLERATTRIBUTES_H_

#include <unordered_map>
#include <avs/CapabilityResources.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace modeController {
                using namespace avs;
                using namespace std;
                using ModeResources = avs::CapabilityResources;
                struct ModeControllerAttributes {
                    ModeControllerAttributes();
                    ModeControllerAttributes(const CapabilityResources& capabilityResources, const unordered_map<string, ModeResources>& modes, bool ordered);
                    const avsCommon::avs::CapabilityResources capabilityResources;
                    const unordered_map<std::string, ModeResources>& modes;
                    const bool ordered;
                };
                inline ModeControllerAttributes::ModeControllerAttributes() : ModeControllerAttributes(CapabilityResources(), modes, false){}
                inline ModeControllerAttributes::ModeControllerAttributes(const CapabilityResources& capabilityResources,
                                                                          const unordered_map<string, ModeResources>& modes, bool ordered) :
                                                                          capabilityResources{capabilityResources}, modes{modes}, ordered{ordered} {}
            }
        }
    }
}
#endif
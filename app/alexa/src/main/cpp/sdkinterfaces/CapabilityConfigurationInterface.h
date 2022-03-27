#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CAPABILITYCONFIGURATIONINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CAPABILITYCONFIGURATIONINTERFACE_H_

#include <memory>
#include <unordered_set>
#include <avs/CapabilityConfiguration.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class CapabilityConfigurationInterface {
            public:
                virtual ~CapabilityConfigurationInterface() = default;
                virtual std::unordered_set<std::shared_ptr<avsCommon::avs::CapabilityConfiguration>> getCapabilityConfigurations();
            };
        }
    }
}
#endif
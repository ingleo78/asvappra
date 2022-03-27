#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POWERRESOURCEMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POWERRESOURCEMANAGERINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class PowerResourceManagerInterface {
            public:
                enum class PowerResourceLevel {
                    STANDBY_LOW = 0,
                    STANDBY_MED,
                    STANDBY_HIGH,
                    ACTIVE_LOW,
                    ACTIVE_MED,
                    ACTIVE_HIGH
                };
                virtual ~PowerResourceManagerInterface() = default;
                virtual void acquirePowerResource(const std::string& component, const PowerResourceLevel level = PowerResourceLevel::ACTIVE_HIGH) = 0;
                virtual void releasePowerResource(const std::string& component) = 0;
                virtual bool isPowerResourceAcquired(const std::string& component) = 0;
            };
        }
    }
}
#endif
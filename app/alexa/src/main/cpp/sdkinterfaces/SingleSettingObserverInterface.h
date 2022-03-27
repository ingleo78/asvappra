#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SINGLESETTINGOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SINGLESETTINGOBSERVERINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class SingleSettingObserverInterface {
            public:
                virtual ~SingleSettingObserverInterface() = default;
                virtual void onSettingChanged(const std::string& key, const std::string& value) = 0;
            };
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SINGLESETTINGOBSERVERINTERFACE_H_

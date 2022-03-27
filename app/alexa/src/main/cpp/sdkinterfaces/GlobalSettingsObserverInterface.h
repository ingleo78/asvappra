#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_GLOBALSETTINGSOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_GLOBALSETTINGSOBSERVERINTERFACE_H_

#include <unordered_map>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class GlobalSettingsObserverInterface {
            public:
                virtual ~GlobalSettingsObserverInterface() = default;
                virtual void onSettingChanged(const std::unordered_map<std::string, std::string>& mapOfSettings) = 0;
            };
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_GLOBALSETTINGSOBSERVERINTERFACE_H_

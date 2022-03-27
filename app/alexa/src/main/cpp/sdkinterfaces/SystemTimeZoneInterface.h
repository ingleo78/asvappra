#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMTIMEZONEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMTIMEZONEINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class SystemTimeZoneInterface {
            public:
                virtual bool setTimezone(const std::string& timeZone);
                virtual ~SystemTimeZoneInterface() = default;
            };
        }
    }
}
#endif
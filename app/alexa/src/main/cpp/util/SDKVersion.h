#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDKVERSION_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDKVERSION_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sdkVersion {
                inline static std::string getCurrentVersion() { return "1.20.0"; }
                inline static int getMajorVersion() { return 1; }
                inline static int getMinorVersion() { return 20; }
                inline static int getPatchVersion() { return 0; }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDKVERSION_H_

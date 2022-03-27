#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_A2DPROLE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_A2DPROLE_H_

#include <string>
#include <iostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                enum class A2DPRole {
                    NONE,
                    SINK,
                    SOURCE
                };
                inline std::string_view a2DPRoleToString(A2DPRole value) {
                    switch (value) {
                        case A2DPRole::NONE: return "NONE";
                        case A2DPRole::SINK: return "SINK";
                        case A2DPRole::SOURCE: return "SOURCE";
                        default: return "UNKNOWN";
                    }
                }
                inline std::ostream& operator<<(std::ostream& stream, const A2DPRole a2DPRole) {
                    return stream << a2DPRoleToString(a2DPRole);
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_A2DPROLE_H_

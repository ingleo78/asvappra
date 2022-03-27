#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_REQUESTER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_REQUESTER_H_

#include "../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/ostream"
#include "../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/string"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class Requester {
                CLOUD,
                DEVICE
            };
            inline std::string_view requesterToString(Requester requester) {
                switch(requester) {
                    case Requester::CLOUD: return "CLOUD";
                    case Requester::DEVICE: return "DEVICE";
                }
                return "UNKNOWN";
            }
            inline std::ostream& operator<<(std::ostream& stream, Requester requester) {
                return stream << (std::string)requesterToString(requester);
            }
        }
    }
}
#endif
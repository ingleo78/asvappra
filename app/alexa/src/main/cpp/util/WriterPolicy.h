#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_WRITERPOLICY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_WRITERPOLICY_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                enum class WriterPolicy {
                    NONBLOCKABLE,
                    ALL_OR_NOTHING,
                    BLOCKING
                };
            }
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_WRITERPOLICY_H_

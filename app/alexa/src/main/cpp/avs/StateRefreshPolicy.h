#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_STATEREFRESHPOLICY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_STATEREFRESHPOLICY_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class StateRefreshPolicy {
                NEVER,
                ALWAYS,
                SOMETIMES
            };
        }
    }
}
#endif
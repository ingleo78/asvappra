#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2SENDSTATUS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2SENDSTATUS_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                enum class HTTP2SendStatus {
                    CONTINUE,
                    PAUSE,
                    COMPLETE,
                    ABORT
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2SENDSTATUS_H_

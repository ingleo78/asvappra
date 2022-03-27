#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONSTATUS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONSTATUS_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                enum class HTTP2ConnectionStatus {
                    CONNECTING,
                    CONNECTED,
                    DISCONNECTED
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONSTATUS_H_

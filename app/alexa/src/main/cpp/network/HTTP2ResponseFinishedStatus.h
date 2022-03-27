#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2RESPONSEFINISHEDSTATUS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2RESPONSEFINISHEDSTATUS_H_

#include <ostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                enum class HTTP2ResponseFinishedStatus {
                    COMPLETE,
                    TIMEOUT,
                    CANCELLED,
                    INTERNAL_ERROR
                };
                inline std::ostream& operator<<(std::ostream& stream, HTTP2ResponseFinishedStatus status) {
                    switch (status) {
                        case HTTP2ResponseFinishedStatus::COMPLETE: return stream << "COMPLETE";
                        case HTTP2ResponseFinishedStatus::TIMEOUT: return stream << "TIMEOUT";
                        case HTTP2ResponseFinishedStatus::CANCELLED: return stream << "CANCELLED";
                        case HTTP2ResponseFinishedStatus::INTERNAL_ERROR: return stream << "INTERNAL_ERROR";
                        default: stream << "";
                    }
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2RESPONSEFINISHEDSTATUS_H_

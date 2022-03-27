#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTTYPE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTTYPE_H_

#include <ostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                enum class HTTP2RequestType {
                    GET,
                    POST
                };
                inline std::ostream& operator<<(std::ostream& stream, HTTP2RequestType type) {
                    switch (type) {
                        case HTTP2RequestType::GET:
                            return stream << "GET";
                        case HTTP2RequestType::POST:
                            return stream << "POST";
                    }
                    return stream << "UNKNOWN(" << static_cast<int>(type) << ")";
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTTYPE_H_

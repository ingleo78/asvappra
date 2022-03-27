#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTSOURCEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTSOURCEINTERFACE_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "HTTP2SendDataResult.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2RequestSourceInterface {
                public:
                    virtual ~HTTP2RequestSourceInterface() = default;
                    virtual std::vector<std::string> getRequestHeaderLines() = 0;
                    virtual HTTP2SendDataResult onSendData(char* bytes, size_t size) = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTSOURCEINTERFACE_H_

#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMEREQUESTSOURCEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMEREQUESTSOURCEINTERFACE_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "HTTP2GetMimeHeadersResult.h"
#include "HTTP2SendDataResult.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2MimeRequestSourceInterface {
                    public:
                        virtual ~HTTP2MimeRequestSourceInterface() = default;
                        virtual std::vector<std::string> getRequestHeaderLines() = 0;
                        virtual HTTP2GetMimeHeadersResult getMimePartHeaderLines() = 0;
                        virtual HTTP2SendDataResult onSendMimePartData(char* bytes, size_t size) = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMEREQUESTSOURCEINTERFACE_H_

#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTINTERFACE_H_

#include <string>
#include "HTTP2RequestSourceInterface.h"
#include "HTTP2ResponseSinkInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2RequestInterface {
                public:
                    virtual ~HTTP2RequestInterface() = default;
                    virtual bool cancel() = 0;
                    virtual std::string getId() const = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTINTERFACE_H_

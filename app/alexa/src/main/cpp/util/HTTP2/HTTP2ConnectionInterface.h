#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONINTERFACE_H_

#include <memory>
#include "HTTP2RequestConfig.h"
#include "HTTP2RequestInterface.h"
#include "HTTP2RequestType.h"
#include "HTTP2ConnectionObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2ConnectionInterface {
                public:
                    virtual ~HTTP2ConnectionInterface() = default;
                    virtual std::shared_ptr<HTTP2RequestInterface> createAndSendRequest(const HTTP2RequestConfig& config) = 0;
                    virtual void disconnect() = 0;
                    virtual void addObserver(std::shared_ptr<HTTP2ConnectionObserverInterface> observer) = 0;
                    virtual void removeObserver(std::shared_ptr<HTTP2ConnectionObserverInterface> observer) = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONINTERFACE_H_

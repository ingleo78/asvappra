#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2RESPONSESINKINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2RESPONSESINKINTERFACE_H_

#include <cstddef>
#include <cstdint>
#include "HTTP2ReceiveDataStatus.h"
#include "HTTP2ResponseFinishedStatus.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2ResponseSinkInterface {
                    public:
                        virtual ~HTTP2ResponseSinkInterface() = default;
                        virtual bool onReceiveResponseCode(long responseCode) = 0;
                        virtual bool onReceiveHeaderLine(const std::string& line) = 0;
                        virtual HTTP2ReceiveDataStatus onReceiveData(const char* bytes, size_t size) = 0;
                        virtual void onResponseFinished(HTTP2ResponseFinishedStatus status) = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2RESPONSESINKINTERFACE_H_

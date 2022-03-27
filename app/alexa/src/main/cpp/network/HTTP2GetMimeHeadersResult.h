#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2GETMIMEHEADERSRESULT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2GETMIMEHEADERSRESULT_H_

#include <string>
#include <vector>
#include "HTTP2SendStatus.h"
#include "util/PlatformDefinitions.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                struct HTTP2GetMimeHeadersResult {
                    public:
                        HTTP2SendStatus status;
                        std::vector<std::string> headers;
                        HTTP2GetMimeHeadersResult(const std::vector<std::string>& headers);
                        static const avscommon_EXPORT HTTP2GetMimeHeadersResult PAUSE;
                        static const avscommon_EXPORT HTTP2GetMimeHeadersResult COMPLETE;
                        static const avscommon_EXPORT HTTP2GetMimeHeadersResult ABORT;
                    private:
                        HTTP2GetMimeHeadersResult(HTTP2SendStatus status, const std::vector<std::string>& headers);
                };
                inline HTTP2GetMimeHeadersResult::HTTP2GetMimeHeadersResult(const std::vector<std::string>& headers) : status{HTTP2SendStatus::CONTINUE},
                        headers{headers} {
                }
                inline HTTP2GetMimeHeadersResult::HTTP2GetMimeHeadersResult(HTTP2SendStatus statusIn,const std::vector<std::string>& headersIn) : status{statusIn},
                        headers{headersIn} {}
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2GETMIMEHEADERSRESULT_H_

#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2SENDDATARESULT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2SENDDATARESULT_H_

#include <cstddef>
#include "util/PlatformDefinitions.h"
#include "HTTP2SendStatus.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                struct HTTP2SendDataResult {
                        HTTP2SendStatus status;
                        size_t size;
                        explicit HTTP2SendDataResult(size_t size);
                        static const avscommon_EXPORT HTTP2SendDataResult PAUSE;
                        static const avscommon_EXPORT HTTP2SendDataResult COMPLETE;
                        static const avscommon_EXPORT HTTP2SendDataResult ABORT;
                    private:
                        HTTP2SendDataResult(HTTP2SendStatus status, size_t size);
                };
                inline HTTP2SendDataResult::HTTP2SendDataResult(size_t sizeIn) : status{HTTP2SendStatus::CONTINUE}, size{sizeIn} {}
                inline HTTP2SendDataResult::HTTP2SendDataResult(HTTP2SendStatus statusIn, size_t sizeIn) : status{statusIn}, size{sizeIn} {}
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2SENDDATARESULT_H_

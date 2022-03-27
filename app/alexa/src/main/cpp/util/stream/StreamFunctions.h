#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STREAM_STREAMFUNCTIONS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STREAM_STREAMFUNCTIONS_H_

#include <memory>
#include <istream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace stream {
                std::unique_ptr<std::istream> streamFromData(const unsigned char* data, size_t length);
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STREAM_STREAMFUNCTIONS_H_

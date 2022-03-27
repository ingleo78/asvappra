#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEMORY_MEMORY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEMORY_MEMORY_H_

#include <memory>
#include <utility>
#include <avs/attachment/InProcessAttachment.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace memory {
                template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) {
                    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
                }
            }
        }
    }
}
#endif
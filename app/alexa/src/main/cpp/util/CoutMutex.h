#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_COUTMUTEX_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_COUTMUTEX_H_

#include <memory>
#include <mutex>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            inline static std::shared_ptr<std::mutex> getCoutMutex() {
                static std::shared_ptr<std::mutex> coutMutex(new std::mutex);
                return coutMutex;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_COUTMUTEX_H_

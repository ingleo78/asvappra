#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_INPROCESSSDS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_INPROCESSSDS_H_

#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include "SharedDataStream.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                struct InProcessSDSTraits {
                    using AtomicIndex = std::atomic<uint64_t>;
                    using AtomicBool = std::atomic<bool>;
                    using Buffer = std::vector<uint8_t>;
                    using Mutex = std::mutex;
                    using ConditionVariable = std::condition_variable;
                    static constexpr const char* traitsName = "alexaClientSDK::avsCommon::utils::sds::InProcessSDSTraits";
                };
                using InProcessSDS = SharedDataStream<InProcessSDSTraits>;
            }
        }
    }
}
#endif
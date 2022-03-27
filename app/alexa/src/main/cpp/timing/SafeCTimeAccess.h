#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_SAFECTIMEACCESS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_SAFECTIMEACCESS_H_

#include <ctime>
#include <memory>
#include <mutex>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                class SafeCTimeAccess {
                public:
                    static std::shared_ptr<SafeCTimeAccess> instance();
                    bool getGmtime(const std::time_t& time, std::tm* calendarTime);
                    bool getLocaltime(const std::time_t& time, std::tm* calendarTime);
                private:
                    SafeCTimeAccess() = default;
                    bool safeAccess(std::tm* (*timeAccessFunction)(const std::time_t* time), const std::time_t& time, std::tm* calendarTime);
                    std::mutex m_timeLock;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_SAFECTIMEACCESS_H_

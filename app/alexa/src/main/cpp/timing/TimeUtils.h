#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_TIMEUTILS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_TIMEUTILS_H_

#include <chrono>
#include <ctime>
#include <string>
#include <util/RetryTimer.h>
#include "SafeCTimeAccess.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                class TimeUtils {
                public:
                    TimeUtils();
                    bool convertToUtcTimeT(const std::tm* utcTm, std::time_t* ret);
                    bool convert8601TimeStringToUnix(const std::string& timeString, int64_t* unixTime);
                    bool getCurrentUnixTime(int64_t* currentTime);
                    bool convertTimeToUtcIso8601Rfc3339(const std::chrono::system_clock::time_point& tp, std::string* iso8601TimeString);
                private:
                    bool localtimeOffset(std::time_t referenceTime, std::time_t* ret);
                    std::shared_ptr<SafeCTimeAccess> m_safeCTimeAccess;
                };
            }
        }
    }
}
#endif
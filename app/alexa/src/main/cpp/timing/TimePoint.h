#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_TIMEPOINT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_TIMEPOINT_H_

#include <string>
#include "TimeUtils.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                class TimePoint {
                public:
                    TimePoint();
                    static TimePoint now();
                    bool setTime_ISO_8601(const std::string& time_ISO_8601);
                    std::string getTime_ISO_8601() const;
                    int64_t getTime_Unix() const;
                private:
                    std::string m_time_ISO_8601;
                    int64_t m_time_Unix;
                    TimeUtils m_timeUtils;
                };
            }
        }
    }
}
#endif
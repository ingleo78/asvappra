#include <timing/TimePoint.h>
#include <logger/Logger.h>
#include <timing/TimeUtils.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                using namespace logger;
                static const std::string TAG("TimePoint");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                TimePoint::TimePoint() : m_time_Unix{0} {}
                TimePoint TimePoint::now() {
                    std::string timeNowStr;
                    auto timeNow = std::chrono::system_clock::now();
                    TimeUtils().convertTimeToUtcIso8601Rfc3339(timeNow, &timeNowStr);
                    TimePoint retValue;
                    retValue.setTime_ISO_8601(timeNowStr);
                    return retValue;
                }
                bool TimePoint::setTime_ISO_8601(const std::string& time_ISO_8601) {
                    int64_t tempUnixTime = 0;
                    if (!m_timeUtils.convert8601TimeStringToUnix(time_ISO_8601, &tempUnixTime)) {
                        ACSDK_ERROR(LX("setTime_ISO_8601Failed").d("input", time_ISO_8601).m("Could not convert to Unix time."));
                        return false;
                    }
                    m_time_ISO_8601 = time_ISO_8601;
                    m_time_Unix = tempUnixTime;
                    return true;
                }
                std::string TimePoint::getTime_ISO_8601() const {
                    return m_time_ISO_8601;
                }
                int64_t TimePoint::getTime_Unix() const {
                    return m_time_Unix;
                }
            }
        }
    }
}

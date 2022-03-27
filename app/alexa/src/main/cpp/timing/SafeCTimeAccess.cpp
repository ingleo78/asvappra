#include "SafeCTimeAccess.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                std::shared_ptr<SafeCTimeAccess> SafeCTimeAccess::instance() {
                    static std::shared_ptr<SafeCTimeAccess> s_safeCTimeAccess(new SafeCTimeAccess);
                    return s_safeCTimeAccess;
                }
                bool SafeCTimeAccess::safeAccess(std::tm* (*timeAccessFunction)(const std::time_t* time), const std::time_t& time, std::tm* calendarTime) {
                    if (!calendarTime) return false;
                    bool succeeded = false;
                    {
                        std::lock_guard<std::mutex> lock{m_timeLock};
                        auto tempCalendarTime = timeAccessFunction(&time);
                        if (tempCalendarTime) {
                            *calendarTime = *tempCalendarTime;
                            succeeded = true;
                        }
                    }
                    return succeeded;
                }
                bool SafeCTimeAccess::getGmtime(const std::time_t& time, std::tm* calendarTime) {
                    return safeAccess(std::gmtime, time, calendarTime);
                }
                bool SafeCTimeAccess::getLocaltime(const std::time_t& time, std::tm* calendarTime) {
                    return safeAccess(std::localtime, time, calendarTime);
                }
            }
        }
    }
}

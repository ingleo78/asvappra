#include <cstdio>
#include <iostream>
#include "logger/LogStringFormatter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                static const char* STRFTIME_FORMAT_STRING = "%Y-%m-%d %H:%M:%S";
                static const int DATE_AND_TIME_STRING_SIZE = 20;
                static const char TIME_AND_MILLIS_SEPARATOR = '.';
                static const char* MILLIS_FORMAT_STRING = "%03d";
                static const int MILLIS_STRING_SIZE = 4;
                static const std::string MILLIS_AND_THREAD_SEPARATOR = " [";
                static const std::string THREAD_AND_LEVEL_SEPARATOR = "] ";
                static const char LEVEL_AND_TEXT_SEPARATOR = ' ';
                static const int MILLISECONDS_PER_SECOND = 1000;
                LogStringFormatter::LogStringFormatter() : m_safeCTimeAccess(timing::SafeCTimeAccess::instance()) {}
                std::string LogStringFormatter::format(Level level, std::chrono::system_clock::time_point time, const char* threadMoniker, const char* text) {
                    bool dateTimeFailure = false;
                    bool millisecondFailure = false;
                    char dateTimeString[DATE_AND_TIME_STRING_SIZE];
                    auto timeAsTime_t = std::chrono::system_clock::to_time_t(time);
                    std::tm timeAsTm;
                    if (!m_safeCTimeAccess->getGmtime(timeAsTime_t, &timeAsTm) ||
                        0 == strftime(dateTimeString, sizeof(dateTimeString), STRFTIME_FORMAT_STRING, &timeAsTm)) {
                        dateTimeFailure = true;
                    }
                    auto timeMillisPart = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count() % MILLISECONDS_PER_SECOND);
                    char millisString[MILLIS_STRING_SIZE];
                    if (snprintf(millisString, sizeof(millisString), MILLIS_FORMAT_STRING, timeMillisPart) < 0) millisecondFailure = true;
                    std::stringstream stringToEmit;
                    stringToEmit << (dateTimeFailure ? "ERROR: strftime() failed.  Date and time not logged." : dateTimeString)
                                 << TIME_AND_MILLIS_SEPARATOR
                                 << (millisecondFailure ? "ERROR: snprintf() failed.  Milliseconds not logged." : millisString)
                                 << MILLIS_AND_THREAD_SEPARATOR << threadMoniker << THREAD_AND_LEVEL_SEPARATOR
                                 << convertLevelToChar(level) << LEVEL_AND_TEXT_SEPARATOR << text;
                    return stringToEmit.str();
                }
            }
        }
    }
}

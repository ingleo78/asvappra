#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <random>
#include <sstream>
#include <timing/TimeUtils.h>
#include <logger/Logger.h>
#include "string/StringUtils.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                using namespace logger;
                using namespace string;
                static const std::string TAG("TimeUtils");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                static const int ENCODED_TIME_STRING_YEAR_STRING_LENGTH = 4;
                static const int ENCODED_TIME_STRING_MONTH_STRING_LENGTH = 2;
                static const int ENCODED_TIME_STRING_DAY_STRING_LENGTH = 2;
                static const int ENCODED_TIME_STRING_HOUR_STRING_LENGTH = 2;
                static const int ENCODED_TIME_STRING_MINUTE_STRING_LENGTH = 2;
                static const int ENCODED_TIME_STRING_SECOND_STRING_LENGTH = 2;
                static const int ENCODED_TIME_STRING_POSTFIX_STRING_LENGTH = 4;
                static const std::string ENCODED_TIME_STRING_DASH_SEPARATOR_STRING = "-";
                static const std::string ENCODED_TIME_STRING_T_SEPARATOR_STRING = "T";
                static const std::string ENCODED_TIME_STRING_COLON_SEPARATOR_STRING = ":";
                static const std::string ENCODED_TIME_STRING_PLUS_SEPARATOR_STRING = "+";
                static const unsigned long ENCODED_TIME_STRING_YEAR_OFFSET = 0;
                static const unsigned long ENCODED_TIME_STRING_MONTH_OFFSET = ENCODED_TIME_STRING_YEAR_OFFSET + ENCODED_TIME_STRING_YEAR_STRING_LENGTH +
                                                                              ENCODED_TIME_STRING_DASH_SEPARATOR_STRING.length();
                static const unsigned long ENCODED_TIME_STRING_DAY_OFFSET = ENCODED_TIME_STRING_MONTH_OFFSET + ENCODED_TIME_STRING_MONTH_STRING_LENGTH +
                                                                            ENCODED_TIME_STRING_DASH_SEPARATOR_STRING.length();
                static const unsigned long ENCODED_TIME_STRING_HOUR_OFFSET = ENCODED_TIME_STRING_DAY_OFFSET + ENCODED_TIME_STRING_DAY_STRING_LENGTH +
                                                                             ENCODED_TIME_STRING_T_SEPARATOR_STRING.length();
                static const unsigned long ENCODED_TIME_STRING_MINUTE_OFFSET = ENCODED_TIME_STRING_HOUR_OFFSET + ENCODED_TIME_STRING_HOUR_STRING_LENGTH +
                                                                               ENCODED_TIME_STRING_COLON_SEPARATOR_STRING.length();
                static const unsigned long ENCODED_TIME_STRING_SECOND_OFFSET = ENCODED_TIME_STRING_MINUTE_OFFSET + ENCODED_TIME_STRING_MINUTE_STRING_LENGTH +
                                                                               ENCODED_TIME_STRING_COLON_SEPARATOR_STRING.length();
                static const unsigned long ENCODED_TIME_STRING_EXPECTED_LENGTH = ENCODED_TIME_STRING_SECOND_OFFSET + ENCODED_TIME_STRING_SECOND_STRING_LENGTH +
                                                                                 ENCODED_TIME_STRING_PLUS_SEPARATOR_STRING.length() +
                                                                                 ENCODED_TIME_STRING_POSTFIX_STRING_LENGTH;
                static bool convertToLocalTimeT(const std::tm* timeStruct, std::time_t* ret) {
                    if (timeStruct == nullptr) return false;
                    std::tm tmCopy = *timeStruct;
                    tmCopy.tm_isdst = -1;
                    *ret = std::mktime(&tmCopy);
                    return *ret >= 0;
                }
                TimeUtils::TimeUtils() : m_safeCTimeAccess{SafeCTimeAccess::instance()} {}
                bool TimeUtils::convertToUtcTimeT(const std::tm* utcTm, std::time_t* ret) {
                    std::time_t converted;
                    std::time_t offset;
                    if (ret == nullptr) {
                        ACSDK_ERROR(LX("convertToUtcTimeT").m("return variable is null"));
                        return false;
                    }
                    if (!convertToLocalTimeT(utcTm, &converted) || !localtimeOffset(converted, &offset)) {
                        ACSDK_ERROR(LX("convertToUtcTimeT").m("failed to convert to local time"));
                        return false;
                    }
                    *ret = converted - offset;
                    return true;
                }
                bool TimeUtils::convert8601TimeStringToUnix(const std::string& timeString, int64_t* convertedTime) {
                    if (!convertedTime) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").m("convertedTime parameter was nullptr."));
                        return false;
                    }
                    std::tm timeInfo;
                    if (timeString.length() != ENCODED_TIME_STRING_EXPECTED_LENGTH) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").d("unexpected time string length:", timeString.length()));
                        return false;
                    }
                    if (!stringToInt(timeString.substr(ENCODED_TIME_STRING_YEAR_OFFSET, ENCODED_TIME_STRING_YEAR_STRING_LENGTH), &(timeInfo.tm_year))) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").m("error parsing year. Input:" + timeString));
                        return false;
                    }
                    if (!stringToInt(timeString.substr(ENCODED_TIME_STRING_MONTH_OFFSET, ENCODED_TIME_STRING_MONTH_STRING_LENGTH), &(timeInfo.tm_mon))) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").m("error parsing month. Input:" + timeString));
                        return false;
                    }
                    if (!stringToInt(timeString.substr(ENCODED_TIME_STRING_DAY_OFFSET, ENCODED_TIME_STRING_DAY_STRING_LENGTH), &(timeInfo.tm_mday))) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").m("error parsing day. Input:" + timeString));
                        return false;
                    }
                    if (!stringToInt(timeString.substr(ENCODED_TIME_STRING_HOUR_OFFSET, ENCODED_TIME_STRING_HOUR_STRING_LENGTH), &(timeInfo.tm_hour))) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").m("error parsing hour. Input:" + timeString));
                        return false;
                    }
                    if (!stringToInt(timeString.substr(ENCODED_TIME_STRING_MINUTE_OFFSET, ENCODED_TIME_STRING_MINUTE_STRING_LENGTH), &(timeInfo.tm_min))) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").m("error parsing minute. Input:" + timeString));
                        return false;
                    }

                    if (!stringToInt(timeString.substr(ENCODED_TIME_STRING_SECOND_OFFSET, ENCODED_TIME_STRING_SECOND_STRING_LENGTH), &(timeInfo.tm_sec))) {
                        ACSDK_ERROR(LX("convert8601TimeStringToUnixFailed").m("error parsing second. Input:" + timeString));
                        return false;
                    }
                    timeInfo.tm_year -= 1900;
                    timeInfo.tm_mon -= 1;
                    std::time_t convertedTimeT;
                    bool ok = convertToUtcTimeT(&timeInfo, &convertedTimeT);
                    if (!ok) return false;
                    *convertedTime = static_cast<int64_t>(convertedTimeT);
                    return true;
                }
                bool TimeUtils::getCurrentUnixTime(int64_t* currentTime) {
                    if (!currentTime) {
                        ACSDK_ERROR(LX("getCurrentUnixTimeFailed").m("currentTime parameter was nullptr."));
                        return false;
                    }
                    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    *currentTime = static_cast<int64_t>(now);
                    return now >= 0;
                }

                bool TimeUtils::convertTimeToUtcIso8601Rfc3339(
                    const std::chrono::system_clock::time_point& tp,
                    std::string* iso8601TimeString) {
                    char buf[29];
                    memset(buf, 0, sizeof(buf));
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
                    auto sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
                    const time_t timeSecs = static_cast<time_t>(sec.count());
                    std::tm utcTm;
                    if (!m_safeCTimeAccess->getGmtime(timeSecs, &utcTm)) {
                        ACSDK_ERROR(LX("convertTimeToUtcIso8601Rfc3339").m("cannot retrieve tm struct"));
                        return false;
                    }
                    auto strftimeResult = std::strftime(buf, sizeof(buf) - 1, "%Y-%m-%dT%H:%M:%S", &utcTm);
                    if (strftimeResult == 0) {
                        ACSDK_ERROR(LX("convertTimeToUtcIso8601Rfc3339Failed").m("strftime(..) failed"));
                        return false;
                    }
                    std::stringstream millisecondTrailer;
                    millisecondTrailer << buf << "." << std::setfill('0') << std::setw(3) << (ms.count() % 1000) << "Z";
                    *iso8601TimeString = millisecondTrailer.str();
                    return true;
                }
                bool TimeUtils::localtimeOffset(std::time_t referenceTime, std::time_t* ret) {
                    std::tm utcTm;
                    std::time_t utc;
                    std::tm localTm;
                    std::time_t local;
                    if (!m_safeCTimeAccess->getGmtime(referenceTime, &utcTm) || !convertToLocalTimeT(&utcTm, &utc) ||
                        !m_safeCTimeAccess->getLocaltime(referenceTime, &localTm) || !convertToLocalTimeT(&localTm, &local)) {
                        ACSDK_ERROR(LX("localtimeOffset").m("cannot retrieve tm struct"));
                        return false;
                    }
                    *ret = utc - local;
                    return true;
                }
            }
        }
    }
}

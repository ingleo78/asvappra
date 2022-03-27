#include <algorithm>
#include <iostream>
#include <errno.h>
#include <iomanip>
#include <limits>
#include <stdlib.h>
#include <logger/Logger.h>
#include "string/StringUtils.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace string {
                using namespace avsCommon::utils::logger;
                static const std::string TAG("StringUtils");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                static const int BASE_TEN = 10;
                bool stringToInt(const std::string& str, int* result) {
                    return stringToInt(str.c_str(), result);
                }
                bool stringToInt(const char* str, int* result) {
                    int64_t tempResult = 0;
                    if (!stringToInt64(str, &tempResult) || tempResult < std::numeric_limits<int>::min() ||
                        tempResult > std::numeric_limits<int>::max()) {
                        ACSDK_ERROR(LX("stringToIntFailed").m("converted number was out of range."));
                        return false;
                    }
                    *result = static_cast<int>(tempResult);
                    return true;
                }
                std::string byteVectorToString(const std::vector<uint8_t>& byteVector) {
                    std::stringstream ss;
                    bool firstTime = true;
                    for (const auto& byte : byteVector) {
                        ss << std::hex << (firstTime ? "" : " ") << "0x" << std::setfill('0') << std::setw(2) << int(byte) << std::dec;
                        firstTime = false;
                    }
                    return ss.str();
                }
                std::string stringToLowerCase(const std::string& input) {
                    std::string lowerCaseString = input;
                    std::transform(lowerCaseString.begin(), lowerCaseString.end(), lowerCaseString.begin(), ::tolower);
                    return lowerCaseString;
                }
                std::string stringToUpperCase(const std::string& input) {
                    std::string upperCaseString = input;
                    std::transform(upperCaseString.begin(), upperCaseString.end(), upperCaseString.begin(), ::toupper);
                    return upperCaseString;
                }
                bool stringToInt64(const std::string& str, int64_t* result) {
                    return stringToInt64(str.c_str(), result);
                }
                bool stringToInt64(const char* str, int64_t* result) {
                    if (!str) {
                        ACSDK_ERROR(LX("stringToInt64Failed").m("string parameter is null."));
                        return false;
                    }
                    if (!result) {
                        ACSDK_ERROR(LX("stringToInt64Failed").m("result parameter is null."));
                        return false;
                    }
                    errno = 0;
                    char* endPtr = nullptr;
                    auto tempResult = strtoll(str, &endPtr, BASE_TEN);
                    if (str == endPtr) {
                        ACSDK_ERROR(LX("stringToInt64Failed").m("input string was not parsable as an integer."));
                        return false;
                    }
                    if (ERANGE == errno || tempResult < std::numeric_limits<int64_t>::min() ||
                        tempResult > std::numeric_limits<int64_t>::max()) {
                        ACSDK_ERROR(LX("stringToInt64Failed").m("converted number was out of range."));
                        return false;
                    }
                    while (isspace(*endPtr)) endPtr++;
                    if (*endPtr != '\0') {
                        ACSDK_ERROR(LX("stringToInt64Failed").m("non-whitespace in string after integer."));
                        return false;
                    }
                    *result = static_cast<int64_t>(tempResult);
                    return true;
                }
                std::string replaceAllSubstring(const std::string& str, const std::string& from, const std::string& to) {
                    size_t pos = 0;
                    std::string subject = str;
                    while ((pos = subject.find(from, pos)) != std::string::npos) {
                        subject.replace(pos, from.length(), to);
                        pos += to.length();
                    }
                    return subject;
                }
                std::string ltrim(const std::string& str) {
                    if (str.empty()) return str;
                    auto iter = std::find_if(str.begin(), str.end(), [](char c) { return !std::isspace(c); });
                    return std::string(iter, str.end());
                }
            }
        }
    }
}

#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STRING_STRINGUTILS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STRING_STRINGUTILS_H_

#include <string>
#include <vector>
#include <stdint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace string {
                bool stringToInt(const std::string& str, int* result);
                bool stringToInt(const char* str, int* result);
                bool stringToInt64(const std::string& str, int64_t* result);
                bool stringToInt64(const char* str, int64_t* result);
                std::string byteVectorToString(const std::vector<uint8_t>& byteVector);
                std::string stringToLowerCase(const std::string& input);
                std::string stringToUpperCase(const std::string& input);
                std::string replaceAllSubstring(const std::string& str, const std::string& from, const std::string& to);
                std::string ltrim(const std::string& str);
            }
        }
    }
}
#endif
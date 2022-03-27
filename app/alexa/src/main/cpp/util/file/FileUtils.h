#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_FILE_FILEUTILS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_FILE_FILEUTILS_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace file {
                bool fileExists(const std::string& filePath);
                bool removeFile(const std::string& filePath);
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_FILE_FILEUTILS_H_

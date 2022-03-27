#include <cstdio>
#include <fstream>
#include <logger/Logger.h>
#include "file/FileUtils.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace file {
                static const std::string TAG("FileUtils");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                bool fileExists(const std::string& filePath) {
                    std::ifstream is(filePath);
                    return is.good();
                }
                bool removeFile(const std::string& filePath) {
                    if (!fileExists(filePath)) {
                        ACSDK_WARN(LX("removeFileFailed").d("reason", "File does not exist."));
                        return false;
                    }
                    int result = std::remove(filePath.c_str());
                    if (result != 0) {
                        ACSDK_ERROR(LX("removeFileFailed").d("reason", "could not remove file.").d("result", result));
                        return false;
                    }
                    return true;
                }
            }
        }
    }
}

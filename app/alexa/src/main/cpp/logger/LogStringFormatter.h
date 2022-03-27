#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGSTRINGFORMATTER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGSTRINGFORMATTER_H_

#include <chrono>
#include "Logger.h"
#include "timing/SafeCTimeAccess.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class LogStringFormatter {
                public:
                    LogStringFormatter();
                    std::string format(
                        Level level,
                        std::chrono::system_clock::time_point time,
                        const char* threadMoniker,
                        const char* text);
                private:
                    std::shared_ptr<timing::SafeCTimeAccess> m_safeCTimeAccess;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGSTRINGFORMATTER_H_

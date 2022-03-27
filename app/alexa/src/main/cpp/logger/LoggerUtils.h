#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGERUTILS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGERUTILS_H_

#include <chrono>
#include "Logger.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                void acsdkDebug9(const LogEntry& entry);
                void acsdkDebug8(const LogEntry& entry);
                void acsdkDebug7(const LogEntry& entry);
                void acsdkDebug6(const LogEntry& entry);
                void acsdkDebug5(const LogEntry& entry);
                void acsdkDebug4(const LogEntry& entry);
                void acsdkDebug3(const LogEntry& entry);
                void acsdkDebug2(const LogEntry& entry);
                void acsdkDebug1(const LogEntry& entry);
                void acsdkDebug0(const LogEntry& entry);
                void acsdkDebug(const LogEntry& entry);
                void acsdkInfo(const LogEntry& entry);
                void acsdkWarn(const LogEntry& entry);
                void acsdkError(const LogEntry& entry);
                void acsdkCritical(const LogEntry& entry);
                void logEntry(Level level, const LogEntry& entry);
                void dumpBytesToStream(std::ostream& stream, const char* prefix, size_t width, const unsigned char* data, size_t size);
            }
        }
    }
}
#endif
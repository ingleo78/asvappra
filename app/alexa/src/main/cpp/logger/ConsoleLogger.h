#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_CONSOLELOGGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_CONSOLELOGGER_H_

#include "Logger.h"
#include "LoggerUtils.h"
#include "LogStringFormatter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class ConsoleLogger : public Logger, private std::ios_base::Init {
                    public:
                        static std::shared_ptr<Logger> instance();
                        void emit(Level level, std::chrono::system_clock::time_point time, const char* threadMoniker, const char* text) override;
                    private:
                        ConsoleLogger();
                        std::shared_ptr<std::mutex> m_coutMutex;
                        LogStringFormatter m_logFormatter;
                };
                std::shared_ptr<Logger> getConsoleLogger();
            }
        }
    }
}
#endif
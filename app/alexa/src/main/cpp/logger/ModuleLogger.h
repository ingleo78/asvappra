#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_MODULELOGGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_MODULELOGGER_H_

#include <string>
#include "Logger.h"
#include "LoggerSinkManager.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class ModuleLogger : public Logger, protected LogLevelObserverInterface, protected SinkObserverInterface {
                    public:
                        ModuleLogger(const std::string& configKey);
                        void setLevel(Level level) override;
                        void emit(Level level, std::chrono::system_clock::time_point time, const char* threadId, const char* text) override;
                    private:
                        void onLogLevelChanged(Level level) override;
                        void onSinkChanged(const std::shared_ptr<Logger>& sink) override;
                        void updateLogLevel();
                        Level m_moduleLogLevel;
                        Level m_sinkLogLevel;
                    protected:
                        std::shared_ptr<Logger> m_sink;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_MODULELOGGER_H_

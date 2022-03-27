#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGERSINKMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGERSINKMANAGER_H_

#include <atomic>
#include <mutex>
#include <vector>
#include "Logger.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class LoggerSinkManager {
                    public:
                        static LoggerSinkManager& instance();
                        void addSinkObserver(SinkObserverInterface* observer);
                        void removeSinkObserver(SinkObserverInterface* observer);
                        void setLevel(Level level);
                        void initialize(const std::shared_ptr<Logger>& sink);
                    private:
                        LoggerSinkManager();
                        std::mutex m_sinkObserverMutex;
                        std::vector<SinkObserverInterface*> m_sinkObservers;
                        std::shared_ptr<Logger> m_sink;
                        Level m_level;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGERSINKMANAGER_H_

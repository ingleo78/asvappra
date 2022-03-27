#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGER_H_

#include <atomic>
#include <chrono>
#include <mutex>
#include <sstream>
#include <vector>
#include "../configuration/ConfigurationNode.h"
#include "Level.h"
#include "LogEntry.h"
#include "LogLevelObserverInterface.h"
#include "SinkObserverInterface.h"

#define ACSDK_STRINGIFY_INNER(expression) #expression
#define ACSDK_STRINGIFY(macro) ACSDK_STRINGIFY_INNER(macro)
#define ACSDK_CONCATENATE_INNER(lhs, rhs) lhs##rhs
#define ACSDK_CONCATENATE(lhs, rhs) ACSDK_CONCATENATE_INNER(lhs, rhs)

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class Logger {
                    public:
                        Logger(Level level);
                        virtual ~Logger() = default;
                        virtual void setLevel(Level level);
                        inline bool shouldLog(Level level) const;
                        void log(Level level, const LogEntry& entry);
                        void logAtExit(Level level, const LogEntry& entry);
                        virtual void emit(Level level, std::chrono::system_clock::time_point time, const char* threadMoniker, const char* text) = 0;
                        void addLogLevelObserver(LogLevelObserverInterface* observer);
                        void removeLogLevelObserver(LogLevelObserverInterface* observer);
                    protected:
                        void init(const configuration::ConfigurationNode configuration);
                        std::atomic<Level> m_level;
                    private:
                        bool initLogLevel(const configuration::ConfigurationNode configuration);
                        void notifyObserversOnLogLevelChanged();
                        std::vector<LogLevelObserverInterface*> m_observers;
                        std::mutex m_observersMutex;
                };
                bool Logger::shouldLog(Level level) const {
                    return level >= m_level;
                }
                #define ACSDK_GET_LOGGER_FUNCTION_NAME(type) ACSDK_CONCATENATE(ACSDK_CONCATENATE(get, type), Logger)
                #ifndef ACSDK_LOG_SINK
                    #define ACSDK_LOG_SINK Console
                #endif
                #define ACSDK_GET_SINK_LOGGER ACSDK_GET_LOGGER_FUNCTION_NAME(ACSDK_LOG_SINK)
                std::shared_ptr<Logger> ACSDK_GET_SINK_LOGGER();
            }
        }
    }
}
#ifdef ACSDK_LOG_MODULE
    #include "AVSCommon/Utils/Logger/ModuleLogger.h"
    namespace alexaClientSDK {
        namespace avsCommon {
            namespace utils {
                namespace logger {
                    #define ACSDK_GET_LOGGER_FUNCTION ACSDK_GET_LOGGER_FUNCTION_NAME(ACSDK_LOG_MODULE)
                    inline Logger& ACSDK_GET_LOGGER_FUNCTION() {
                        static ModuleLogger moduleLogger(ACSDK_STRINGIFY(ACSDK_LOG_MODULE));
                        return moduleLogger;
                    }
                }
            }
        }
    }
#else
    namespace alexaClientSDK {
        namespace avsCommon {
            namespace utils {
                namespace logger {
                    inline Logger& ACSDK_GET_LOGGER_FUNCTION() {
                        static std::shared_ptr<Logger> logger = ACSDK_GET_SINK_LOGGER();
                        return *logger;
                    }
                }
            }
        }
    }
#endif
#define ACSDK_LOG(level, entry)                                                                       \
    do {                                                                                              \
        auto& loggerInstance = alexaClientSDK::avsCommon::utils::logger::ACSDK_GET_LOGGER_FUNCTION(); \
        if (loggerInstance.shouldLog(level)) {                                                        \
            loggerInstance.log(level, entry);                                                         \
        }                                                                                             \
    } while (false)
#ifdef ACSDK_DEBUG_LOG_ENABLED
    #define ACSDK_DEBUG9(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG9, entry)
    #define ACSDK_DEBUG8(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG8, entry)
    #define ACSDK_DEBUG7(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG7, entry)
    #define ACSDK_DEBUG6(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG6, entry)
    #define ACSDK_DEBUG5(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG5, entry)
    #define ACSDK_DEBUG4(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG4, entry)
    #define ACSDK_DEBUG3(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG3, entry)
    #define ACSDK_DEBUG2(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG2, entry)
    #define ACSDK_DEBUG1(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG1, entry)
    #define ACSDK_DEBUG0(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG0, entry)
    #define ACSDK_DEBUG(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG0, entry)
#else
    #define ACSDK_DEBUG9(entry)
    #define ACSDK_DEBUG8(entry)
    #define ACSDK_DEBUG7(entry)
    #define ACSDK_DEBUG6(entry)
    #define ACSDK_DEBUG5(entry)
    #define ACSDK_DEBUG4(entry)
    #define ACSDK_DEBUG3(entry)
    #define ACSDK_DEBUG2(entry)
    #define ACSDK_DEBUG1(entry)
    #define ACSDK_DEBUG0(entry)
    #define ACSDK_DEBUG(entry)
#endif
#define ACSDK_INFO(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::INFO, entry)
#define ACSDK_WARN(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::WARN, entry)
#define ACSDK_ERROR(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::ERROR, entry)
#define ACSDK_CRITICAL(entry) ACSDK_LOG(alexaClientSDK::avsCommon::utils::logger::Level::CRITICAL, entry)
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGGER_H_

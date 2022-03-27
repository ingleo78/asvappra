#include <algorithm>
#include <chrono>
#include "Logger.h"
#include "ThreadMoniker.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                static const std::string CONFIG_KEY_LOGGER = "logger";
                static const std::string CONFIG_KEY_LOG_LEVEL = "logLevel";
                static constexpr auto AT_EXIT_THREAD_ID = "0";
                Logger::Logger(Level level) : m_level{level} { }
                void Logger::log(Level level, const LogEntry& entry) {
                    if (shouldLog(level)) {
                        emit(level, std::chrono::system_clock::now(), ThreadMoniker::getThisThreadMoniker().c_str(), entry.c_str());
                    }
                }
                void Logger::init(const configuration::ConfigurationNode configuration) {
                    if (!initLogLevel(configuration)) {
                        initLogLevel(configuration::ConfigurationNode::getRoot()[CONFIG_KEY_LOGGER]);
                    }
                }
                bool Logger::initLogLevel(const configuration::ConfigurationNode configuration) {
                    std::string name;
                    if (configuration.getString(CONFIG_KEY_LOG_LEVEL, &name)) {
                        Level level = convertNameToLevel(name);
                        if (level != Level::UNKNOWN) {
                            setLevel(level);
                            return true;
                        } else {
                            log(Level::ERROR, LogEntry("Logger", "unknownLogLevel").d("name", name));
                        }
                    }
                    return false;
                }
                void Logger::setLevel(Level level) {
                    if (m_level != level) {
                        m_level = level;
                        notifyObserversOnLogLevelChanged();
                    }
                    #ifndef ACSDK_DEBUG_LOG_ENABLED
                        if (m_level <= Level::DEBUG0) {
                            log(Level::WARN,
                                LogEntry("Logger", "debugLogLevelSpecifiedWhenDebugLogsCompiledOut")
                                    .d("level", m_level)
                                    .m("\n"
                                       "\nWARNING: By default DEBUG logs are compiled out of RELEASE builds."
                                       "\nRebuild with the cmake parameter -DCMAKE_BUILD_TYPE=DEBUG to enable debug logs."
                                       "\n"));
                        }
                    #endif
                }
                void Logger::addLogLevelObserver(LogLevelObserverInterface* observer) {
                    {
                        std::lock_guard<std::mutex> lock(m_observersMutex);
                        m_observers.push_back(observer);
                    }
                    observer->onLogLevelChanged(m_level);
                }
                void Logger::removeLogLevelObserver(LogLevelObserverInterface* observer) {
                    std::lock_guard<std::mutex> lock(m_observersMutex);
                    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
                }
                void Logger::notifyObserversOnLogLevelChanged() {
                    std::vector<LogLevelObserverInterface*> observersCopy;
                    {
                        std::lock_guard<std::mutex> lock(m_observersMutex);
                        observersCopy = m_observers;
                    }
                    for (auto observer : observersCopy) {
                        observer->onLogLevelChanged(m_level);
                    }
                }
                void Logger::logAtExit(Level level, const LogEntry& entry) {
                    if (shouldLog(level)) {
                        emit(level, std::chrono::system_clock::now(), AT_EXIT_THREAD_ID, entry.c_str());
                    }
                }
            }
        }
    }
}

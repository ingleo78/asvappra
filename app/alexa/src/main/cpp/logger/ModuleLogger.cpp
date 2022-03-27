#include "ModuleLogger.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                void ModuleLogger::emit(
                    Level level,
                    std::chrono::system_clock::time_point time,
                    const char* threadId,
                    const char* text) {
                    if (shouldLog(level)) m_sink->emit(level, time, threadId, text);
                }
                void ModuleLogger::setLevel(Level level) {
                    m_moduleLogLevel = level;
                    updateLogLevel();
                }
                void ModuleLogger::onLogLevelChanged(Level level) {
                    m_sinkLogLevel = level;
                    updateLogLevel();
                }
                void ModuleLogger::onSinkChanged(const std::shared_ptr<Logger>& logger) {
                    if (m_sink)  m_sink->removeLogLevelObserver(this);
                    m_sink = logger;
                    m_sink->addLogLevelObserver(this);
                }
                void ModuleLogger::updateLogLevel() {
                    if (Level::UNKNOWN == m_sinkLogLevel) Logger::setLevel(m_moduleLogLevel);
                    else if (Level::UNKNOWN == m_moduleLogLevel) Logger::setLevel(m_sinkLogLevel);
                    else Logger::setLevel((m_sinkLogLevel > m_moduleLogLevel) ? m_sinkLogLevel : m_moduleLogLevel);
                }
                ModuleLogger::ModuleLogger(const std::string& configKey) : Logger(Level::UNKNOWN), m_moduleLogLevel(Level::UNKNOWN), m_sinkLogLevel(Level::UNKNOWN),
                        m_sink(nullptr) {
                    LoggerSinkManager::instance().addSinkObserver(this);
                    init(configuration::ConfigurationNode::getRoot()[configKey]);
                }
            }
        }
    }
}

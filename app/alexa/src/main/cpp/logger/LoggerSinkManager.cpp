#include <algorithm>
#include "LoggerSinkManager.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                LoggerSinkManager& LoggerSinkManager::instance() {
                    static LoggerSinkManager singleLoggerSinkManager;
                    return singleLoggerSinkManager;
                }
                void LoggerSinkManager::addSinkObserver(SinkObserverInterface* observer) {
                    if (!observer) return;
                    {
                        std::lock_guard<std::mutex> lock(m_sinkObserverMutex);
                        m_sinkObservers.push_back(observer);
                    }
                    observer->onSinkChanged(m_sink);
                }
                void LoggerSinkManager::removeSinkObserver(SinkObserverInterface* observer) {
                    if (!observer) return;
                    std::lock_guard<std::mutex> lock(m_sinkObserverMutex);
                    m_sinkObservers.erase(std::remove(m_sinkObservers.begin(), m_sinkObservers.end(), observer), m_sinkObservers.end());
                }
                void LoggerSinkManager::setLevel(Level level) {
                    m_level = level;
                    if (m_sink) m_sink->setLevel(level);
                }

                void LoggerSinkManager::initialize(const std::shared_ptr<Logger>& sink) {
                    if (!sink) return;
                    if (m_sink == sink) return;
                    if (m_level != Level::UNKNOWN) sink->setLevel(m_level);
                    std::vector<SinkObserverInterface*> observersCopy;
                    {
                        std::lock_guard<std::mutex> lock(m_sinkObserverMutex);
                        observersCopy = m_sinkObservers;
                    }
                    m_sink = sink;
                    for (auto observer : observersCopy) {
                        observer->onSinkChanged(m_sink);
                    }
                }
                LoggerSinkManager::LoggerSinkManager() : m_sink{ACSDK_GET_SINK_LOGGER()}, m_level(Level::UNKNOWN) {}
            }
        }
    }
}

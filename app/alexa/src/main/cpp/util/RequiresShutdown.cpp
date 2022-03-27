#include <mutex>
#include <unordered_set>
#include <logger/Logger.h>
#include <logger/ModuleLogger.h>
#include "RequiresShutdown.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            static const std::string TAG("RequiresShutdown");
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            class ShutdownMonitor {
            public:
                void add(const RequiresShutdown* object);
                void remove(const RequiresShutdown* object);
                ShutdownMonitor();
                ~ShutdownMonitor();
            private:
                std::mutex m_mutex;
                using Objects = std::unordered_set<const RequiresShutdown*>;
                Objects m_objects;
                alexaClientSDK::avsCommon::utils::logger::ModuleLogger m_destructorLogger;
            };
            void ShutdownMonitor::add(const RequiresShutdown* object) {
                if (nullptr == object) {
                    ACSDK_ERROR(LX("addFailed").d("reason", "nullptrObject"));
                }
                bool inserted = false;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    inserted = m_objects.insert(object).second;
                }
                if (!inserted) {
                    ACSDK_ERROR(LX("addFailed").d("reason", "alreadyAdded").d("name", object->name()));
                }
            }
            void ShutdownMonitor::remove(const RequiresShutdown* object) {
                if (nullptr == object) {
                    ACSDK_ERROR(LX("removeFailed").d("reason", "nullptrObject"));
                }
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_objects.erase(object) == 0) {
                    ACSDK_ERROR(LX("removeFailed").d("reason", "notFound").d("name", object->name()));
                }
            }
            ShutdownMonitor::ShutdownMonitor() : m_destructorLogger(ACSDK_STRINGIFY(ACSDK_LOG_MODULE)) {}
            ShutdownMonitor::~ShutdownMonitor() {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (auto object : m_objects) {
                    if (!object->isShutdown()) {
                        m_destructorLogger.logAtExit(
                            alexaClientSDK::avsCommon::utils::logger::Level::WARN,
                            LX("ShutdownMonitor").d("reason", "no shutdown() call").d("name: ", object->name()));
                    }
                    m_destructorLogger.logAtExit(logger::Level::WARN, LX("ShutdownMonitor").d("reason", "never deleted").d("name", object->name()));
                }
            }
            static ShutdownMonitor g_shutdownMonitor;
            RequiresShutdown::RequiresShutdown(const std::string& name) : m_name{name}, m_isShutdown{false} {
                g_shutdownMonitor.add(this);
            }
            RequiresShutdown::~RequiresShutdown() {
                if (!m_isShutdown) {
                    ACSDK_ERROR(LX("~RequiresShutdownFailed").d("reason", "notShutdown").d("name", name()));
                }
                g_shutdownMonitor.remove(this);
            }
            const std::string& RequiresShutdown::name() const {
                return m_name;
            }
            void RequiresShutdown::shutdown() {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_isShutdown) {
                    ACSDK_ERROR(LX("shutdownFailed").d("reason", "alreadyShutdown").d("name", name()));
                    return;
                }
                doShutdown();
                m_isShutdown = true;
            }
            bool RequiresShutdown::isShutdown() const {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_isShutdown;
            }
        }
    }
}

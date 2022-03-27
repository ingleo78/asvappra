#include "WaitEvent.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            WaitEvent::WaitEvent() : m_wakeUpTriggered{false} {}
            void WaitEvent::wakeUp() {
                std::lock_guard<std::mutex> lock{m_mutex};
                m_wakeUpTriggered = true;
                m_condition.notify_all();
            }
            bool WaitEvent::wait(const std::chrono::milliseconds& timeout) {
                std::unique_lock<std::mutex> lock{m_mutex};
                return m_condition.wait_for(lock, timeout, [this] { return m_wakeUpTriggered; });
            }
            void WaitEvent::reset() {
                std::lock_guard<std::mutex> lock{m_mutex};
                m_wakeUpTriggered = false;
            }
        }
    }
}

#include <logger/Logger.h>
#include <timing/SystemClockMonitor.h>


static const std::string TAG("SystemClockMonitor");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                void SystemClockMonitor::notifySystemClockSynchronized() {
                    notifyObservers();
                }
                void SystemClockMonitor::addSystemClockMonitorObserver(
                    const std::shared_ptr<avsCommon::sdkInterfaces::SystemClockMonitorObserverInterface>& observer) {
                    if (!observer) {
                        ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                        return;
                    }
                    std::lock_guard<std::mutex> lock{m_systemClockObserverMutex};
                    m_observers.insert(observer);
                }
                void SystemClockMonitor::removeSystemClockMonitorObserver(
                    const std::shared_ptr<avsCommon::sdkInterfaces::SystemClockMonitorObserverInterface>& observer) {
                    if (!observer) {
                        ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                        return;
                    }
                    std::lock_guard<std::mutex> lock{m_systemClockObserverMutex};
                    m_observers.erase(observer);
                }
                void SystemClockMonitor::notifyObservers() {
                    std::unique_lock<std::mutex> lock{m_systemClockObserverMutex};
                    auto observersCopy = m_observers;
                    lock.unlock();
                    for (const auto& observer : observersCopy) {
                        observer->onSystemClockSynchronized();
                    }
                }
            }
        }
    }
}

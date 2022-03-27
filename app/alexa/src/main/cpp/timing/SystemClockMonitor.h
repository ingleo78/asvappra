#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_SYSTEMCLOCKMONITOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_SYSTEMCLOCKMONITOR_H_

#include <memory>
#include <mutex>
#include <unordered_set>
#include <sdkinterfaces/SystemClockMonitorObserverInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                class SystemClockMonitor {
                public:
                    void notifySystemClockSynchronized();
                    void addSystemClockMonitorObserver(const std::shared_ptr<avsCommon::sdkInterfaces::SystemClockMonitorObserverInterface>& observer);
                    void removeSystemClockMonitorObserver(const std::shared_ptr<avsCommon::sdkInterfaces::SystemClockMonitorObserverInterface>& observer);
                private:
                    void notifyObservers();
                    std::mutex m_systemClockObserverMutex;
                    std::unordered_set<std::shared_ptr<sdkInterfaces::SystemClockMonitorObserverInterface>> m_observers;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_SYSTEMCLOCKMONITOR_H_

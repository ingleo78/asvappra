#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTBUS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTBUS_H_

#include <memory>
#include <mutex>
#include <unordered_map>
#include <list>
#include <string>
#include <vector>
#include "BluetoothEvents.h"
#include "BluetoothEventListenerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                class BluetoothEventBus {
                public:
                    BluetoothEventBus();
                    using ListenerList = std::list<std::weak_ptr<BluetoothEventListenerInterface>>;
                    void sendEvent(const BluetoothEvent& event);
                    void addListener(
                        const std::vector<BluetoothEventType>& eventTypes,
                        std::shared_ptr<BluetoothEventListenerInterface> listener);
                    void removeListener(
                        const std::vector<BluetoothEventType>& eventTypes,
                        std::shared_ptr<BluetoothEventListenerInterface> listener);
                private:
                    std::mutex m_mutex;
                    std::unordered_map<BluetoothEventType, ListenerList, BluetoothEventTypeHash> m_listenerMap;
                };
            }
        }
    }
}
#endif
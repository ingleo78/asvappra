#include <logger/Logger.h>
#include "bluetooth/BluetoothEventBus.h"
#include "error/FinallyGuard.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                static const std::string TAG{"BluetoothEventBus"};
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                BluetoothEventBus::BluetoothEventBus() {}
                void BluetoothEventBus::sendEvent(const BluetoothEvent& event) {
                    ListenerList listenerList;
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        auto mapIterator = m_listenerMap.find(event.getType());
                        if (mapIterator == m_listenerMap.end()) {
                            // No listeners registered
                            return;
                        }
                        listenerList = mapIterator->second;
                    }
                    for (auto listenerWeakPtr : listenerList) {
                        std::shared_ptr<BluetoothEventListenerInterface> listener = listenerWeakPtr.lock();
                        if (listener != nullptr) {
                            listener->onEventFired(event);
                        }
                    }
                }
                void BluetoothEventBus::addListener(const std::vector<BluetoothEventType>& eventTypes, std::shared_ptr<BluetoothEventListenerInterface> listener) {
                    if (listener == nullptr) {
                        ACSDK_ERROR(LX("addListenerFailed").d("reason", "Listener cannot be null"));
                        return;
                    }
                    std::lock_guard<std::mutex> lock(m_mutex);
                    for (BluetoothEventType eventType : eventTypes) {
                        ListenerList& listenerList = m_listenerMap[eventType];
                        auto iter = listenerList.begin();
                        while (iter != listenerList.end()) {
                            auto listenerWeakPtr = *iter;
                            auto listenerPtr = listenerWeakPtr.lock();
                            if (listenerPtr == nullptr) {
                                iter = listenerList.erase(iter);
                            } else {
                                if (listenerPtr == listener) {
                                    ACSDK_ERROR(LX("addListenerFailed").d("reason", "The same listener already exists"));
                                    break;
                                }
                                ++iter;
                            }
                        }
                        if (iter == listenerList.end()) {
                            listenerList.push_back(listener);
                        }
                    }
                }
                void BluetoothEventBus::removeListener(const std::vector<BluetoothEventType>& eventTypes, std::shared_ptr<BluetoothEventListenerInterface> listener) {
                    if (listener == nullptr) {
                        ACSDK_ERROR(LX("removeListenerFailed").d("reason", "Listener cannot be null"));
                        return;
                    }
                    std::lock_guard<std::mutex> lock(m_mutex);
                    for (BluetoothEventType eventType : eventTypes) {
                        auto mapIterator = m_listenerMap.find(eventType);
                        if (mapIterator == m_listenerMap.end()) {
                            ACSDK_ERROR(LX("removeListenerFailed").d("reason", "Listener not subscribed"));
                            continue;
                        }
                        ListenerList& listenerList = mapIterator->second;
                        auto iter = listenerList.begin();
                        while (iter != listenerList.end()) {
                            auto listenerWeakPtr = *iter;
                            auto listenerPtr = listenerWeakPtr.lock();
                            if (listenerPtr == nullptr) {
                                iter = listenerList.erase(iter);
                            } else if (listenerPtr == listener) {
                                listenerList.erase(iter);
                                break;
                            } else {
                                ++iter;
                            }
                        }
                        if (listenerList.empty()) {
                            m_listenerMap.erase(mapIterator);
                        }
                    }
                }
            }
        }
    }
}

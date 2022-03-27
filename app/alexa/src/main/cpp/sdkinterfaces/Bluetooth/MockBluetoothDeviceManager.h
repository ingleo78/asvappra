#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHDEVICEMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHDEVICEMANAGER_H_

#include <gmock/gmock.h>
#include "BluetoothDeviceManagerInterface.h"
#include "BluetoothHostControllerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace test {
                    using namespace std;
                    using namespace utils;
                    using namespace utils::bluetooth;
                    using namespace testing;
                    class MockBluetoothDeviceManager : public BluetoothDeviceManagerInterface {
                    public:
                        shared_ptr<BluetoothHostControllerInterface> getHostController();
                        list<std::shared_ptr<BluetoothDeviceInterface>> getDiscoveredDevices() override;
                        shared_ptr<BluetoothEventBus> getEventBus() override;
                        MockBluetoothDeviceManager(shared_ptr<BluetoothHostControllerInterface> hostcontroller,
                                                   list<shared_ptr<BluetoothDeviceInterface>> discoveredDevices, shared_ptr<BluetoothEventBus> eventBus);
                    protected:
                        shared_ptr<BluetoothHostControllerInterface> m_hostController;
                        list<shared_ptr<BluetoothDeviceInterface>> m_discoveredDevices;
                        shared_ptr<BluetoothEventBus> m_eventBus;
                    };
                    inline shared_ptr<BluetoothHostControllerInterface> MockBluetoothDeviceManager::getHostController() {
                        return m_hostController;
                    }
                    inline list<shared_ptr<BluetoothDeviceInterface>> MockBluetoothDeviceManager::getDiscoveredDevices() {
                        return m_discoveredDevices;
                    }
                    inline shared_ptr<BluetoothEventBus> MockBluetoothDeviceManager::getEventBus() {
                        return m_eventBus;
                    }
                    inline MockBluetoothDeviceManager::MockBluetoothDeviceManager(shared_ptr<BluetoothHostControllerInterface> hostcontroller,
                                                                                  list<shared_ptr<BluetoothDeviceInterface>> discoveredDevices,
                                                                                  shared_ptr<BluetoothEventBus> eventBus) : m_hostController{hostcontroller},
                                                                                  m_discoveredDevices(discoveredDevices), m_eventBus(eventBus) {}
                }
            }
        }
    }
}
#endif
#ifndef ACSDKBLUETOOTHINTERFACES_MOCKBLUETOOTHDEVICEOBSERVER_H_
#define ACSDKBLUETOOTHINTERFACES_MOCKBLUETOOTHDEVICEOBSERVER_H_

#include <gmock/gmock.h>
#include "BluetoothDeviceObserverInterface.h"

namespace alexaClientSDK {
    namespace acsdkBluetoothInterfaces {
        namespace test {
            class MockBluetoothDeviceObserver : public BluetoothDeviceObserverInterface {
            public:
                MOCK_METHOD1(onActiveDeviceConnected, void(const DeviceAttributes& attributes));
                MOCK_METHOD1(onActiveDeviceDisconnected, void(const DeviceAttributes& attributes));
            };
        }
    }
}
#endif
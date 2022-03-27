#ifndef ACSDKBLUETOOTHINTERFACES_BLUETOOTHDEVICEOBSERVERINTERFACE_H_
#define ACSDKBLUETOOTHINTERFACES_BLUETOOTHDEVICEOBSERVERINTERFACE_H_

#include <string>
#include <unordered_set>

namespace alexaClientSDK {
    namespace acsdkBluetoothInterfaces {
        using namespace std;
        class BluetoothDeviceObserverInterface {
        public:
            struct DeviceAttributes {
                DeviceAttributes() = default;
                string name;
                unordered_set<string> supportedServices;
            };
            virtual ~BluetoothDeviceObserverInterface() = default;
            virtual void onActiveDeviceConnected(const DeviceAttributes& attributes) = 0;
            virtual void onActiveDeviceDisconnected(const DeviceAttributes& attributes) = 0;
        };
    }
}
#endif
#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICEMANAGERINTERFACE_H_

#include <list>
#include <memory>
#include <util/bluetooth/BluetoothEventBus.h>
#include "BluetoothHostControllerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                class BluetoothDeviceManagerInterface {
                public:
                    virtual ~BluetoothDeviceManagerInterface() = default;
                    virtual std::shared_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothHostControllerInterface> getHostController();
                    virtual std::list<std::shared_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceInterface>> getDiscoveredDevices() = 0;
                    virtual std::shared_ptr<avsCommon::utils::bluetooth::BluetoothEventBus> getEventBus() = 0;
                };
            }
        }
    }
}
#endif
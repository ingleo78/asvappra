#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZBLUETOOTHDEVICEMANAGER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_BLUEZBLUETOOTHDEVICEMANAGER_H_

#include <sdkinterfaces/Bluetooth/BluetoothDeviceManagerInterface.h>
#include <util/bluetooth/BluetoothEventBus.h>
#include "bluetooth_implementations/BlueZDeviceManager.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class BlueZBluetoothDeviceManager : public avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface {
            public:
                static std::unique_ptr<BlueZBluetoothDeviceManager> create(std::shared_ptr<avsCommon::utils::bluetooth::BluetoothEventBus> eventBus);
                virtual ~BlueZBluetoothDeviceManager() override;
                std::shared_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothHostControllerInterface> getHostController();
                std::list<std::shared_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceInterface>> getDiscoveredDevices() override;
                std::shared_ptr<avsCommon::utils::bluetooth::BluetoothEventBus> getEventBus() override;
            private:
                BlueZBluetoothDeviceManager(std::shared_ptr<BlueZDeviceManager> deviceManager);
                std::shared_ptr<BlueZDeviceManager> m_deviceManager;
            };
        }
    }
}
#endif
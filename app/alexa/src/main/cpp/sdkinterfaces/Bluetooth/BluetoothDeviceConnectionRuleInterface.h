#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICECONNECTIONRULEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_BLUETOOTHDEVICECONNECTIONRULEINTERFACE_H_

#include <map>
#include <set>
#include <util/bluetooth/DeviceCategory.h>
#include "BluetoothDeviceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                class BluetoothDeviceConnectionRuleInterface {
                public:
                    virtual ~BluetoothDeviceConnectionRuleInterface() = default;
                    virtual bool shouldExplicitlyConnect() = 0;
                    virtual bool shouldExplicitlyDisconnect() = 0;
                    virtual std::set<std::shared_ptr<BluetoothDeviceInterface>> devicesToDisconnect(
                        std::map<DeviceCategory, std::set<std::shared_ptr<BluetoothDeviceInterface>>> connectedDevices) = 0;
                    virtual std::set<DeviceCategory> getDeviceCategories() = 0;
                    virtual std::set<std::string> getDependentProfiles() = 0;
                };
            }
        }
    }
}
#endif
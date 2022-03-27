#ifndef ACSDKBLUETOOTH_BASICDEVICECONNECTIONRULE_H_
#define ACSDKBLUETOOTH_BASICDEVICECONNECTIONRULE_H_

#include <sdkinterfaces/Bluetooth/BluetoothDeviceConnectionRuleInterface.h>

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace sdkInterfaces::bluetooth;
        class BasicDeviceConnectionRule : public BluetoothDeviceConnectionRuleInterface {
        public:
            static shared_ptr<BasicDeviceConnectionRule> create();
            bool shouldExplicitlyConnect() override;
            bool shouldExplicitlyDisconnect() override;
            set<shared_ptr<BluetoothDeviceInterface>> devicesToDisconnect(map<DeviceCategory, set<shared_ptr<BluetoothDeviceInterface>>> connectedDevices) override;
            set<DeviceCategory> getDeviceCategories() override;
            set<string> getDependentProfiles() override;
        private:
            BasicDeviceConnectionRule();
            set<DeviceCategory> m_categories;
            set<string> m_profiles;
        };
    }
}
#endif
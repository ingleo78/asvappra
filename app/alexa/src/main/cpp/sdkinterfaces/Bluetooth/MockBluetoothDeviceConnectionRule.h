#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHDEVICECONNECTIONRULE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_MOCKBLUETOOTHDEVICECONNECTIONRULE_H_

#include <gmock/gmock.h>
#include "BluetoothDeviceConnectionRuleInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace test {
                    using namespace std;
                    class MockBluetoothDeviceConnectionRule : public BluetoothDeviceConnectionRuleInterface {
                    public:
                        MockBluetoothDeviceConnectionRule(set<DeviceCategory> categories, set<string> profiles);
                        bool shouldExplicitlyConnect() override;
                        bool shouldExplicitlyDisconnect() override;
                        set<shared_ptr<BluetoothDeviceInterface>> devicesToDisconnect(map<DeviceCategory, set<shared_ptr<BluetoothDeviceInterface>>> connectedDevices) override;
                        set<DeviceCategory> getDeviceCategories() override;
                        set<string> getDependentProfiles() override;
                        void setExplicitlyConnect(bool explicitlyConnect);
                        void setExplicitlyDisconnect(bool explicitlyDisconnect);
                        void setDevicesToDisconnect(set<shared_ptr<BluetoothDeviceInterface>> devices);
                    protected:
                        set<DeviceCategory> m_categories;
                        set<string> m_profiles;
                        bool m_explicitlyConnect;
                        bool m_explicitlyDisconnect;
                        set<shared_ptr<BluetoothDeviceInterface>> m_disconnectedDevices;
                    };
                    MockBluetoothDeviceConnectionRule::MockBluetoothDeviceConnectionRule(set<DeviceCategory> categories, set<string> profiles) :
                                                                                         m_categories{categories}, m_profiles{profiles}, m_explicitlyConnect{false},
                                                                                         m_explicitlyDisconnect{false} {}
                    set<DeviceCategory> MockBluetoothDeviceConnectionRule::getDeviceCategories() {
                        return m_categories;
                    }
                    set<string> MockBluetoothDeviceConnectionRule::getDependentProfiles() {
                        return m_profiles;
                    }
                    bool MockBluetoothDeviceConnectionRule::shouldExplicitlyConnect() {
                        return m_explicitlyConnect;
                    }
                    bool MockBluetoothDeviceConnectionRule::shouldExplicitlyDisconnect() {
                        return m_explicitlyDisconnect;
                    }
                    set<shared_ptr<BluetoothDeviceInterface>> MockBluetoothDeviceConnectionRule::devicesToDisconnect(map<DeviceCategory,
                            set<shared_ptr<BluetoothDeviceInterface>>> connectedDevices) {
                        return m_disconnectedDevices;
                    }
                    void MockBluetoothDeviceConnectionRule::setExplicitlyConnect(bool explicitlyConnect) {
                        m_explicitlyConnect = explicitlyConnect;
                    }
                    void MockBluetoothDeviceConnectionRule::setExplicitlyDisconnect(bool explicitlyDisconnect) {
                        m_explicitlyDisconnect = explicitlyDisconnect;
                    }
                    void MockBluetoothDeviceConnectionRule::setDevicesToDisconnect(set<shared_ptr<BluetoothDeviceInterface>> devices) {
                        m_disconnectedDevices = devices;
                    }
                }
            }
        }
    }
}
#endif
#include <sdkinterfaces/Bluetooth/Services/A2DPSinkInterface.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPControllerInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include "BasicDeviceConnectionRule.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace sdkInterfaces::bluetooth::services;
        shared_ptr<BasicDeviceConnectionRule> BasicDeviceConnectionRule::create() {
            return shared_ptr<BasicDeviceConnectionRule>(new BasicDeviceConnectionRule());
        }
        bool BasicDeviceConnectionRule::shouldExplicitlyConnect() {
            return true;
        }
        bool BasicDeviceConnectionRule::shouldExplicitlyDisconnect() {
            return true;
        }
        set<shared_ptr<BluetoothDeviceInterface>> BasicDeviceConnectionRule::devicesToDisconnect(map<DeviceCategory, set<shared_ptr<BluetoothDeviceInterface>>> connectedDevices) {
            set<shared_ptr<BluetoothDeviceInterface>> devicesToDisconnect;
            for (const auto& category : m_categories) {
                auto devicesIt = connectedDevices.find(category);
                if (devicesIt != connectedDevices.end()) devicesToDisconnect.insert(devicesIt->second.begin(), devicesIt->second.end());
            }
            return devicesToDisconnect;
        }
        BasicDeviceConnectionRule::BasicDeviceConnectionRule() {
            m_categories = { DeviceCategory::AUDIO_VIDEO, DeviceCategory::PHONE, DeviceCategory::OTHER, DeviceCategory::UNKNOWN };
            m_profiles = { A2DPSinkInterface::UUID, A2DPSourceInterface::UUID, AVRCPControllerInterface::UUID, AVRCPTargetInterface::UUID };
        }
        set<DeviceCategory> BasicDeviceConnectionRule::getDeviceCategories() {
            return m_categories;
        }
        set<string> BasicDeviceConnectionRule::getDependentProfiles() {
            return m_profiles;
        }
    }
}
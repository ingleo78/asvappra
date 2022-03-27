#ifndef ACSDKBLUETOOTH_BLUETOOTHEVENTSTATE_H_
#define ACSDKBLUETOOTH_BLUETOOTHEVENTSTATE_H_

#include <string>
#include <avs/Requester.h>
#include <sdkinterfaces/Bluetooth/BluetoothDeviceInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace sdkInterfaces::bluetooth;
        class BluetoothEventState {
        public:
            DeviceState getDeviceState() const;
            Optional<Requester> getRequester() const;
            Optional<string> getProfileName() const;
            BluetoothEventState(DeviceState state, Optional<Requester> requester, Optional<string> profileName);
        private:
            DeviceState m_state;
            Optional<Requester> m_requester;
            Optional<string> m_profileName;
        };
    }
}
#endif
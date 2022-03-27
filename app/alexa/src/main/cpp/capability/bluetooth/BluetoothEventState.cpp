#include "BluetoothEventState.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        BluetoothEventState::BluetoothEventState(DeviceState state, Optional<Requester> requester, Optional<string> profileName) : m_state(state),
                                                 m_requester(requester), m_profileName(profileName) {}
        Optional<Requester> BluetoothEventState::getRequester() const {
            return m_requester;
        }
        DeviceState BluetoothEventState::getDeviceState() const {
            return m_state;
        }
        Optional<string> BluetoothEventState::getProfileName() const {
            return m_profileName;
        }
    }
}
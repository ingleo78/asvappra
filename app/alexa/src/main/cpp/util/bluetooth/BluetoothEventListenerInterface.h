#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTLISTENERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_BLUETOOTHEVENTLISTENERINTERFACE_H_

#include "BluetoothEvents.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                class BluetoothEventListenerInterface {
                public:
                    virtual ~BluetoothEventListenerInterface() = default;
                    virtual void onEventFired(const BluetoothEvent& event) = 0;
                };
            }
        }
    }
}
#endif
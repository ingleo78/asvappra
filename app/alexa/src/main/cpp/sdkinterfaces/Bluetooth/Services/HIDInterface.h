#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_HIDINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_HIDINTERFACE_H_

#include "BluetoothServiceInterface.h"
#include "MockBluetoothService.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    class HIDInterface : public BluetoothServiceInterface {
                    public:
                        static constexpr const char* UUID = "00001124-0000-1000-8000-00805f9b34fb";
                        static constexpr const char* NAME = "HumanInterfaceDeviceService";
                    };
                }
            }
        }
    }
}
#endif
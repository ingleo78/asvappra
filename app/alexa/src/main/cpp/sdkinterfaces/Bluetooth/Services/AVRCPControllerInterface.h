#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPCONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_AVRCPCONTROLLERINTERFACE_H_

#include "BluetoothServiceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    class AVRCPControllerInterface : public BluetoothServiceInterface {
                    public:
                        static constexpr const char* UUID = "0000110e-0000-1000-8000-00805f9b34fb";
                        static constexpr const char* NAME = "A/V_RemoteControl";
                        virtual ~AVRCPControllerInterface() = default;
                    };
                }
            }
        }
    }
}
#endif
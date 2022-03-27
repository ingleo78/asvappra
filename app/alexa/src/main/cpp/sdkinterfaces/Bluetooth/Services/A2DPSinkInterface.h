#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSINKINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSINKINTERFACE_H_

#include "BluetoothServiceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    class A2DPSinkInterface : public BluetoothServiceInterface {
                    public:
                        static constexpr const char* UUID = "0000110b-0000-1000-8000-00805f9b34fb";
                        static constexpr const char* NAME = "AudioSink";
                    };
                }
            }
        }
    }
}
#endif
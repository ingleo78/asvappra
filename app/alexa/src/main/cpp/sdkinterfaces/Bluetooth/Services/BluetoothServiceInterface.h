#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_BLUETOOTHSERVICEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_BLUETOOTHSERVICEINTERFACE_H_

#include <memory>
#include "SDPRecordInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    class BluetoothServiceInterface {
                    public:
                        virtual std::shared_ptr<SDPRecordInterface> getRecord() = 0;
                        virtual ~BluetoothServiceInterface() = default;
                        virtual void setup() = 0;
                        virtual void cleanup() = 0;
                    };
                }
            }
        }
    }
}
#endif
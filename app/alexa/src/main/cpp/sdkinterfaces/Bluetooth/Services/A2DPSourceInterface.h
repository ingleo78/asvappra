#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSOURCEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_A2DPSOURCEINTERFACE_H_

#include <memory>
#include <util/bluetooth/FormattedAudioStreamAdapter.h>
#include "BluetoothServiceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    class A2DPSourceInterface : public BluetoothServiceInterface {
                    public:
                        static constexpr const char* UUID = "0000110a-0000-1000-8000-00805f9b34fb";
                        static constexpr const char* NAME = "AudioSource";
                        virtual std::shared_ptr<avsCommon::utils::bluetooth::FormattedAudioStreamAdapter> getSourceStream() = 0;
                        virtual ~A2DPSourceInterface() = default;
                    };
                }
            }
        }
    }
}
#endif
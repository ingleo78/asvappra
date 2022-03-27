#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_SDPRECORDINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_SDPRECORDINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    class SDPRecordInterface {
                    public:
                        static const std::string_view BASE_UUID() {
                            return "00000000-0000-1000-8000-00805f9b34fb";
                        }
                        virtual ~SDPRecordInterface() = default;
                        virtual std::string getName() const = 0;
                        virtual std::string getUuid() const = 0;
                        virtual std::string getVersion() const = 0;
                    };
                }
            }
        }
    }
}
#endif
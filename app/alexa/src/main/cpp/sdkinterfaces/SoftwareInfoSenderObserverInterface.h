#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SOFTWAREINFOSENDEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SOFTWAREINFOSENDEROBSERVERINTERFACE_H_

#include <cstdint>
#include <limits>
#include <stdint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace softwareInfo {
                typedef int32_t FirmwareVersion;
                static const FirmwareVersion INVALID_FIRMWARE_VERSION = 0;
                static const FirmwareVersion MAX_FIRMWARE_VERSION = std::numeric_limits<FirmwareVersion>::max();
                inline bool isValidFirmwareVersion(FirmwareVersion version) {
                    return version > 0;
                }
            };
            class SoftwareInfoSenderObserverInterface {
            public:
                virtual ~SoftwareInfoSenderObserverInterface() = default;
                virtual void onFirmwareVersionAccepted(softwareInfo::FirmwareVersion firmwareVersion);
            };
        }
    }
}
#endif
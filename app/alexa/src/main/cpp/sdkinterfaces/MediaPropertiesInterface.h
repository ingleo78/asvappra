#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MEDIAPROPERTIESINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MEDIAPROPERTIESINTERFACE_H_

#include <chrono>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class MediaPropertiesInterface {
            public:
                virtual ~MediaPropertiesInterface() = default;
                virtual std::chrono::milliseconds getAudioItemOffset();
                virtual std::chrono::milliseconds getAudioItemDuration() {
                    return std::chrono::milliseconds::zero();
                };
            };
        }
    }
}
#endif
#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERMODECONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERMODECONTROLLERINTERFACE_H_

#include "EqualizerTypes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                class EqualizerModeControllerInterface {
                public:
                    virtual ~EqualizerModeControllerInterface() = default;
                    virtual bool setEqualizerMode(EqualizerMode mode);
                };
            }
        }
    }
}
#endif
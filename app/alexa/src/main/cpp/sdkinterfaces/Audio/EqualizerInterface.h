#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERINTERFACE_H_

#include "EqualizerTypes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                class EqualizerInterface {
                public:
                    virtual ~EqualizerInterface() = default;
                    virtual void setEqualizerBandLevels(EqualizerBandLevelMap bandLevelMap);
                    virtual int getMinimumBandLevel();
                    virtual int getMaximumBandLevel();
                };
            }
        }
    }
}
#endif
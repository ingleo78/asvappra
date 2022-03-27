#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERCONTROLLERLISTENERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERCONTROLLERLISTENERINTERFACE_H_

#include "EqualizerTypes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                class EqualizerControllerListenerInterface {
                public:
                    virtual ~EqualizerControllerListenerInterface() = default;
                    virtual void onEqualizerStateChanged(const EqualizerState& newState);
                    virtual void onEqualizerSameStateChanged(const EqualizerState& newState);
                };
            }
        }
    }
}
#endif
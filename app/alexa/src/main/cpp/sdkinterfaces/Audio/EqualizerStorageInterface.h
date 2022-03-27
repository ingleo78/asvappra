#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERSTORAGEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERSTORAGEINTERFACE_H_

#include <memory>
#include <util/error/SuccessResult.h>
#include "EqualizerTypes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                class EqualizerStorageInterface {
                public:
                    virtual ~EqualizerStorageInterface() = default;
                    virtual void saveState(const EqualizerState& state);
                    virtual avsCommon::utils::error::SuccessResult<EqualizerState> loadState();
                    virtual void clear();
                };
            }
        }
    }
}
#endif
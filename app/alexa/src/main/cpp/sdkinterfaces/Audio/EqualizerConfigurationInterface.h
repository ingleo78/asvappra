#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERCONFIGURATIONINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERCONFIGURATIONINTERFACE_H_

#include <set>
#include <string>
#include <util/error/SuccessResult.h>
#include <functional/hash.h>
#include "EqualizerTypes.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                class EqualizerConfigurationInterface {
                public:
                    virtual ~EqualizerConfigurationInterface() = default;
                    virtual bool isEnabled() const;
                    virtual std::set<EqualizerBand> getSupportedBands() const;
                    virtual std::set<EqualizerMode> getSupportedModes() const;
                    virtual int getMinBandLevel() const;
                    virtual int getMaxBandLevel() const;
                    virtual int getDefaultBandDelta() const;
                    virtual EqualizerState getDefaultState() const;
                    virtual bool isBandSupported(EqualizerBand band) const;
                    virtual bool isModeSupported(EqualizerMode mode) const;
                };
            }
        }
    }
}
#endif
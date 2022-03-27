#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEAKERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEAKERINTERFACE_H_

#include <ostream>
#include <stdint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class SpeakerInterface {
            public:
                struct SpeakerSettings {
                    int8_t volume;
                    bool mute;
                    bool operator==(const SpeakerSettings& rhs) const {
                        return volume == rhs.volume && mute == rhs.mute;
                    }
                };
                virtual bool setVolume(int8_t volume);
                virtual bool setMute(bool mute);
                virtual bool getSpeakerSettings(SpeakerSettings* settings);
                virtual ~SpeakerInterface() = default;
            };
        }
    }
}
#endif
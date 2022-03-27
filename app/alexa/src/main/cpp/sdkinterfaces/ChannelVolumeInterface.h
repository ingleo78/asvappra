#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CHANNELVOLUMEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CHANNELVOLUMEINTERFACE_H_

#include <functional>
#include <iostream>
#include <stdint.h>
#include <avs/SpeakerConstants/SpeakerConstants.h>
#include "SpeakerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class ChannelVolumeInterface {
            public:
                enum class Type {
                    AVS_SPEAKER_VOLUME,
                    AVS_ALERTS_VOLUME
                };
                virtual bool startDucking();
                virtual bool stopDucking();
                virtual bool setUnduckedVolume(int8_t volume);
                virtual bool setMute(bool mute);
                virtual bool getSpeakerSettings(avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings) const;
                virtual Type getSpeakerType() const;
                virtual ~ChannelVolumeInterface() = default;
            };
            inline std::ostream& operator<<(std::ostream& stream, ChannelVolumeInterface::Type type) {
                switch(type) {
                    case ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME:
                        stream << "AVS_SPEAKER_VOLUME";
                        return stream;
                    case ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME:
                        stream << "AVS_ALERTS_VOLUME";
                        return stream;
                }
                stream << "UNKNOWN_CHANNEL_VOLUME_TYPE";
                return stream;
            }
        }
    }
}
#endif